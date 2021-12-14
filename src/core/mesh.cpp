/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/core/mesh.h>
#include <nori/core/bsdf.h>
#include <nori/core/emitter.h>
#include <nori/core/discretePDF.h>

NORI_NAMESPACE_BEGIN

Mesh::Mesh() { }

Mesh::~Mesh() {
    delete m_bsdf;
}

void Mesh::activate() {
    if (!m_bsdf)
    {
        /* If no material was assigned, instantiate a diffuse BRDF */
        LOG(WARNING) << "No BSDF was specified, create a default BSDF.";
        m_bsdf = static_cast<BSDF *>(
            NoriObjectFactory::createInstance(DEFAULT_MESH_BSDF, PropertyList()));
    }

    std::vector<float> areas(getTriangleCount());
    // Create the pdf (defined by the area of each triangle)
    for (uint32_t i = 0; i < getTriangleCount(); i++)
    {
        float area = surfaceArea(i);
        areas[i] = area;
        m_meshArea += area;
    }
    m_invMeshArea = 1.0f / m_meshArea;

    m_pPDF.reset(new DiscretePDF1D(areas.data(), int(areas.size())));

}

float Mesh::surfaceArea(uint32_t index) const {
    uint32_t i0 = m_F(0, index), i1 = m_F(1, index), i2 = m_F(2, index);

    const Point3f p0 = m_V.col(i0), p1 = m_V.col(i1), p2 = m_V.col(i2);

    return 0.5f * Vector3f((p1 - p0).cross(p2 - p0)).norm();
}

bool Mesh::rayIntersect(uint32_t index, const Ray3f &ray, float &u, float &v, float &t) const {
    uint32_t i0 = m_F(0, index), i1 = m_F(1, index), i2 = m_F(2, index);
    const Point3f p0 = m_V.col(i0), p1 = m_V.col(i1), p2 = m_V.col(i2);

    /* Find vectors for two edges sharing v[0] */
    Vector3f edge1 = p1 - p0, edge2 = p2 - p0;

    /* Begin calculating determinant - also used to calculate U parameter */
    Vector3f pvec = ray.d.cross(edge2);

    /* If determinant is near zero, ray lies in plane of triangle */
    float det = edge1.dot(pvec);

    if (det > -1e-8f && det < 1e-8f)
        return false;
    float inv_det = 1.0f / det;

    /* Calculate distance from v[0] to ray origin */
    Vector3f tvec = ray.o - p0;

    /* Calculate U parameter and test bounds */
    u = tvec.dot(pvec) * inv_det;
    if (u < 0.0 || u > 1.0)
        return false;

    /* Prepare to test V parameter */
    Vector3f qvec = tvec.cross(edge1);

    /* Calculate V parameter and test bounds */
    v = ray.d.dot(qvec) * inv_det;
    if (v < 0.0 || u + v > 1.0)
        return false;

    /* Ray intersects triangle -> compute t */
    t = edge2.dot(qvec) * inv_det;

    return t >= ray.mint && t <= ray.maxt;
}

const MatrixXf &Mesh::getVertexPositions() const
{
    return m_V;
}

const MatrixXf &Mesh::getVertexNormals() const
{
    return m_N;
}

const MatrixXf &Mesh::getVertexTexCoords() const
{
    return m_UV;
}

const MatrixXu &Mesh::getIndices() const
{
    return m_F;
}

bool Mesh::isEmitter() const
{
    return m_emitter != nullptr;
}

Emitter *Mesh::getEmitter()
{
    return m_emitter;
}

const BSDF *Mesh::getBSDF() const
{
    return m_bsdf;
}

const std::string &Mesh::getName() const
{
    return m_name;
}

BoundingBox3f Mesh::getBoundingBox(uint32_t index) const {
    BoundingBox3f result(m_V.col(m_F(0, index)));
    result.expandBy(m_V.col(m_F(1, index)));
    result.expandBy(m_V.col(m_F(2, index)));
    return result;
}

Point3f Mesh::getCentroid(uint32_t index) const {
    return (1.0f / 3.0f) *
        (m_V.col(m_F(0, index)) +
         m_V.col(m_F(1, index)) +
         m_V.col(m_F(2, index)));
}

void Mesh::addChild(NoriObject *obj) {
    switch (obj->getClassType()) {
        case EBSDF:
            if (m_bsdf)
                throw NoriException(
                    "Mesh: tried to register multiple BSDF instances!");
            m_bsdf = static_cast<BSDF *>(obj);
            break;

        case EEmitter:
            {
                Emitter *emitter = static_cast<Emitter *>(obj);
                if (m_emitter)
                    throw NoriException(
                        "Mesh: tried to register multiple Emitter instances!");
                m_emitter = emitter;
                if (m_emitter->getEmitterType() != EEmitterType::EArea)
                {
                    throw NoriException("Mesh: only area light can be attached!");
                }
            }
            break;

        default:
            throw NoriException("Mesh::addChild(<%s>) is not supported!",
                                classTypeName(obj->getClassType()));
    }
}

std::string Mesh::toString() const
{
    return tfm::format(
        "Mesh[\n"
        "  name = \"%s\",\n"
        "  vertexCount = %i,\n"
        "  triangleCount = %i,\n"
        "  bsdf = %s,\n"
        "  emitter = %s\n"
        "]",
        m_name,
        m_V.cols(),
        m_F.cols(),
        m_bsdf ? indent(m_bsdf->toString()) : std::string("null"),
        m_emitter ? indent(m_emitter->toString()) : std::string("null")
    );
}

NoriObject::EClassType Mesh::getClassType() const
{
    return EMesh;
}


void Mesh::samplePosition(float sample1D, const Point2f & sample2D, Point3f & samplePoint, Normal3f & sampleNormal) const
{
    size_t triangleIdx = m_pPDF->sampleDiscrete(sample1D);

    float sqrOneMinusEpsilon1 = std::sqrt(1.0f - sample2D.x());
    float alpha = 1.0f - sqrOneMinusEpsilon1;
    float beta = sample2D.y() * sqrOneMinusEpsilon1;
    float gamma = 1.0f - alpha - beta;

    uint32_t idx0 = m_F(0, triangleIdx), idx1 = m_F(1, triangleIdx), idx2 = m_F(2, triangleIdx);
    Point3f p0 = m_V.col(idx0), p1 = m_V.col(idx1), p2 = m_V.col(idx2);

    samplePoint = gamma * p0 + alpha * p1 + beta * p2;

    if (m_N.size() > 0)
    {
        Normal3f N0 = m_N.col(idx0), N1 = m_N.col(idx1), N2 = m_N.col(idx2);
        sampleNormal = gamma * N0 + alpha * N1 + beta * N2;
    }
    else
    {
        sampleNormal = (p1 - p0).cross(p2 - p0).normalized();
    }
}

float Mesh::pdf() const
{
    return m_invMeshArea;
}

NORI_NAMESPACE_END
