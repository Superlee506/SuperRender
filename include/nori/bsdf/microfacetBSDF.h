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

    virtual void addChild(NoriObject *pChildObj, const std::string &name) override;

    virtual void activate() override;

private:
    static float beckmannD(const Normal3f & M, float alpha);
    static float smithBeckmannG1(const Vector3f & V, const Normal3f & M, float alpha);

private:
    std::unique_ptr<Texture> m_pAlpha;
    float m_intIOR, m_extIOR;
    std::unique_ptr<Texture> m_pKd;
    float m_eta, m_invEta;
};


NORI_NAMESPACE_END
