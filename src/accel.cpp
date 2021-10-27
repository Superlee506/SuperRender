/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/accel.h>
#include <Eigen/Geometry>
#include<chrono>
#include <queue>
NORI_NAMESPACE_BEGIN
void getHitAttributes(const uint32_t& hitIndex, Intersection& its){
    /* At this point, we now know that there is an intersection,
    and we know the triangle index of the closest such intersection.
    The following computes a number of additional properties which
    characterize the intersection (normals, texture coordinates, etc..)
    */
    /* Find the barycentric coordinates */
    Vector3f bary;
    bary << 1-its.uv.sum(), its.uv;
    /* References to all relevant mesh buffers */
    const Mesh *mesh   = its.mesh;
    const MatrixXf &V  = mesh->getVertexPositions();
    const MatrixXf &N  = mesh->getVertexNormals();
    const MatrixXf &UV = mesh->getVertexTexCoords();
    const MatrixXu &F  = mesh->getIndices();
    /* Vertex indices of the triangle */
    uint32_t idx0 = F(0, hitIndex), idx1 = F(1, hitIndex), idx2 = F(2, hitIndex);
    Point3f p0 = V.col(idx0), p1 = V.col(idx1), p2 = V.col(idx2);
    /* Compute the intersection positon accurately
       using barycentric coordinates */
    its.p = bary.x() * p0 + bary.y() * p1 + bary.z() * p2;
    /* Compute proper texture coordinates if provided by the mesh */
    if (UV.size() > 0)
        its.uv = bary.x() * UV.col(idx0) +
                 bary.y() * UV.col(idx1) +
                 bary.z() * UV.col(idx2);
    /* Compute the geometry frame */
    its.geoFrame = Frame((p1-p0).cross(p2-p0).normalized());
    if (N.size() > 0) {
        /* Compute the shading frame. Note that for simplicity,
           the current implementation doesn't attempt to provide
           tangents that are continuous across the surface. That
           means that this code will need to be modified to be able
           use anisotropic BRDFs, which need tangent continuity */
        its.shFrame = Frame(
                (bary.x() * N.col(idx0) +
                 bary.y() * N.col(idx1) +
                 bary.z() * N.col(idx2)).normalized());
    } else {
        its.shFrame = its.geoFrame;
    }
}
bool Accel::traverseBruteForce(bool shadow, Ray3f& ray_, Intersection& its, uint32_t& hitIdx) const{
    bool foundIntersection = false;  // Was an intersection found so far?
    hitIdx = (uint32_t) -1;      // Triangle index of the closest intersection
    Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)
    /* Brute force search through all triangles */
    for (uint32_t idx = 0; idx < m_mesh->getTriangleCount(); ++idx) {
        float u, v, t;
        if (m_mesh->rayIntersect(idx, ray, u, v, t)) {
            /* An intersection was found! Can terminate
               immediately if this is a shadow ray query */
            if (shadow)
                return true;
            ray.maxt = its.t = t;
            its.uv = Point2f(u, v);
            its.mesh = m_mesh;
            hitIdx = idx;
            foundIntersection = true;
        }
    }

    return foundIntersection;
}

bool Accel::traverse(size_t nodeIndex, bool shadow, Ray3f& ray, Intersection& its, uint32_t& hitIdx) const {
    const OctNode& node = m_tree[nodeIndex];
    if (!node.bbox.rayIntersect(ray)) {
        return false;
    }
    bool isHit = false;
    if (node.child == 0) {//Leaf node: traverse all the Primitives
        for (uint32_t idx : node.indice) {
            float u, v, t;
            // Just need the closest hit point
            if (m_mesh->rayIntersect(idx, ray, u, v, t) && t < ray.maxt) {
                if (shadow) {
                    return true;
                }
                ray.maxt = t;//设置更近的碰撞点
                its.t = t;
                its.uv = Point2f(u, v);
                its.mesh = m_mesh;
                hitIdx = idx;
                isHit = true;
            }
        }
    } else {//不是叶节点
        std::pair<size_t, float> children[8] = {};
        for (size_t i = 0; i < 8; i++) {
            size_t childPtr = node.child + i;
            children[i] = std::make_pair(childPtr, m_tree[childPtr].bbox.distanceTo(ray.o));
        }
        //因为距离射线起点的碰撞盒更容易被碰到，所以先排个序
        std::sort(children, children + 8, [](const auto& l, const auto& r) { return l.second < r.second; });
        for (const auto& ch : children) {
            isHit |= traverse(ch.first, shadow, ray, its, hitIdx);//递归求交
            if (shadow && isHit) {
                return true;
            }
        }
    }
    return isHit;
}

void Accel::addMesh(Mesh *mesh) {
    if (m_mesh)
        throw NoriException("Accel: only a single mesh is supported!");
    m_mesh = mesh;
    m_bbox = m_mesh->getBoundingBox();
}

void Accel::addPrimitive(uint32_t idx) {
    auto triBox = m_mesh->getBoundingBox(idx);//The bounding box of processing triangle
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
            for (uint32_t meshIdx : m_tree[ptr].indice) {//Add the triangle to children's node
                for (size_t chPtr = m_tree[ptr].child; chPtr < m_tree[ptr].child + 8; chPtr++) {
                    if (m_tree.at(chPtr).bbox.overlaps(m_mesh->getBoundingBox(meshIdx))) {
                        m_tree.at(chPtr).indice.emplace_back(meshIdx);
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

void Accel::build() {
    if (m_mesh == nullptr) {
        return;
    }
    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    m_tree.clear();
    m_tree.emplace_back(OctNode()); // the first node is null
    m_tree.emplace_back(OctNode{0, m_mesh->getBoundingBox()});// root
    for (uint32_t idx = 0; idx < m_mesh->getTriangleCount(); ++idx) {//Add every triangle to the OctTree
        this->addPrimitive(idx);
    }
    auto end = high_resolution_clock::now();
    std::cout << "OctTree build time:" << duration_cast<milliseconds>(end - start).count() << "ms\n";
    std::cout << "max depth:" << m_maxDepth << "\n";
    std::cout << "node count:" << m_nodeNum + 1 << "\n";
    std::cout << "leaf count:" << m_leafNum + 1 << "\n";
}

bool Accel::rayIntersect(const Ray3f &ray_, Intersection &its, bool shadowRay) const {
    Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)
    uint32_t hitTriangleIndex = (uint32_t) -1;      // Triangle index of the closest intersection
    // bool foundIntersection = this->traverse(1, shadowRay, ray, its, hitTriangleIndex);
    bool foundIntersection = this->traverseBruteForce(shadowRay, ray, its, hitTriangleIndex);
    if(foundIntersection){
       getHitAttributes(hitTriangleIndex, its);
    }
    return foundIntersection;
}

NORI_NAMESPACE_END

