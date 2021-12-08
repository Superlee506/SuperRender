/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/bsdf/mirrorBSDF.h>
#include <nori/core/frame.h>

NORI_NAMESPACE_BEGIN

MirrorBSDF::MirrorBSDF(const PropertyList &) { }

Color3f MirrorBSDF::eval(const BSDFQueryRecord &) const
{
    /* Discrete BRDFs always evaluate to zero in Nori */
    return Color3f(0.0f);
}

float MirrorBSDF::pdf(const BSDFQueryRecord &) const
{
    /* Discrete BRDFs always evaluate to zero in Nori */
    return 0.0f;
}

Color3f MirrorBSDF::sample(BSDFQueryRecord &bRec, const Point2f &) const
{
    if (Frame::cosTheta(bRec.wi) <= 0)
        return Color3f(0.0f);

    // Reflection in local coordinates
    bRec.wo = Vector3f(
            -bRec.wi.x(),
            -bRec.wi.y(),
            bRec.wi.z()
    );
    bRec.measure = EDiscrete;

    /* Relative index of refraction: no change */
    bRec.eta = 1.0f;

    return Color3f(1.0f);
}

std::string MirrorBSDF::toString() const
{
    return "Mirror[]";
}

NORI_REGISTER_CLASS(MirrorBSDF, "mirror");
NORI_NAMESPACE_END
