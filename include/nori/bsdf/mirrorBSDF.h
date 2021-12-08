/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once
#include <nori/core/common.h>
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

/// Ideal mirror BRDF
class MirrorBSDF : public BSDF {
public:
    MirrorBSDF(const PropertyList &);

    virtual Color3f eval(const BSDFQueryRecord &) const override;

    virtual float pdf(const BSDFQueryRecord &) const override;

    virtual Color3f sample(BSDFQueryRecord &bRec, const Point2f &) const override;

    virtual std::string toString() const override;
};

NORI_NAMESPACE_END
