//
// Created by superqqli on 2021/12/13.
//

#include <nori/acceleration/hlbvhAcceleration.h>
#include <nori/core/timer.h>
#include <nori/core/primitiveShape.h>
#include <nori/core/intersection.h>
#include <tbb\tbb.h>

NORI_NAMESPACE_BEGIN
struct mortonShape
{
    uint32_t iShape = 0;
    uint32_t mortonCode = 0;
};

struct BVHBuildNode
{
    BoundingBox3f bBox;
    BVHBuildNode * pChildren[2] = { nullptr };
    uint32_t iSplitAxis = 0;
    uint32_t nFirstShapeOffset = 0;
    uint32_t nShape = 0;

    void initLeaf(uint32_t iFirst, uint32_t N, const BoundingBox3f & B)
    {
        iSplitAxis = 3;
        nFirstShapeOffset = iFirst;
        nShape = N;
        bBox = B;
        pChildren[0] = pChildren[1] = nullptr;
    }

    void initInterior(uint32_t iAxis, BVHBuildNode * pLeft, BVHBuildNode * pRight)
    {
        iSplitAxis = iAxis;
        pChildren[0] = pLeft;
        pChildren[1] = pRight;
        bBox = pLeft->bBox;
        bBox.expandBy(pRight->bBox);
        nShape = 0;
        nFirstShapeOffset = uint32_t(-1);
    }
};

struct HLBVHTreeLet
{
    uint32_t iStart;
    uint32_t nShape;
    BVHBuildNode * pNodes;
};

struct BVHBucket
{
    uint32_t nShape = 0;
    BoundingBox3f bBox;
};

struct LinearBVHNode
{
    BoundingBox3f bBox;
    union
    {
        uint32_t nShapeOffset;   // Leaf
        uint32_t nRightChildOffset;  // Interior
    };
    uint32_t nShape = 0;
    uint32_t iAxis = 0;
};

HLBVHAccel::HLBVHAccel(const PropertyList & propList) :
        Accel(propList)
{
    m_leafSize = uint32_t(propList.getInteger(XML_ACCELERATION_HLBVH_LEAF_SIZE, DEFAULT_ACCELERATION_HLBVH_LEAF_SIZE));
}

HLBVHAccel::~HLBVHAccel()
{
    delete[] m_pNodes;
}

