/*
    This file is part of Nori, a simple educational ray tracer
    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/acceleration/accel.h>
#include <Eigen/Geometry>

NORI_NAMESPACE_BEGIN
Accel::Accel() { m_meshOffset.push_back(0u); }
Accel::~Accel() { }

uint32_t Accel::BaseInternals::findMesh(Accel const &accel, uint32_t &idx) {
    auto it = std::lower_bound(accel.m_meshOffset.begin(), accel.m_meshOffset.end(), idx + 1) - 1;
    idx -= *it;
    return (uint32_t)(it - accel.m_meshOffset.begin());
}

void Accel::BaseInternals::getHitAttributes(const uint32_t& hitIndexThisMesh, Intersection& its){
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
    uint32_t idx0 = F(0, hitIndexThisMesh), idx1 = F(1, hitIndexThisMesh), idx2 = F(2, hitIndexThisMesh);
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


//// Return an axis-aligned bounding box containing the given triangle
BoundingBox3f Accel::BaseInternals::getBoundingBox(Accel const &accel, uint32_t index) {
    uint32_t meshIdx = findMesh(accel, index);
    return accel.m_meshes[meshIdx]->getBoundingBox(index);
}

//// Return the centroid of the given triangle
Point3f Accel::BaseInternals::getCentroid(Accel const &accel, uint32_t index) {
    uint32_t meshIdx = findMesh(accel, index);
    return accel.m_meshes[meshIdx]->getCentroid(index);
}


void Accel::addMesh(Mesh *mesh) {
    m_meshes.push_back(mesh);
    m_meshOffset.push_back(m_meshOffset.back() + mesh->getTriangleCount());
    m_bbox.expandBy(mesh->getBoundingBox());
}

void Accel::build() {
    /* Nothing to do here for now */
}

bool Accel::rayIntersect(const Ray3f &ray_, Intersection &its, bool shadowRay) const {
    bool foundIntersection = false;  // Was an intersection found so far?
    uint32_t f = (uint32_t) -1;      // Triangle index of the closest intersection

    Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)

    /* Brute force search through all triangles */
    for(auto& pMehs: m_meshes)
    {
        if(pMehs == nullptr)
        {
            continue;
        }
        for (uint32_t idx = 0; idx < pMehs->getTriangleCount(); ++idx) {
            float u, v, t;
            if (pMehs->rayIntersect(idx, ray, u, v, t)) {
                /* An intersection was found! Can terminate
                   immediately if this is a shadow ray query */
                if (shadowRay)
                    return true;
                ray.maxt = its.t = t;
                its.uv = Point2f(u, v);
                its.mesh = pMehs;
                f = idx;
                foundIntersection = true;
            }
        }
    }

    if (foundIntersection) {
        BaseInternals::getHitAttributes(f, its);
    }

    return foundIntersection;
}

NORI_NAMESPACE_END