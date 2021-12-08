/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once
#include <nori/core/common.h>
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

/// Ideal dielectric BSDF
class DielectricBSDF : public BSDF {
public:
    DielectricBSDF(const PropertyList &propList);

    virtual Color3f eval(const BSDFQueryRecord &) const override;

    virtual float pdf(const BSDFQueryRecord &) const override;

    virtual Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const override;

    virtual std::string toString() const override;

private:
    float m_intIOR, m_extIOR;
};

NORI_NAMESPACE_END