void HLBVHAccel::build()
{
    Timer hlbvhBuildTimer;

    // Compute bounding box of all shapes centroids
    BoundingBox3f bBox = m_pShapes[0]->getBoundingBox();
    for (uint32_t i = 1; i < m_pShapes.size(); i++)
    {
        bBox.expandBy(m_pShapes[i]->getBoundingBox());
    }

    // Compute Morton indices of shapes
    Point3f bBoxExtents = bBox.getExtents();
    std::vector<mortonShape> mortonShapes(m_pShapes.size());

    constexpr int MORTON_BITS = 10;
    constexpr int MORTON_SCALE = 1 << MORTON_BITS;

    tbb::blocked_range<int> mortonRange(0, int(m_pShapes.size()));
    auto mortonMap = [&](const tbb::blocked_range<int> & range)
    {
        for (int i = range.begin(); i < range.end(); i++)
        {
            mortonShapes[i].iShape = i;
            mortonShapes[i].mortonCode = encodeMorton3((m_pShapes[i]->getCentroid() - bBox.min).cwiseQuotient(bBoxExtents) * MORTON_SCALE);
        }
    };

    /// Uncomment the following line for single threaded computing
    //mortonMap(mortonRange);

    /// Default: parallel computing
    tbb::parallel_for(mortonRange, mortonMap);

    // Radix sort shape Morton indices
    radixSort(mortonShapes);

    // Create LBVH treelets at bottom of BVH

    // Find intervals of shapes for each treelet
    std::vector<HLBVHTreeLet> treeletsToBuild;
    for (uint32_t iStart = 0, iEnd = 1; iEnd <= mortonShapes.size(); iEnd++)
    {
        constexpr uint32_t MASK = 0b111111111111000000000000000000;

        if (iEnd == uint32_t(mortonShapes.size()) || ((mortonShapes[iStart].mortonCode & MASK) != (mortonShapes[iEnd].mortonCode & MASK)))
        {
            // Add entry to TreeletsToBuild for this treelet
            uint32_t nShape = iEnd - iStart;
            uint32_t nMaxBVHNodes = 2 * nShape;

            // For performance concerned, constructor should not be executed here
            BVHBuildNode * pNodes = m_memoryArena.alloc<BVHBuildNode>(nMaxBVHNodes, false);
            HLBVHTreeLet treeLet;
            treeLet.iStart = iStart;
            treeLet.nShape = nShape;
            treeLet.pNodes = pNodes;
            treeletsToBuild.push_back(treeLet);

            iStart = iEnd;
        }
    }

    // Create HLBVH for treelets
    std::atomic<int> nAtomicTotal(0), nAtomicLeaf(0), nOrderedShapeOffset(0);
    std::vector<PrimitiveShape*> orderedShapes(m_pShapes.size());

    tbb::blocked_range<int> treeletRange(0, int(treeletsToBuild.size()));
    auto treeletMap = [&](const tbb::blocked_range<int> & range)
    {
        const int iFirstBitIdx = 29 - 12;
        for (int i = range.begin(); i < range.end(); i++)
        {
            uint32_t nTotalNodes = 0;
            uint32_t nLeafNodes = 0;
            HLBVHTreeLet & treeLet = treeletsToBuild[i];

            treeLet.pNodes = emitHlbvh(
                    treeLet.pNodes,
                    &mortonShapes[treeLet.iStart],
                    treeLet.nShape,
                    &nTotalNodes,
                    &nLeafNodes,
                    orderedShapes,
                    &nOrderedShapeOffset,
                    iFirstBitIdx
            );

            nAtomicTotal += nTotalNodes;
            nAtomicLeaf += nLeafNodes;
        }
    };

    /// Uncomment the following line for single threaded building
    //treeletMap(treeletRange);

    /// Default: parallel building
    tbb::parallel_for(treeletRange, treeletMap);

    m_nNodes = nAtomicTotal;
    m_nLeafs = nAtomicLeaf;

    // Create and return SAH BVH from HLBVH treelets
    std::vector<BVHBuildNode*> finishedTreelets;
    finishedTreelets.reserve(treeletsToBuild.size());
    for (HLBVHTreeLet & Treelet : treeletsToBuild)
    {
        finishedTreelets.push_back(Treelet.pNodes);
    }

    BVHBuildNode * pRoot = buildUpperSAH(finishedTreelets, 0, uint32_t(finishedTreelets.size()));

    m_pShapes.swap(orderedShapes);

    uint32_t nOffset = 0;
    m_pNodes = new LinearBVHNode[m_nNodes];
    flattenBvhTree(pRoot, &nOffset);
    CHECK(m_nNodes == nOffset);

    m_memoryArena.release();

    LOG(INFO) << "Build HLBVH (" << m_nNodes << " nodes, with " << m_nLeafs << " leafs) in " <<
              hlbvhBuildTimer.elapsedString() << " and take " << memString(m_nNodes * sizeof(LinearBVHNode)) << ".";
}

