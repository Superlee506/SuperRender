//
// Created by superqqli on 2021/12/10.
//
#include <nori/core/shape.h>
#include <nori/core/intersection.h>

NORI_NAMESPACE_BEGIN

Mesh * Shape::getMesh() const
{
    return nullptr;
}

uint32_t Shape::getFacetIndex() const
{
    return uint32_t(-1);
}

bool Shape::isEmitter() const
{
    return false;
}

Emitter * Shape::getEmitter()
{
    return nullptr;
}

const Emitter * Shape::getEmitter() const
{
    return nullptr;
}

void Shape::computeCurvature(const Intersection & isect, float & H, float & K) const
{
    if (!isect.bHasUVPartial)
    {
        H = 0.0f;
        K = 0.0f;
        LOG(ERROR) << "Curvature cannot be computed, since there is no derivative information.";
        return;
    }
    /* Compute the coefficients of the first and second fundamental form */
    float E = isect.dPdU.dot(isect.dPdU);
    float F = isect.dPdU.dot(isect.dPdV);
    float G = isect.dPdV.dot(isect.dPdV);
    float L = -isect.dPdU.dot(isect.dNdU);
    float M = -isect.dPdV.dot(isect.dNdU);
    float N = -isect.dPdV.dot(isect.dNdV);

    float InvDenom = 1.0f / (E * G - F * F);
    K = (L * N - M * M) * InvDenom;
    H = 0.5f * (L * G - 2.0f * M * F + N * E) * InvDenom;
}

NoriObject::EClassType Shape::getClassType() const
{
    return EClassType::EShape;
}

NORI_NAMESPACE_END