/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nori/acceleration/bvhAcceleration.h>
#include <nori/core/intersection.h>
#include <nori/core/primitiveShape.h>
#include <nori/core/timer.h>
#include <tbb/tbb.h>
#include <atomic>

/*
* =======================================================================
*   NOTE    NOTE    NOTE    NOTE    NOTE    NOTE    NOTE    NOTE    NOTE
* =======================================================================
*   This code was ripped out of the old version of nori that came with a
*   default BVH implementation. Since the focus of our lecture is on
*   light transport, we do not require that you implement 'low-level' ray
*   tracing yourself. Still, you are encouraged to replace this with your
*   own toy implementation of an acceleration data structure.
*
*   For the original code, see
*   https://github.com/wjakob/nori-old/blob/master/src/bvh.cpp
* =======================================================================
*/

/*
* =======================================================================
*   WARNING    WARNING    WARNING    WARNING    WARNING    WARNING
* =======================================================================
*   Remember to put on SAFETY GOGGLES before looking at this file. You
*   are most certainly not expected to read or understand any of it.
* =======================================================================
*/

NORI_NAMESPACE_BEGIN

BvhAccel::BvhAccel(const PropertyList& list) : Accel(list)
{

}

BvhAccel::~BvhAccel() { }

/* Bin data structure for counting triangles and computing their bounding box */
struct Bins {
    static const int BIN_COUNT = 16;
    Bins() { memset(counts, 0, sizeof(uint32_t) * BIN_COUNT); }
    uint32_t counts[BIN_COUNT];
    BoundingBox3f bbox[BIN_COUNT];
};

struct BvhAccel::Internals {
/**
* \brief Build task for parallel Accel construction
*
* This class uses the task scheduling system of Intel' Thread Building Blocks
* to parallelize the divide and conquer Accel build at all levels.
*
* The used methodology is roughly that described in
* "Fast and Parallel Construction of SAH-based Bounding Volume Hierarchies"
* by Ingo Wald (Proc. IEEE/EG Symposium on Interactive Ray Tracing, 2007)
*/
    class BVHBuildTask : public tbb::task {
    private:
        BvhAccel &bvh;
        uint32_t node_idx;
        uint32_t *start, *end, *temp;

    public:
        /// Build-related parameters
        enum {
            /// Switch to a serial build when less than 32 triangles are left
            SERIAL_THRESHOLD = 32,

            /// Process triangles in batches of 1K for the purpose of parallelization
            GRAIN_SIZE = 1000,

            /// Heuristic cost value for traversal operations
            TRAVERSAL_COST = 1,

            /// Heuristic cost value for intersection operations
            INTERSECTION_COST = 1
        };

    public:
        /**
        * Create a new build task
        *
        * \param bvh
        *    Reference to the underlying Accel
        *
        * \param node_idx
        *    Index of the Accel node that should be built
        *
        * \param start
        *    Start pointer into a list of triangle indices to be processed
        *
        * \param end
        *    End pointer into a list of triangle indices to be processed
        *
        *  \param temp
        *    Pointer into a temporary memory region that can be used for
        *    construction purposes. The usable length is <tt>end-start</tt>
        *    unsigned integers.
        */
        BVHBuildTask(BvhAccel &bvh, uint32_t node_idx, uint32_t *start, uint32_t *end, uint32_t *temp)
                : bvh(bvh), node_idx(node_idx), start(start), end(end), temp(temp) { }