bool HLBVHAccel::rayIntersect(const Ray3f & ray, Intersection & its, bool bShadowRay) const
{
    if (m_pNodes == nullptr)
    {
        return false;
    }

    bool bFoundIntersection = false;       // Was an intersection found so far?
    PrimitiveShape * pFoundShape = nullptr;

    Ray3f rayCopy(ray);
    bool bDirNeg[3] = {rayCopy.dRcp.x() < 0, rayCopy.dRcp.y() < 0, rayCopy.dRcp.z() < 0 };

    uint32_t nToVisitOffset = 0, iCurrentNodeIndex = 0;
    uint32_t iNodesToVisit[1024];

    while (true)
    {
        const LinearBVHNode * pLinearNode = &m_pNodes[iCurrentNodeIndex];

        if (pLinearNode->bBox.rayIntersect(rayCopy))
        {
            // Leaf node
            if (pLinearNode->nShape > 0)
            {
                for (uint32_t i = 0; i < pLinearNode->nShape; i++)
                {
                    float U, V, T;
                    PrimitiveShape * pShape = m_pShapes[pLinearNode->nShapeOffset + i];
                    if (pShape->rayIntersect(rayCopy, U, V, T))
                    {
                        if (bShadowRay)
                        {
                            return true;
                        }

                        rayCopy.maxt = its.t = T;
                        its.uv = Point2f(U, V);
                        its.pShape = pShape;

                        pFoundShape = pShape;
                        bFoundIntersection = true;
                    }
                }

                if (nToVisitOffset == 0)
                {
                    break;
                }

                iCurrentNodeIndex = iNodesToVisit[--nToVisitOffset];
            }
                // Interior node
            else
            {
                if (bDirNeg[pLinearNode->iAxis])
                {
                    iNodesToVisit[nToVisitOffset++] = iCurrentNodeIndex + 1;
                    iCurrentNodeIndex = pLinearNode->nRightChildOffset;
                }
                else
                {
                    iNodesToVisit[nToVisitOffset++] = pLinearNode->nRightChildOffset;
                    iCurrentNodeIndex = iCurrentNodeIndex + 1;
                }
            }
        }
        else
        {
            if (nToVisitOffset == 0)
            {
                break;
            }
            iCurrentNodeIndex = iNodesToVisit[--nToVisitOffset];
        }
    }

    if (bFoundIntersection)
    {
        pFoundShape->postIntersect(its);
        its.computeScreenSpacePartial(ray);
    }

    return bFoundIntersection;
}

std::string HLBVHAccel::toString() const
{
    return tfm::format(
            "HLBVHAcceleration[\n"
            "  node = %s,\n"
            "  leafNode = %f\n"
            "]",
            m_nNodes,
            m_nLeafs
    );
}

uint32_t HLBVHAccel::leftShift3(uint32_t x) const
{
    CHECK(x <= (1 << 10));
    if (x == (1 << 10))
    {
        x--;
    }

    // x = ---- --98 ---- ---- ---- ---- 7654 3210
    x = (x | (x << 16)) & 0b00000011000000000000000011111111;
    // x = ---- --98 ---- ---- 7654 ---- ---- 3210
    x = (x | (x << 8 )) & 0b00000011000000001111000000001111;
    // x = ---- --98 ---- 76-- --54 ---- 32-- --10
    x = (x | (x << 4 )) & 0b00000011000011000011000011000011;
    // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
    x = (x | (x << 2 )) & 0b00001001001001001001001001001001;

    return x;
}

uint32_t HLBVHAccel::encodeMorton3(const Vector3f & vec) const
{
    CHECK(vec.x() >= 0.0f && vec.y() >= 0.0f && vec.z() >= 0.0f);
    return (
            (leftShift3(uint32_t(vec.z())) << 2) |
            (leftShift3(uint32_t(vec.y())) << 1) |
            leftShift3(uint32_t(vec.x()))
    );
}

