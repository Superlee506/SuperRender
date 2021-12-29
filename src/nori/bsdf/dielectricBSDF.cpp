/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/bsdf/dielectricBSDF.h>
#include <nori/core/texture.h>
#include <nori/core/frame.h>
#include <nori/core/bsdfQueryRecord.h>

NORI_NAMESPACE_BEGIN

DielectricBSDF::DielectricBSDF(const PropertyList &propList)
{
    /* Interior IOR (default: BK7 borosilicate optical glass) */
    m_intIOR = propList.getFloat(XML_BSDF_DIELECTRIC_INT_IOR, DEFAULT_BSDF_DIELECTRIC_INT_IOR);

    /* Exterior IOR (default: air) */
    m_extIOR = propList.getFloat(XML_BSDF_DIELECTRIC_EXT_IOR, DEFAULT_BSDF_DIELECTRIC_EXT_IOR);

    /* Specular reflectance */
    m_pKsReflect.reset(new ConstantColor3fTexture(propList.getColor(XML_BSDF_DIELECTRIC_KS_REFLECT, DEFAULT_BSDF_DIELECTRIC_KS_REFLECT)))  ;

    /* Specular transmittance */
    m_pKsRefract.reset(new ConstantColor3fTexture(propList.getColor(XML_BSDF_DIELECTRIC_KS_REFRACT, DEFAULT_BSDF_DIELECTRIC_KS_REFRACT)));

    m_eta = m_intIOR / m_extIOR;
    m_invEta = 1.0f / m_eta;
}

DielectricBSDF::~DielectricBSDF()
{

}

Color3f DielectricBSDF::eval(const BSDFQueryRecord & bRec) const
{
    float CosThetaI = Frame::cosTheta(bRec.wi);
    float CosThetaO = Frame::cosTheta(bRec.wo);
    float CosThetaT;

    float FresnelTerm = fresnelDielectric(CosThetaI, m_eta, m_invEta, CosThetaT);

    if (CosThetaI * CosThetaO >= 0.0f)
    {
        if (std::abs(reflect(bRec.wi).dot(bRec.wo) - 1.0f) <= DeltaEpsilon)
        {
            return m_pKsReflect->eval(bRec.its) * FresnelTerm;
        }
    }
    else
    {
        if (std::abs(refract(bRec.wi, CosThetaT, m_eta, m_invEta).dot(bRec.wo) - 1.0f) <= DeltaEpsilon)
        {
            float factor = (bRec.mode == ETransportMode::ERadiance) ? (CosThetaT < 0.0f ? m_invEta : m_eta) : 1.0f;
            return m_pKsRefract->eval(bRec.its) * (factor * factor) * (1.0f - FresnelTerm);
        }
    }

    return Color3f(0.0f);
}

float DielectricBSDF::pdf(const BSDFQueryRecord & bRec) const
{
    float cosThetaI = Frame::cosTheta(bRec.wi);
    float cosThetaO = Frame::cosTheta(bRec.wo);
    float cosThetaT;

    float fresnelTerm = fresnelDielectric(cosThetaI, m_eta, m_invEta, cosThetaT);

    if (cosThetaI * cosThetaO >= 0.0f)
    {
        if (std::abs(reflect(bRec.wi).dot(bRec.wo) - 1.0f) <= DeltaEpsilon)
        {
            return fresnelTerm;
        }
    }
    else
    {
        if (std::abs(refract(bRec.wi, cosThetaT, m_eta, m_invEta).dot(bRec.wo) - 1.0f) <= DeltaEpsilon)
        {
            return 1.0f - fresnelTerm;
        }
    }

    return 0.0f;
}

Color3f DielectricBSDF::sample(BSDFQueryRecord &bRec, const Point2f &sample) const
{
    bRec.measure = EMeasure::EDiscrete;

    float cosThetaI = Frame::cosTheta(bRec.wi);
    float cosThetaT;
    float fresnelTerm = fresnelDielectric(cosThetaI, m_eta, m_invEta, cosThetaT);

    // Reflection
    if (sample.x() < fresnelTerm)
    {
        bRec.wo = reflect(bRec.wi);
        bRec.eta = 1.0f;

        return m_pKsReflect->eval(bRec.its);
    }
        // Refraction
    else
    {
        bRec.wo = refract(bRec.wi, cosThetaT, m_eta, m_invEta);
        bRec.eta = cosThetaT < 0.0f ? m_eta : m_invEta;
        /* Radiance must be scaled to account for the solid angle compression
        that occurs when crossing the interface. */
        float Factor = (bRec.mode == ETransportMode::ERadiance) ? (cosThetaT < 0.0f ? m_invEta : m_eta) : 1.0f;
        return m_pKsRefract->eval(bRec.its) * (Factor * Factor);
    }
}

std::string DielectricBSDF::toString() const
{
    return tfm::format(
            "Dielectric[\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  ksReflect = %s,\n"
            "  ksRefract = %s\n"
            "]",
            m_intIOR,
            m_extIOR,
            m_pKsReflect->isConstant() ? m_pKsReflect->getAverage().toString() : indent(m_pKsReflect->toString()),
            m_pKsRefract->isConstant() ? m_pKsRefract->getAverage().toString() : indent(m_pKsRefract->toString())
    );
}

void DielectricBSDF::addChild(NoriObject* pChildObj, const std::string & name)
{
    if (pChildObj->getClassType() == EClassType::ETexture && name == XML_BSDF_DIELECTRIC_KS_REFLECT)
    {
        if (m_pKsReflect->isConstant())
        {
            m_pKsReflect.release() ;
            m_pKsReflect.reset((Texture *)(pChildObj));
            if (m_pKsReflect->isMonochromatic())
            {
                LOG(WARNING) << "KsReflect texture is monochromatic! Make sure that it is done intentionally.";
            }
        }
        else
        {
            throw NoriException("DielectricBSDF: tried to specify multiple KsReflect texture");
        }
    }
    else if (pChildObj->getClassType() == EClassType::ETexture && name == XML_BSDF_DIELECTRIC_KS_REFRACT)
    {
        if (m_pKsRefract->isConstant())
        {
            m_pKsRefract.release();
            m_pKsRefract.reset((Texture *)(pChildObj));
            if (m_pKsRefract->isMonochromatic())
            {
                LOG(WARNING) << "KsRefract texture is monochromatic! Make sure that it is done intentionally.";
            }
        }
        else
        {
            throw NoriException("DielectricBSDF: tried to specify multiple KsRefract texture");
        }
    }
    else
    {
        throw NoriException("DielectricBSDF::AddChild(<%s>, <%s>) is not supported!",
                              classTypeName(pChildObj->getClassType()), name
        );
    }
}

void DielectricBSDF::activate()
{
    addBsdfType(EBSDFType::EDeltaReflection);
    addBsdfType(EBSDFType::EDeltaTransmission);
    if (!m_pKsRefract->isConstant() || !m_pKsReflect->isConstant())
    {
        addBsdfType(EBSDFType::EUVDependent);
    }
}

NORI_REGISTER_CLASS(DielectricBSDF, XML_BSDF_DIELECTRIC);
NORI_NAMESPACE_END
