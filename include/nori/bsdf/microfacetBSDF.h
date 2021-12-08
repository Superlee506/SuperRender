/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once
#include <nori/core/common.h>
#include <nori/core/bsdf.h>


NORI_NAMESPACE_BEGIN

class MicrofacetBSDF : public BSDF {
public:
    MicrofacetBSDF(const PropertyList &propList);

    /// Evaluate the BRDF for the given pair of directions
    virtual Color3f eval(const BSDFQueryRecord &bRec) const override;

    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    virtual float pdf(const BSDFQueryRecord &bRec) const override;

    /// Sample the BRDF
    virtual Color3f sample(BSDFQueryRecord &bRec, const Point2f &_sample) const override;

    virtual bool isDiffuse() const override;

    virtual std::string toString() const override;

private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    Color3f m_kd;
};


NORI_NAMESPACE_END