void HLBVHAccel::radixSort(std::vector<mortonShape> & mortonShapes) const
{
    // TODO : Could be paralleled

    std::vector<mortonShape> tempVector(mortonShapes.size());
    constexpr int BIT_PER_PASS = 6;
    constexpr int BITS = 30;
    static_assert((BITS % BIT_PER_PASS) == 0, "Radix sort bitsPerPass must evenly divide nBits");

    constexpr int nPasses = BITS / BIT_PER_PASS;

    for (int pass = 0; pass < nPasses; ++pass)
    {
        // Perform one pass of radix sort, sorting BIT_PER_PASS bits
        int lowBit = pass * BIT_PER_PASS;

        // Set in and out vector pointers for radix sort pass
        std::vector<mortonShape> & in = (pass & 1) ? tempVector : mortonShapes;
        std::vector<mortonShape> & out = (pass & 1) ? mortonShapes : tempVector;

        // Count number of zero bits in array for current radix sort bit
        constexpr int BUCKET_NUM = 1 << BIT_PER_PASS;
        int bucketCount[BUCKET_NUM] = {0 };
        constexpr int BIT_MASK = (1 << BIT_PER_PASS) - 1;

        for (const mortonShape & mortonShape : in)
        {
            int bucketIdx = (mortonShape.mortonCode >> lowBit) & BIT_MASK;
            CHECK(bucketIdx >= 0 && bucketIdx < BUCKET_NUM);
            ++bucketCount[bucketIdx];
        }

        // Compute starting index in output array for each bucket
        int outIndex[BUCKET_NUM];
        outIndex[0] = 0;
        for (int i = 1; i < BUCKET_NUM; ++i)
        {
            outIndex[i] = outIndex[i - 1] + bucketCount[i - 1];
        }

        // Store sorted values in output array
        for (const mortonShape & mortonShape : in)
        {
            int BucketIdx = (mortonShape.mortonCode >> lowBit) & BIT_MASK;
            out[outIndex[BucketIdx]++] = mortonShape;
        }
    }

    // Copy final result from TempVector, if needed
    if (nPasses & 1)
    {
        std::swap(mortonShapes, tempVector);
    }
}

BVHBuildNode * HLBVHAccel::emitHlbvh(
        BVHBuildNode *& pBuildNodes,
        mortonShape * pMortonShapes,
        uint32_t nShape,
        uint32_t * nTotalNodes,
        uint32_t * nLeafNodes,
        std::vector<PrimitiveShape*> & orderedShapes,
        std::atomic<int> * nOrderedShapeOffset,
        int iFirstBitIdx
)
{
    CHECK(nShape > 0);

    // Create and return leaf node of HLBVH treelet
    if (iFirstBitIdx == -1 || nShape <= m_leafSize)
    {
        (*nTotalNodes)++;
        (*nLeafNodes)++;

        BVHBuildNode * pNode = pBuildNodes++;

        // First fetch the nOrderedShapeOffset and then add nShape to it
        uint32_t nFirstShapeOffset = nOrderedShapeOffset->fetch_add(nShape);

        uint32_t shapeIdx = pMortonShapes[0].iShape;

        BoundingBox3f bBox = m_pShapes[shapeIdx]->getBoundingBox();
        orderedShapes[0 + nFirstShapeOffset] = m_pShapes[shapeIdx];

        for (uint32_t i = 1; i < nShape; i++)
        {
            shapeIdx = pMortonShapes[i].iShape;
            orderedShapes[i + nFirstShapeOffset] = m_pShapes[shapeIdx];
            bBox.expandBy(m_pShapes[shapeIdx]->getBoundingBox());
        }

        pNode->initLeaf(nFirstShapeOffset, nShape, bBox);
        return pNode;
    }
    else
    {
        int mask = 1 << iFirstBitIdx;

        // Advance to next subtree level if there's no HLBVH split for this bit
        if ((pMortonShapes[0].mortonCode & mask) == (pMortonShapes[nShape - 1].mortonCode & mask))
        {
            return emitHlbvh(
                    pBuildNodes,
                    pMortonShapes,
                    nShape,
                    nTotalNodes,
                    nLeafNodes,
                    orderedShapes,
                    nOrderedShapeOffset,
                    iFirstBitIdx - 1
            );
        }

        // Find HLBVH split point for this dimension
        uint32_t iSearchStart = 0, iSearchEnd = nShape - 1;
        while (iSearchStart + 1 != iSearchEnd)
        {
            CHECK(iSearchStart != iSearchEnd);
            uint32_t iMid = (iSearchStart + iSearchEnd) / 2;

            if ((pMortonShapes[iSearchStart].mortonCode & mask) == (pMortonShapes[iMid].mortonCode & mask))
            {
                iSearchStart = iMid;
            }
            else
            {
                CHECK((pMortonShapes[iMid].mortonCode & mask) == (pMortonShapes[iSearchEnd].mortonCode & mask));
                iSearchEnd = iMid;
            }
        }

        uint32_t nSplitOffset = iSearchEnd;
        CHECK(nSplitOffset <= nShape - 1);
        CHECK((pMortonShapes[nSplitOffset - 1].mortonCode & mask) != (pMortonShapes[nSplitOffset].mortonCode & mask));

        // Create and return interior HLBVH node
        (*nTotalNodes)++;

        BVHBuildNode * pNode = pBuildNodes++;
        BVHBuildNode * pLeft = emitHlbvh(
                pBuildNodes,
                pMortonShapes,
                nSplitOffset,
                nTotalNodes,
                nLeafNodes,
                orderedShapes,
                nOrderedShapeOffset,
                iFirstBitIdx - 1
        );
        BVHBuildNode * pRight = emitHlbvh(
                pBuildNodes,
                &pMortonShapes[nSplitOffset],
                nShape - nSplitOffset,
                nTotalNodes,
                nLeafNodes,
                orderedShapes,
                nOrderedShapeOffset,
                iFirstBitIdx - 1
        );
        uint32_t iAxis = uint32_t(iFirstBitIdx % 3);
        pNode->initInterior(iAxis, pLeft, pRight);
        return pNode;
    }
}

