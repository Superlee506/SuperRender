/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/bsdf/dielectricBSDF.h>

NORI_NAMESPACE_BEGIN

DielectricBSDF::DielectricBSDF(const PropertyList &propList)
{
    /* Interior IOR (default: BK7 borosilicate optical glass) */
    m_intIOR = propList.getFloat("intIOR", 1.5046f);

    /* Exterior IOR (default: air) */
    m_extIOR = propList.getFloat("extIOR", 1.000277f);
}

Color3f DielectricBSDF::eval(const BSDFQueryRecord &) const
{
    /* Discrete BRDFs always evaluate to zero in Nori */
    return Color3f(0.0f);
}

float DielectricBSDF::pdf(const BSDFQueryRecord &) const
{
    /* Discrete BRDFs always evaluate to zero in Nori */
    return 0.0f;
}

Color3f DielectricBSDF::sample(BSDFQueryRecord &bRec, const Point2f &sample) const
{
    throw NoriException("Unimplemented!");
}

std::string DielectricBSDF::toString() const
{
    return tfm::format(
            "Dielectric[\n"
            "  intIOR = %f,\n"
            "  extIOR = %f\n"
            "]",
            m_intIOR, m_extIOR);
}
NORI_REGISTER_CLASS(DielectricBSDF, "dielectric");
NORI_NAMESPACE_END
