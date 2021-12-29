/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/core/object.h>

NORI_NAMESPACE_BEGIN

void NoriObject::addChild(NoriObject *, const std::string & name) {
    throw NoriException(
        "NoriObject::addChild() is not implemented for objects of type '%s'!",
        classTypeName(getClassType()));
}

void NoriObject::activate() { /* Do nothing */ }

void NoriObject::setParent(NoriObject *, const std::string &) { /* Do nothing */ }

std::string NoriObject::classTypeName(EClassType type) {
    switch (type)
    {
        case EScene:                return "scene";
        case EMesh:                 return "mesh";
        case EBSDF:                 return "bsdf";
        case EPhaseFunction:        return "phaseFunction";
        case EEmitter:              return "emitter";
        case ETexture:              return "texture";
        case EMedium:               return "medium";
        case ECamera:               return "camera";
        case EIntegrator:           return "integrator";
        case ESampler:              return "sampler";
        case ETest:                 return "test";
        case EReconstructionFilter: return "reconstructionFilter";
        case EAcceleration:         return "acceleration";
        case EShape:                return "shape";
        default:                    return "<unknown>";
    }
}

std::map<std::string, NoriObjectFactory::Constructor> *NoriObjectFactory::m_constructors = nullptr;

void NoriObjectFactory::registerClass(const std::string &name, const Constructor &constr) {
    if (!m_constructors)
        m_constructors = new std::map<std::string, NoriObjectFactory::Constructor>();
    (*m_constructors)[name] = constr;
}

NORI_NAMESPACE_END
