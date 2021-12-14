//
// Created by superqqli on 2021/12/13.
//
#include <nori/core/emitter.h>


NORI_NAMESPACE_BEGIN

NoriObject::EClassType Emitter::getClassType() const
{
    return EClassType::EEmitter;
}

void Emitter::setMesh(Mesh * pMesh)
{
    m_pMesh = pMesh;
}

EEmitterType Emitter::getEmitterType() const
{
    return m_Type;
}

bool Emitter::isDelta() const
{
    return m_Type == EEmitterType::EPoint;
}

NORI_NAMESPACE_END