        task *execute() {
            uint32_t size = (uint32_t)(end - start);
            BvhAccel::BVHNode &node = bvh.m_nodes[node_idx];

            /* Switch to a serial build when less than SERIAL_THRESHOLD triangles are left */
            if (size < SERIAL_THRESHOLD) {
                execute_serially(bvh, node_idx, start, end, temp);
                return nullptr;
            }

            /* Always split along the largest axis */
            int axis = node.bbox.getLargestAxis();
            float min = node.bbox.min[axis], max = node.bbox.max[axis],
                    inv_bin_size = Bins::BIN_COUNT / (max - min);

            /* Accumulate all triangles into bins */
            Bins bins = tbb::parallel_reduce(
                    tbb::blocked_range<uint32_t>(0u, size, GRAIN_SIZE),
                    Bins(),
                    /* MAP: Bin a number of triangles and return the resulting 'Bins' data structure */
                    [&](const tbb::blocked_range<uint32_t> &range, Bins result) {
                        for (uint32_t i = range.begin(); i != range.end(); ++i) {
                            uint32_t f = start[i];
                            float centroid = BaseInternals::getCentroid(bvh, f)[axis];

                            int index = std::min(std::max(
                                    (int)((centroid - min) * inv_bin_size), 0),
                                                 (Bins::BIN_COUNT - 1));

                            result.counts[index]++;
                            result.bbox[index].expandBy(BaseInternals::getBoundingBox(bvh, f));
                        }
                        return result;
                    },
                    /* REDUCE: Combine two 'Bins' data structures */
                    [](const Bins &b1, const Bins &b2) {
                        Bins result;
                        for (int i = 0; i < Bins::BIN_COUNT; ++i) {
                            result.counts[i] = b1.counts[i] + b2.counts[i];
                            result.bbox[i] = BoundingBox3f::merge(b1.bbox[i], b2.bbox[i]);
                        }
                        return result;
                    }
            );

            /* Choose the best split plane based on the binned data */
            BoundingBox3f bbox_left[Bins::BIN_COUNT];
            bbox_left[0] = bins.bbox[0];
            for (int i = 1; i<Bins::BIN_COUNT; ++i) {
                bins.counts[i] += bins.counts[i - 1];
                bbox_left[i] = BoundingBox3f::merge(bbox_left[i - 1], bins.bbox[i]);
            }

            BoundingBox3f bbox_right = bins.bbox[Bins::BIN_COUNT - 1], best_bbox_right;
            int64_t best_index = -1;
            float best_cost = (float)INTERSECTION_COST * size;
            float tri_factor = (float)INTERSECTION_COST / node.bbox.getSurfaceArea();

            for (int i = Bins::BIN_COUNT - 2; i >= 0; --i) {
                uint32_t prims_left = bins.counts[i], prims_right = (uint32_t)(end - start) - bins.counts[i];
                float sah_cost = 2.0f * TRAVERSAL_COST +
                                 tri_factor * (prims_left * bbox_left[i].getSurfaceArea() +
                                               prims_right * bbox_right.getSurfaceArea());
                if (sah_cost < best_cost) {
                    best_cost = sah_cost;
                    best_index = i;
                    best_bbox_right = bbox_right;
                }
                bbox_right = BoundingBox3f::merge(bbox_right, bins.bbox[i]);
            }

            if (best_index == -1) {
                /* Could not find a good split plane -- retry with
                more careful serial code just to be sure.. */
                execute_serially(bvh, node_idx, start, end, temp);
                return nullptr;
            }

            uint32_t left_count = bins.counts[best_index];
            int node_idx_left = node_idx + 1;
            int node_idx_right = node_idx + 2 * left_count;

            bvh.m_nodes[node_idx_left].bbox = bbox_left[best_index];
            bvh.m_nodes[node_idx_right].bbox = best_bbox_right;
            node.inner.rightChild = node_idx_right;
            node.inner.axis = axis;
            node.inner.flag = 0;

            std::atomic<uint32_t> offset_left(0),
                    offset_right(bins.counts[best_index]);

            tbb::parallel_for(
                    tbb::blocked_range<uint32_t>(0u, size, GRAIN_SIZE),
                    [&](const tbb::blocked_range<uint32_t> &range) {
                        uint32_t count_left = 0, count_right = 0;
                        for (uint32_t i = range.begin(); i != range.end(); ++i) {
                            uint32_t f = start[i];
                            float centroid = BaseInternals::getCentroid(bvh, f)[axis];
                            int index = (int)((centroid - min) * inv_bin_size);
                            (index <= best_index ? count_left : count_right)++;
                        }
                        uint32_t idx_l = offset_left.fetch_add(count_left);
                        uint32_t idx_r = offset_right.fetch_add(count_right);
                        for (uint32_t i = range.begin(); i != range.end(); ++i) {
                            uint32_t f = start[i];
                            float centroid = BaseInternals::getCentroid(bvh, f)[axis];
                            int index = (int)((centroid - min) * inv_bin_size);
                            if (index <= best_index)
                                temp[idx_l++] = f;
                            else
                                temp[idx_r++] = f;
                        }
                    }
            );
            memcpy(start, temp, size * sizeof(uint32_t));
            assert(offset_left == left_count && offset_right == size);

            /* Create an empty parent task */
            tbb::task& c = *new (allocate_continuation()) tbb::empty_task;
            c.set_ref_count(2);

            /* Post right subtree to scheduler */
            BVHBuildTask &b = *new (c.allocate_child())
                    BVHBuildTask(bvh, node_idx_right, start + left_count,
                                 end, temp + left_count);
            spawn(b);

            /* Directly start working on left subtree */
            recycle_as_child_of(c);
            node_idx = node_idx_left;
            end = start + left_count;

            return this;
        }

