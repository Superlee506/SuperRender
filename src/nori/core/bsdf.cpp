//
// Created by superqqli on 2021/12/13.
//
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

NoriObject::EClassType BSDF::getClassType() const { return EBSDF; }

bool BSDF::isDiffuse() const { return false; }

bool BSDF::isAnisotropic() const
{
    return false;
}

void BSDF::addBsdfType(uint32_t Type)
{
    m_combinedType |= Type;
}

bool BSDF::hasBsdfType(EBSDFType Type) const
{
    return (uint32_t(Type) & m_combinedType) != 0;
}

uint32_t BSDF::getBsdfTypes() const
{
    return m_combinedType;
}


NORI_NAMESPACE_END