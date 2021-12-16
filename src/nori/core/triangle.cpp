//
// Created by superqqli on 2021/12/10.
//
#include <nori/core/triangle.h>
#include <nori/core/mesh.h>
#include <nori/core/emitter.h>
#include <nori/core/intersection.h>
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

Triangle::Triangle()
{

}

Triangle::Triangle(Mesh * pMesh, uint32_t * pFacet, uint32_t iFacet) :
        m_pMesh(pMesh), m_pFacet(pFacet), m_iFacet(iFacet)
{

}

void Triangle::samplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const
{
    /** This method is correct theoretically, but won't pass the test */
    //// Ref : https://blog.csdn.net/noahzuo/article/details/52886447 or <<Graphics Gems>>
    //float SqrY = Sample.y();
    //float Alpha = 1.0f - SqrY;
    //float Beta = (1.0f - Sample.x()) * SqrY;
    //float Gamma = Sample.x() * SqrY;

    float SqrOneMinusEpsilon1 = std::sqrt(1.0f - Sample.x());
    float Alpha = 1.0f - SqrOneMinusEpsilon1;
    float Beta = Sample.y() * SqrOneMinusEpsilon1;
    float Gamma = 1.0f - Alpha - Beta;

    const MatrixXu & Indices = m_pMesh->getIndices();
    const MatrixXf & Positions = m_pMesh->getVertexPositions();
    const MatrixXf & Normals = m_pMesh->getVertexNormals();

    uint32_t Idx0 = Indices(0, m_iFacet), Idx1 = Indices(1, m_iFacet), Idx2 = Indices(2, m_iFacet);
    Point3f P0 = Positions.col(Idx0), P1 = Positions.col(Idx1), P2 = Positions.col(Idx2);

    P = Gamma * P0 + Alpha * P1 + Beta * P2;

    if (Normals.size() > 0)
    {
        Normal3f N0 = Normals.col(Idx0), N1 = Normals.col(Idx1), N2 = Normals.col(Idx2);
        N = Gamma * N0 + Alpha * N1 + Beta * N2;
    }
}

float Triangle::surfaceArea() const
{
    return m_pMesh->surfaceArea(m_iFacet);
}

BoundingBox3f Triangle::getBoundingBox() const
{
    return m_pMesh->getBoundingBox(m_iFacet);
}

Point3f Triangle::getCentroid() const
{
    return m_pMesh->getCentroid(m_iFacet);
}

bool Triangle::rayIntersect(const Ray3f & Ray, float & U, float & V, float & T) const
{
    return m_pMesh->rayIntersect(m_iFacet, Ray, U, V, T);
}

