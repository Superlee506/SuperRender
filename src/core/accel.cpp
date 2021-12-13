/*
    This file is part of Nori, a simple educational ray tracer
    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/core/accel.h>
#include <nori/core/intersection.h>
#include <nori/core/mesh.h>
#include <nori/core/triangle.h>


NORI_NAMESPACE_BEGIN
Accel::Accel(const PropertyList & PropList) { m_meshOffset.push_back(0u); }

Accel::~Accel() { }

NoriObject::EClassType Accel::getClassType() const
{
    return EClassType::EAcceleration;
}

std::string Accel::toString() const
{
    return "BrutoLoop[]";
}

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

    m_pShapes.reserve(m_pShapes.size() + mesh->getTriangleCount());
    MatrixXu indices = mesh->getIndices(); // triangle indices
    uint32_t * pData = indices.data();
    Triangle * pTri = m_memoryArena.alloc<Triangle>(indices.cols());
    for (size_t i = 0; i < indices.cols(); i++)
    {
        pTri[i].m_pMesh = mesh;
        pTri[i].m_pFacet = pData + i * 3;
        pTri[i].m_iFacet = uint32_t(i);
        m_pShapes.push_back((PrimitiveShape *)(&pTri[i]));
    }
}

void Accel::build() {
    /* Nothing to do here for now */
}

const BoundingBox3f &Accel::getBoundingBox() const { return m_bbox; }

size_t Accel::getUsedMemoryForPrimitive() const
{
    return m_memoryArena.totalAllocated();
}


bool Accel::rayIntersect(const Ray3f &ray_, Intersection &its, bool bShadowRay) const {
    bool bFoundIntersection = false;  // Was an intersection found so far?
    PrimitiveShape* pHitPrimitive = nullptr;
    Ray3f ray(ray_); /// Make a copy of the ray (we will need to update its '.maxt' value)
    for(size_t i = 0; i < m_pShapes.size(); i++)
    {
        float u, v, t;
        if (m_pShapes[i]->rayIntersect(ray_, u, v, t))
        {
            /* An intersection was found! Can terminate
			immediately if this is a shadow ray query */
            if (bShadowRay)
            {
                return true;
            }

            ray.maxt = its.t = t;
            its.uv = Point2f(u, v);
            its.pShape = m_pShapes[i];

            pHitPrimitive = m_pShapes[i];
            bFoundIntersection = true;
        }
    }

    if (bFoundIntersection)
    {
        pHitPrimitive->postIntersect(its);
        its.computeScreenSpacePartial(ray);
    }
    return bFoundIntersection;
}

NORI_REGISTER_CLASS(Accel, XML_ACCELERATION_BRUTO_LOOP);
NORI_NAMESPACE_END