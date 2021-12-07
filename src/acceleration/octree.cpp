/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/acceleration/octree.h>
#include <Eigen/Geometry>
#include<chrono>
#include <queue>
NORI_NAMESPACE_BEGIN


bool OctreeAccel::traverse(size_t nodeIndex, bool shadow, Ray3f& ray, Intersection& its, uint32_t& hitIdx) const {
    const OctNode& node = m_tree[nodeIndex];
    if (!node.bbox.rayIntersect(ray)) {
        return false;
    }
    bool isHit = false;
    if (node.child == 0) {//Leaf node: traverse all the Primitives
        for (uint32_t idx : node.indice) {
            float u, v, t;
            const uint32_t meshIndex = BaseInternals::findMesh(*this, idx);
            // Just need the closest hit point
            if (m_meshes[meshIndex]->rayIntersect(idx, ray, u, v, t) && t < ray.maxt) {
                if (shadow) {
                    return true;
                }
                ray.maxt = t;//设置更近的碰撞点
                its.t = t;
                its.uv = Point2f(u, v);
                its.mesh = m_meshes[meshIndex];
                hitIdx = idx;
                isHit = true;
            }
        }
    } else {// Not the leaf node
        std::pair<size_t, float> children[8] = {};
        for (size_t i = 0; i < 8; i++) {
            size_t childPtr = node.child + i;
            children[i] = std::make_pair(childPtr, m_tree[childPtr].bbox.distanceTo(ray.o));
        }
        //sort the bbox by by the distance
        std::sort(children, children + 8, [](const auto& l, const auto& r) { return l.second < r.second; });
        for (const auto& ch : children) {
            isHit |= traverse(ch.first, shadow, ray, its, hitIdx);
            if (shadow && isHit) {
                return true;
            }
        }
    }
    return isHit;
}


void OctreeAccel::addPrimitive(uint32_t idx) {
    uint32_t idxThisMesh = idx;
    const uint32_t meshIndex = BaseInternals::findMesh(*this, idxThisMesh);
    auto triBox = m_meshes[meshIndex]->getBoundingBox(idxThisMesh);//The bounding box of processing triangle
    std::queue<std::pair<size_t, int>> q;//<Node index, Node depth>
    q.push(std::make_pair(1, 1));//Start from the root node
    while (!q.empty()) {
        auto [ptr, dep] = q.front();
        q.pop();
        if (!m_tree[ptr].bbox.overlaps(triBox)) {// triangle not in the Bbox
            continue;
        }
        m_maxDepth = std::max(m_maxDepth, (size_t)dep);//Update depth
        if (m_tree[ptr].child == 0) {//Leaf node
            m_tree[ptr].indice.emplace_back(idx);//Add the triangle to the node
            if (m_tree[ptr].indice.size() < MAX_PRIMITIVE_NUM) {// not exceed the max num of triangles
                continue;
            }
            if (dep >= MAX_TREE_DEPTH) {//if reach the max depth, not add new node
                continue;
            }
            m_nodeNum += MAX_CHILD_NUM;//Total num + 8
            m_leafNum += (MAX_CHILD_NUM - 1);//because this node is not leaf node anymore
            m_tree[ptr].child = m_tree.size();//Update the index of child node
            for (int i = 0; i < MAX_CHILD_NUM; i++) {// Update the bbox of child node
                Vector3f minPoint;
                Vector3f maxPoint;
                Vector3f center = m_tree[ptr].bbox.getCenter();
                Vector3f corner = m_tree[ptr].bbox.getCorner(i);
                for (size_t j = 0; j < 3; j++) {
                    minPoint[j] = std::min(center[j], corner[j]);
                    maxPoint[j] = std::max(center[j], corner[j]);
                }
                BoundingBox3f childBox(minPoint, maxPoint);
                m_tree.emplace_back(OctNode{0, childBox});
            }
            for (uint32_t primitiveIdx : m_tree[ptr].indice) {//Add the triangle to children's node
                for (size_t chPtr = m_tree[ptr].child; chPtr < m_tree[ptr].child + 8; chPtr++) {
                    if (m_tree.at(chPtr).bbox.overlaps(m_meshes[meshIndex]->getBoundingBox(primitiveIdx))) {
                        m_tree.at(chPtr).indice.emplace_back(primitiveIdx);
                    }
                }
            }
            m_tree[ptr].indice.clear();//Clean the node's triangle indices
            m_tree[ptr].indice.shrink_to_fit();
        } else {
            for (size_t i = 0; i < 8; i++) {//Not the leaf node
                q.push(std::make_pair(m_tree[ptr].child + i, dep + 1));
            }
        }
    }
}

void OctreeAccel::build() {
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    m_tree.clear();
    m_tree.emplace_back(OctNode()); // the first node is null
    m_tree.emplace_back(OctNode{0, m_bbox});// root
    uint32_t globalId = 0;
    for(uint32_t meshId = 0; meshId < m_meshes.size(); meshId++)
    {
        if(m_meshes[meshId] == nullptr)
        {
            continue;
        }
        for (uint32_t idx = 0; idx < m_meshes[meshId]->getTriangleCount(); ++idx) {//Add every triangle to the OctTree
            this->addPrimitive(globalId);
            globalId++;
        }
    }


    auto end = high_resolution_clock::now();
    std::cout << "OctTree build time:" << duration_cast<milliseconds>(end - start).count() << "ms\n";
    std::cout << "max depth:" << m_maxDepth << "\n";
    std::cout << "node count:" << m_nodeNum + 1 << "\n";
    std::cout << "leaf count:" << m_leafNum + 1 << "\n";
}

bool OctreeAccel::rayIntersect(const Ray3f &ray_, Intersection &its, bool shadowRay) const {
    Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)
    uint32_t hitIdx = (uint32_t) -1;      // Triangle index of the closest intersection
    bool foundIntersection = this->traverse(1, shadowRay, ray, its, hitIdx);
    if(foundIntersection){
        BaseInternals::getHitAttributes(hitIdx, its);
    }
    return foundIntersection;
}

NORI_NAMESPACE_END