void Triangle::postIntersect(Intersection & its) const
{
    /* At this point, we now know that there is an intersection,
    and we know the triangle index of the closest such intersection.

    The following computes a number of additional properties which
    characterize the intersection (normals, texture coordinates, etc..)
    */

    its.pEmitter = its.pShape->getEmitter();
    its.pBSDF = its.pShape->getBSDF();

    /* Find the barycentric coordinates */
    Vector3f Barycentric;
    Barycentric << 1 - its.uv.sum(), its.uv;
    its.uv = Point2f(0.0f);

    /* References to all relevant mesh buffers */
    const Mesh * pMesh = its.pShape->getMesh();
    const MatrixXf & V = pMesh->getVertexPositions();
    const MatrixXf & N = pMesh->getVertexNormals();
    const MatrixXf & UV = pMesh->getVertexTexCoords();
    const MatrixXu & F = pMesh->getIndices();

    /* Vertex indices of the triangle */
    uint32_t Idx0 = F(0, m_iFacet), Idx1 = F(1, m_iFacet), Idx2 = F(2, m_iFacet);
    Point3f P0 = V.col(Idx0), P1 = V.col(Idx1), P2 = V.col(Idx2);

    /* Compute the intersection positon accurately using barycentric coordinates */
    its.p = Barycentric.x() * P0 + Barycentric.y() * P1 + Barycentric.z() * P2;

    /* Compute proper texture coordinates if provided by the mesh */
    if (UV.size() > 0)
    {
        its.uv = Barycentric.x() * UV.col(Idx0) + Barycentric.y() * UV.col(Idx1) + Barycentric.z() * UV.col(Idx2);
    }

    /* Compute the geometry frame */
    its.geoFrame = Frame((P1 - P0).cross(P2 - P0).normalized());

    if (N.size() > 0 && UV.size() > 0)
    {
        Point2f UV0 = UV.col(Idx0), UV1 = UV.col(Idx1), UV2 = UV.col(Idx2);
        Vector3f dP1 = P1 - P0, dP2 = P2 - P0;
        Vector2f dUV1 = UV1 - UV0, dUV2 = UV2 - UV0;
        float Length = dP1.cross(dP2).norm();

        if (Length == 0)
        {
            its.shFrame = its.geoFrame;
        }
        else
        {
            float Det = dUV1.x() * dUV2.y() - dUV1.y() * dUV2.x();

            if (Det == 0.0f)
            {
                // Triangle degeneration

                its.shFrame = Frame(
                        (Barycentric.x() * N.col(Idx0) + Barycentric.y() * N.col(Idx1) + Barycentric.z() * N.col(Idx2)).normalized()
                );

                its.dPdU = its.shFrame.s;
                its.dPdV = its.shFrame.t;
                its.dNdU = Vector3f(0.0f);
                its.dNdV = Vector3f(0.0f);
                its.bHasUVPartial = true;
            }
            else
            {
                float InvDet = 1.0f / Det;
                Vector3f dPdU = ( dUV2.y() * dP1 - dUV1.y() * dP2) * InvDet;
                Vector3f dPdV = (-dUV2.x() * dP1 + dUV1.x() * dP2) * InvDet;

                Normal3f NShade = Barycentric.x() * N.col(Idx0) + Barycentric.y() * N.col(Idx1) + Barycentric.z() * N.col(Idx2);
                float Ln = NShade.norm();
                float InvLn = 1.0f / Ln;
                NShade *= InvLn;

                Vector3f dNdU = (N.col(Idx1) - N.col(Idx0)) * InvLn;
                dNdU -= NShade * NShade.dot(dNdU);
                Vector3f dNdV = (N.col(Idx2) - N.col(Idx0)) * InvLn;
                dNdV -= NShade * NShade.dot(dNdV);

                its.dNdU = ( dUV2.y() * dNdU - dUV1.y() * dNdV) * InvDet;
                its.dNdV = (-dUV2.x() * dNdU + dUV1.x() * dNdV) * InvDet;
                its.dPdU = dPdU;
                its.dPdV = dPdV;
                its.bHasUVPartial = true;

                its.shFrame = Frame(NShade, dPdU);
            }
        }
    }
    else if (N.size() > 0)
    {
        its.shFrame = Frame(
                (Barycentric.x() * N.col(Idx0) + Barycentric.y() * N.col(Idx1) + Barycentric.z() * N.col(Idx2)).normalized()
        );
    }
    else
    {
        its.shFrame = its.geoFrame;
    }
}

Mesh * Triangle::getMesh() const
{
    return m_pMesh;
}

uint32_t Triangle::getFacetIndex() const
{
    return m_iFacet;
}

bool Triangle::isEmitter() const
{
    return m_pMesh->isEmitter();
}

Emitter * Triangle::getEmitter()
{
    return m_pMesh->getEmitter();
}

const Emitter * Triangle::getEmitter() const
{
    return m_pMesh->getEmitter();
}

const BSDF * Triangle::getBSDF() const
{
    return m_pMesh->getBSDF();
}

std::string Triangle::toString() const
{
    const MatrixXf & V = m_pMesh->getVertexPositions();
    uint32_t iV0 = m_pFacet[0];
    uint32_t iV1 = m_pFacet[1];
    uint32_t iV2 = m_pFacet[2];
    Point3f P0 = V.col(iV0);
    Point3f P1 = V.col(iV1);
    Point3f P2 = V.col(iV2);

    return tfm::format(
            "Triangle[\n"
            "  v0 = %s, v1 = %s, v2 = %s,\n"
            "  emmiter = %s,\n"
            "  BSDF = %s"
            "]",
            P0.toString(),
            P1.toString(),
            P2.toString(),
            isEmitter() ? indent(getEmitter()->toString()) : "<null>",
            indent(getBSDF()->toString())
    );
}

NORI_NAMESPACE_END

