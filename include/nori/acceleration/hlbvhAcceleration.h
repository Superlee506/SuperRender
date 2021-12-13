//
// Created by superqqli on 2021/12/13.
//

#pragma once
#include <nori/core/accel.h>
NORI_NAMESPACE_BEGIN

struct mortonShape;
struct BVHBuildNode;
struct LinearBVHNode;

class HLBVHAccel : public Accel
{
public:
    HLBVHAccel(const PropertyList & propList);

    virtual ~HLBVHAccel();

    virtual void build() override;

    virtual bool rayIntersect(const Ray3f & ray, Intersection & its, bool bShadowRay) const;

    virtual std::string toString() const override;

private:
    uint32_t leftShift3(uint32_t x) const;
    uint32_t encodeMorton3(const Vector3f & vec) const;
    void radixSort(std::vector<mortonShape> & mortonShapes) const;
    BVHBuildNode * emitHlbvh(
            BVHBuildNode *& pBuildNodes,
            mortonShape * pMortonShapes,
            uint32_t nShape,
            uint32_t * nTotalNodes,
            uint32_t * nLeafNodes,
            std::vector<PrimitiveShape*> & orderedShapes,
            std::atomic<int> * nOrderedShapeOffset,
            int iFirstBitIdx
    );
    BVHBuildNode * buildUpperSAH(
            std::vector<BVHBuildNode*> & treeletRoots,
            uint32_t iStart,
            uint32_t iEnd
    );
    uint32_t flattenBvhTree(BVHBuildNode * pNode, uint32_t * pOffset);

private:
    uint32_t m_leafSize = 0;
    uint32_t m_nNodes = 0;
    uint32_t m_nLeafs = 0;
    MemoryArena m_memoryArena;/// Use it's own memory manager
    LinearBVHNode * m_pNodes;
};

NORI_NAMESPACE_END
