/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/
#pragma once
#include <nori/core/common.h>
#include <nori/core/bsdf.h>


NORI_NAMESPACE_BEGIN

/**
 * \brief Diffuse / Lambertian BRDF model
 */
class DiffuseBSDF : public BSDF {
public:
    DiffuseBSDF(const PropertyList &propList);

    /// Evaluate the BRDF model
    virtual Color3f eval(const BSDFQueryRecord &bRec) const override;

    /// Compute the density of \ref sample() wrt. solid angles
    virtual float pdf(const BSDFQueryRecord &bRec) const override;

    /// Draw a a sample from the BRDF model
    virtual Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const override;

    virtual bool isDiffuse() const override;

    /// Return a human-readable summary
    virtual std::string toString() const override;

    virtual void addChild(NoriObject * pChildObj, const std::string & name) override;

    virtual void activate() override;

private:
    std::unique_ptr<Texture> m_pAlbedo;
};

NORI_NAMESPACE_END