        /// Single-threaded build function
        static void execute_serially(BvhAccel& bvh, uint32_t node_idx, uint32_t *start, uint32_t *end, uint32_t *temp) {
            BvhAccel::BVHNode &node = bvh.m_nodes[node_idx];
            uint32_t size = (uint32_t)(end - start);
            float best_cost = (float)INTERSECTION_COST * size;
            int64_t best_index = -1, best_axis = -1;
            float *left_areas = (float *)temp;

            /* Try splitting along every axis */
            for (int axis = 0; axis<3; ++axis) {
                /* Sort all triangles based on their centroid positions projected on the axis */
                std::sort(start, end, [&](uint32_t f1, uint32_t f2) {
                    return BaseInternals::getCentroid(bvh, f1)[axis] < BaseInternals::getCentroid(bvh, f2)[axis];
                });

                BoundingBox3f bbox;
                for (uint32_t i = 0; i<size; ++i) {
                    uint32_t f = *(start + i);
                    bbox.expandBy(BaseInternals::getBoundingBox(bvh, f));
                    left_areas[i] = (float)bbox.getSurfaceArea();
                }
                if (axis == 0)
                    node.bbox = bbox;

                bbox.reset();

                /* Choose the best split plane */
                float tri_factor = INTERSECTION_COST / node.bbox.getSurfaceArea();
                for (uint32_t i = size - 1; i >= 1; --i) {
                    uint32_t f = *(start + i);
                    bbox.expandBy(BaseInternals::getBoundingBox(bvh, f));

                    float left_area = left_areas[i - 1];
                    float right_area = bbox.getSurfaceArea();
                    uint32_t prims_left = i;
                    uint32_t prims_right = size - i;

                    float sah_cost = 2.0f * TRAVERSAL_COST +
                                     tri_factor * (prims_left * left_area +
                                                   prims_right * right_area);

                    if (sah_cost < best_cost) {
                        best_cost = sah_cost;
                        best_index = i;
                        best_axis = axis;
                    }
                }
            }

            if (best_index == -1) {
                /* Splitting does not reduce the cost, make a leaf */
                node.leaf.flag = 1;
                node.leaf.start = (uint32_t)(start - bvh.m_indices.data());
                node.leaf.size = size;
                return;
            }

            std::sort(start, end, [&](uint32_t f1, uint32_t f2) {
                return BaseInternals::getCentroid(bvh, f1)[best_axis] < BaseInternals::getCentroid(bvh, f2)[best_axis];
            });

            uint32_t left_count = (uint32_t)best_index;
            uint32_t node_idx_left = node_idx + 1;
            uint32_t node_idx_right = node_idx + 2 * left_count;
            node.inner.rightChild = node_idx_right;
            node.inner.axis = best_axis;
            node.inner.flag = 0;

            execute_serially(bvh, node_idx_left, start, start + left_count, temp);
            execute_serially(bvh, node_idx_right, start + left_count, end, temp + left_count);
        }
    };

    static std::pair<float, uint32_t> statistics(BvhAccel &bvh, uint32_t node_idx = 0) {
        const BVHNode &node = bvh.m_nodes[node_idx];
        if (node.isLeaf()) {
            return std::make_pair((float)BVHBuildTask::INTERSECTION_COST * node.leaf.size, 1u);
        }
        else {
            std::pair<float, uint32_t> stats_left = statistics(bvh, node_idx + 1u);
            std::pair<float, uint32_t> stats_right = statistics(bvh, node.inner.rightChild);
            float saLeft = bvh.m_nodes[node_idx + 1u].bbox.getSurfaceArea();
            float saRight = bvh.m_nodes[node.inner.rightChild].bbox.getSurfaceArea();
            float saCur = node.bbox.getSurfaceArea();
            float sahCost =
                    2 * BVHBuildTask::TRAVERSAL_COST +
                    (saLeft * stats_left.first + saRight * stats_right.first) / saCur;
            return std::make_pair(
                    sahCost,
                    stats_left.second + stats_right.second + 1u
            );
        }
    }

};

