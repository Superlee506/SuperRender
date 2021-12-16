//
// Created by superqqli on 2021/12/13.
//
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

NoriObject::EClassType BSDF::getClassType() const { return EBSDF; }

bool BSDF::isDiffuse() const { return false; }


NORI_NAMESPACE_END