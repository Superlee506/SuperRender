/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/bsdf/microfacetBSDF.h>
#include <nori/core/frame.h>
#include <nori/core/warp.h>

NORI_NAMESPACE_BEGIN

MicrofacetBSDF::MicrofacetBSDF(const PropertyList &propList)
{
    /* RMS surface roughness */
    m_alpha = propList.getFloat("alpha", 0.1f);

    /* Interior IOR (default: BK7 borosilicate optical glass) */
    m_intIOR = propList.getFloat("intIOR", 1.5046f);

    /* Exterior IOR (default: air) */
    m_extIOR = propList.getFloat("extIOR", 1.000277f);

    /* Albedo of the diffuse base material (a.k.a "kd") */
    m_kd = propList.getColor("kd", Color3f(0.5f));

    /* To ensure energy conservation, we must scale the
       specular component by 1-kd.

       While that is not a particularly realistic model of what
       happens in reality, this will greatly simplify the
       implementation. Please see the course staff if you're
       interested in implementing a more realistic version
       of this BRDF. */
    m_ks = 1 - m_kd.maxCoeff();
}

/// Evaluate the BRDF for the given pair of directions
Color3f MicrofacetBSDF::eval(const BSDFQueryRecord &bRec) const
{
    throw NoriException("MicrofacetBRDF::eval(): not implemented!");
}

/// Evaluate the sampling density of \ref sample() wrt. solid angles
float MicrofacetBSDF::pdf(const BSDFQueryRecord &bRec) const
{
    throw NoriException("MicrofacetBRDF::pdf(): not implemented!");
}

/// Sample the BRDF
Color3f MicrofacetBSDF::sample(BSDFQueryRecord &bRec, const Point2f &_sample) const
{
    throw NoriException("MicrofacetBRDF::sample(): not implemented!");

    // Note: Once you have implemented the part that computes the scattered
    // direction, the last part of this function should simply return the
    // BRDF value divided by the solid angle density and multiplied by the
    // cosine factor from the reflection equation, i.e.
    // return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);
}

bool MicrofacetBSDF::isDiffuse() const
{
    /* While microfacet BRDFs are not perfectly diffuse, they can be
       handled by sampling techniques for diffuse/non-specular materials,
       hence we return true here */
    return true;
}

std::string MicrofacetBSDF::toString() const
{
    return tfm::format(
            "Microfacet[\n"
            "  alpha = %f,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s,\n"
            "  ks = %f\n"
            "]",
            m_alpha,
            m_intIOR,
            m_extIOR,
            m_kd.toString(),
            m_ks
    );
}

NORI_REGISTER_CLASS(MicrofacetBSDF, "microfacet");
NORI_NAMESPACE_END