BVHBuildNode * HLBVHAccel::buildUpperSAH(
        std::vector<BVHBuildNode*> & treeletRoots,
        uint32_t iStart,
        uint32_t iEnd
)
{
    CHECK(iStart < iEnd);

    uint32_t nNodes = iEnd - iStart;
    if (nNodes == 1)
    {
        return treeletRoots[iStart];
    }

    m_nNodes++;

    BVHBuildNode * pNode = m_memoryArena.alloc<BVHBuildNode>();

    // Compute bounds of all nodes under this HLBVH node
    BoundingBox3f bBox = treeletRoots[iStart]->bBox;
    for (uint32_t i = iStart + 1; i < iEnd; i++)
    {
        bBox.expandBy(treeletRoots[i]->bBox);
    }

    // Compute bound of HLBVH node centroids, choose split dimension
    BoundingBox3f centroid(0.5f * (treeletRoots[iStart]->bBox.min + treeletRoots[iStart]->bBox.max));
    for (uint32_t i = iStart + 1; i < iEnd; i++)
    {
        centroid.expandBy(0.5f * (treeletRoots[i]->bBox.min + treeletRoots[i]->bBox.max));
    }

    uint32_t iSplitDim = centroid.getMajorAxis();
    if (centroid.max[iSplitDim] == centroid.min[iSplitDim])
    {
        uint32_t iMid = (iStart + iEnd) / 2;
        CHECK(iMid > iStart && iMid < iEnd);

        pNode->initInterior(
                iSplitDim,
                buildUpperSAH(treeletRoots, iStart, iMid),
                buildUpperSAH(treeletRoots, iMid, iEnd)
        );

        return pNode;
    }

    constexpr int BUCKET_NUM = 12;
    BVHBucket buckets[BUCKET_NUM];

    float invNorm = 1.0f / (centroid.max[iSplitDim] - centroid.min[iSplitDim]);
    float invSurfaceArea = 1.0f / bBox.getSurfaceArea();

    // Initialize Buckets for HLBVH SAH partition buckets
    for (uint32_t i = iStart; i < iEnd; i++)
    {
        uint32_t bucketIdx = uint32_t((BUCKET_NUM - 1) * (treeletRoots[i]->bBox.getCenter()[iSplitDim] - centroid.min[iSplitDim]) * invNorm);
        CHECK(bucketIdx >= 0 && bucketIdx < BUCKET_NUM);

        buckets[bucketIdx].nShape++;
        if (buckets[bucketIdx].bBox.isValid())
        {
            buckets[bucketIdx].bBox.expandBy(treeletRoots[i]->bBox);
        }
        else
        {
            buckets[bucketIdx].bBox = treeletRoots[i]->bBox;
        }
    }

    // Compute costs for splitting after each bucket
    float cost[BUCKET_NUM - 1];
    for (uint32_t i = 0; i < BUCKET_NUM - 1; i++)
    {
        BoundingBox3f leftBox, rightBox;
        uint32_t nLeftShapes = 0, nRightShapes = 0;

        // Left
        leftBox = buckets[0].bBox;
        for (uint32_t j = 0; j <= i; j++)
        {
            leftBox.expandBy(buckets[j].bBox);
            nLeftShapes += buckets[j].nShape;
        }

        // Right
        rightBox = buckets[i + 1].bBox;
        for (uint32_t j = i + 1; j <= BUCKET_NUM - 1; j++)
        {
            rightBox.expandBy(buckets[j].bBox);
            nRightShapes += buckets[j].nShape;
        }

        cost[i] = 0.125f + (
                                   leftBox.getSurfaceArea() * nLeftShapes +
                                           rightBox.getSurfaceArea() * nRightShapes
                           ) * invSurfaceArea;
    }

    // Find bucket to split at that minimizes SAH metric
    float minCost = cost[0];
    uint32_t iMinCostSplitBucket = 0;
    for (uint32_t i = 1; i < BUCKET_NUM - 1; i++)
    {
        if (cost[i] < minCost)
        {
            minCost = cost[i];
            iMinCostSplitBucket = i;
        }
    }

    auto midIter = std::partition(
            &treeletRoots[iStart],
            &treeletRoots[iEnd - 1] + 1,
            [=](const BVHBuildNode * pNode)
            {
                uint32_t bucketIdx = uint32_t((BUCKET_NUM - 1) * (pNode->bBox.getCenter()[iSplitDim] - centroid.min[iSplitDim]) * invNorm);
                CHECK(bucketIdx >= 0 && bucketIdx < uint32_t(BUCKET_NUM));
                return bucketIdx <= iMinCostSplitBucket;
            }
    );

    uint32_t iMid = uint32_t(midIter - &treeletRoots[0]);
    CHECK(iMid > iStart && iMid < iEnd);

    pNode->initInterior(
            iSplitDim,
            buildUpperSAH(treeletRoots, iStart, iMid),
            buildUpperSAH(treeletRoots, iMid, iEnd)
    );

    return pNode;
}