void BvhAccel::build() {
    uint32_t size = m_pShapes.size();
    if (size == 0)
        return;
    cout << "Constructing a SAH BVH (" << m_meshes.size()
         << (m_meshes.size() == 1 ? " mesh, " : " meshes, ")
         << size << " triangles) .. ";
    cout.flush();
    Timer timer;

    /* Conservative estimate for the total number of nodes */
    m_nodes.resize(2 * size);
    memset(m_nodes.data(), 0, sizeof(BVHNode) * m_nodes.size());
    m_nodes[0].bbox = m_bbox;
    m_indices.resize(size);

    if (sizeof(BVHNode) != 32)
        throw NoriException("BVH Node is not packed! Investigate compiler settings.");

    for (uint32_t i = 0; i < size; ++i)
        m_indices[i] = i;

    uint32_t *indices = m_indices.data(), *temp = new uint32_t[size];
    Internals::BVHBuildTask& task = *new(tbb::task::allocate_root())
            Internals::BVHBuildTask(*this, 0u, indices, indices + size, temp);
    tbb::task::spawn_root_and_wait(task);
    delete[] temp;
    std::pair<float, uint32_t> stats = Internals::statistics(*this);

    /* The node array was allocated conservatively and now contains
    many unused entries -- do a compactification pass. */
    std::vector<BVHNode> compactified(stats.second);
    std::vector<uint32_t> skipped_accum(m_nodes.size());

    for (int64_t i = stats.second - 1, j = m_nodes.size(), skipped = 0; i >= 0; --i) {
        while (m_nodes[--j].isUnused())
            skipped++;
        BVHNode &new_node = compactified[i];
        new_node = m_nodes[j];
        skipped_accum[j] = (uint32_t)skipped;

        if (new_node.isInner()) {
            new_node.inner.rightChild = (uint32_t)
                    (i + new_node.inner.rightChild - j -
                     (skipped - skipped_accum[new_node.inner.rightChild]));
        }
    }
    cout << "done (took " << timer.elapsedString() << " and "
         << memString(sizeof(BVHNode) * m_nodes.size() + sizeof(uint32_t)*m_indices.size())
         << ", SAH cost = " << stats.first
         << ")." << endl;

    m_nodes = std::move(compactified);
}

bool BvhAccel::rayIntersect(const Ray3f &ray_, Intersection &its, bool shadowRay) const {
    uint32_t node_idx = 0, stack_idx = 0, stack[64];

    its.t = std::numeric_limits<float>::infinity();

    /* Use an adaptive ray epsilon */
    Ray3f ray(ray_);
    if (ray.mint == Epsilon)
        ray.mint = std::max(ray.mint, ray.mint * ray.o.array().abs().maxCoeff());

    if (m_nodes.empty() || ray.maxt < ray.mint)
        return false;

    bool foundIntersection = false;  // Was an intersection found so far?
    uint32_t f = (uint32_t) -1;      // Triangle index of the closest intersection

    while (true) {
        const BVHNode &node = m_nodes[node_idx];

        if (!node.bbox.rayIntersect(ray)) {
            if (stack_idx == 0)
                break;
            node_idx = stack[--stack_idx];
            continue;
        }

        if (node.isInner()) {
            stack[stack_idx++] = node.inner.rightChild;
            node_idx++;
            assert(stack_idx<64);
        }
        else {
            for (uint32_t i = node.start(), end = node.end(); i < end; ++i) {
                uint32_t idx = m_indices[i];
                // const Mesh *mesh = m_meshes[BaseInternals::findMesh(*this, idx)];
                const Mesh *mesh = m_pShapes[idx]->getMesh();

                float u, v, t;
               // if (mesh->rayIntersect(idx, ray, u, v, t))
               if (m_pShapes[idx]->rayIntersect(ray, u, v, t))
               {
                    if (shadowRay)
                        return true;
                    ray.maxt = its.t = t;
                    its.uv = Point2f(u, v);
                    its.mesh = mesh;
                    f = idx;
                    foundIntersection = true;
                }
            }
            if (stack_idx == 0)
                break;
            node_idx = stack[--stack_idx];
            continue;
        }
    }

    if (foundIntersection) {
        BaseInternals::getHitAttributes(f, its);
    }

    return foundIntersection;
}

std::string BvhAccel::toString() const
{
    return tfm::format(
            "BVHAcceleration[\n"
            "  node = %s,\n"
            "]",
            m_nodes.size()
    );
}

NORI_REGISTER_CLASS(BvhAccel, XML_ACCELERATION_BVH);
NORI_NAMESPACE_END