uint32_t HLBVHAccel::flattenBvhTree(BVHBuildNode * pNode, uint32_t * pOffset)
{
    CHECK(*pOffset < m_nNodes);

    LinearBVHNode * pLinearNode = &m_pNodes[*pOffset];
    pLinearNode->bBox = pNode->bBox;

    uint32_t offset = (*pOffset)++;

    // Leaf node
    if (pNode->nShape > 0)
    {
        CHECK(pNode->pChildren[0] == nullptr && pNode->pChildren[1] == nullptr);
        CHECK(pNode->nShape < 65536);

        pLinearNode->nShapeOffset = pNode->nFirstShapeOffset;
        pLinearNode->nShape = pNode->nShape;
    }
        // Interior node
    else
    {
        pLinearNode->iAxis = pNode->iSplitAxis;
        pLinearNode->nShape = 0;
        flattenBvhTree(pNode->pChildren[0], pOffset);
        pLinearNode->nRightChildOffset = flattenBvhTree(pNode->pChildren[1], pOffset);

        CHECK(pNode->bBox.isValid() && pNode->pChildren[0]->bBox.isValid() && pNode->pChildren[1]->bBox.isValid());
        CHECK(pNode->bBox.contains(pNode->pChildren[0]->bBox) && pNode->bBox.contains(pNode->pChildren[1]->bBox));
    }

    return offset;
}

NORI_REGISTER_CLASS(HLBVHAccel, XML_ACCELERATION_HLBVH);
NORI_NAMESPACE_END
