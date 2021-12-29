/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/bsdf/microfacetBSDF.h>
#include <nori/core/frame.h>
#include <nori/core/warp.h>
#include <nori/core/texture.h>
#include <nori/core/bsdfQueryRecord.h>

NORI_NAMESPACE_BEGIN

MicrofacetBSDF::MicrofacetBSDF(const PropertyList &propList)
{
    /* RMS surface roughness */
    m_pAlpha.reset(new ConstantFloatTexture(clamp(propList.getFloat(XML_BSDF_MICROFACET_ALPHA, DEFAULT_BSDF_MICROFACET_ALPHA), float(MIN_ALPHA), float(MAX_ALPHA))));

    /* Interior IOR (default: BK7 borosilicate optical glass) */
    m_intIOR = propList.getFloat(XML_BSDF_MICROFACET_INT_IOR, DEFAULT_BSDF_MICROFACET_INT_IOR);

    /* Exterior IOR (default: air) */
    m_extIOR = propList.getFloat(XML_BSDF_MICROFACET_EXT_IOR, DEFAULT_BSDF_MICROFACET_EXT_IOR);

    /* Albedo of the diffuse base material (a.k.a "kd") */
    m_pKd.reset(new ConstantColor3fTexture(propList.getColor(XML_BSDF_MICROFACET_KD, DEFAULT_BSDF_MICROFACET_ALBEDO)));

    /* To ensure energy conservation, we must scale the
       specular component by 1-kd.

       While that is not a particularly realistic model of what
       happens in reality, this will greatly simplify the
       implementation. Please see the course staff if you're
       interested in implementing a more realistic version
       of this BRDF. */
    m_eta = m_intIOR / m_extIOR;
    m_invEta = 1.0f / m_eta;
}

/// Evaluate the BRDF for the given pair of directions
Color3f MicrofacetBSDF::eval(const BSDFQueryRecord &bRec) const
{
    if (bRec.measure != EMeasure::ESolidAngle ||
        Frame::cosTheta(bRec.wi) <= 0.0f ||
        Frame::cosTheta(bRec.wo) <= 0.0f
            )
    {
        return Color3f(0.0f);
    }

    float alpha = clamp(m_pAlpha->eval(bRec.its)[0], float(MIN_ALPHA), float(MAX_ALPHA));
    Color3f kd = m_pKd->eval(bRec.its);

    /*
    To ensure energy conservation, we must scale the
    specular component by 1-kd.

    While that is not a particularly realistic model of what
    happens in reality, this will greatly simplify the
    implementation.
    */
    float ks = 1.0f - kd.maxCoeff();

    Vector3f wh = (bRec.wi + bRec.wo).normalized();

    float CosThetaT;

    float D = beckmannD(wh, alpha);
    float F = fresnelDielectric(wh.dot(bRec.wi), m_eta, m_invEta, CosThetaT);
    float G = smithBeckmannG1(bRec.wi, wh, alpha) * smithBeckmannG1(bRec.wo, wh, alpha);

    Color3f specularTerm = ks * F * D * G / (4.0f * Frame::cosTheta(bRec.wi) * Frame::cosTheta(bRec.wo));
    if (!specularTerm.isValid())
    {
        specularTerm = Color3f(0.0f);
    }

    Color3f diffuseTerm = kd / float(M_PI);
    return specularTerm + diffuseTerm;
}

/// Evaluate the sampling density of \ref sample() wrt. solid angles
float MicrofacetBSDF::pdf(const BSDFQueryRecord &bRec) const
{
    if (bRec.measure != EMeasure::ESolidAngle ||
        Frame::cosTheta(bRec.wi) <= 0.0f ||
        Frame::cosTheta(bRec.wo) <= 0.0f
            )
    {
        return 0.0f;
    }

    float alpha = clamp(m_pAlpha->eval(bRec.its)[0], float(MIN_ALPHA), float(MAX_ALPHA));
    Color3f kd = m_pKd->eval(bRec.its);

    /*
    To ensure energy conservation, we must scale the
    specular component by 1-kd.

    While that is not a particularly realistic model of what
    happens in reality, this will greatly simplify the
    implementation.
    */
    float ks = 1.0f - kd.maxCoeff();

    Vector3f wh = (bRec.wi + bRec.wo).normalized();
    float J = 0.25f / wh.dot(bRec.wo);
    float specularPdf = ks * Warp::squareToBeckmannPdf(wh, alpha) * J;
    float diffusePdf = (1.0f - ks) * Warp::squareToCosineHemispherePdf(bRec.wo);
    return specularPdf + diffusePdf;
}

/// Sample the BRDF
Color3f MicrofacetBSDF::sample(BSDFQueryRecord &bRec, const Point2f &_sample) const
{
    if (Frame::cosTheta(bRec.wi) <= 0.0f)
    {
        return Color3f(0.0f);
    }

    bRec.measure = EMeasure::ESolidAngle;

    float alpha = clamp(m_pAlpha->eval(bRec.its)[0], float(MIN_ALPHA), float(MAX_ALPHA));
    Color3f kd = m_pKd->eval(bRec.its);

    /*
    To ensure energy conservation, we must scale the
    specular component by 1-kd.

    While that is not a particularly realistic model of what
    happens in reality, this will greatly simplify the
    implementation.
    */
    float ks = 1.0f - kd.maxCoeff();

    if (_sample.x() < ks)
    {
        // Specular reflection
        float reuseSampleX = _sample.x() / ks;
        Normal3f wh = Warp::squareToBeckmann(Point2f(reuseSampleX, _sample.y()), alpha);
        bRec.wo = 2.0f * wh.dot(bRec.wi) * wh - bRec.wi;
    }
    else
    {
        // Diffuse
        float reuseSampleX = (_sample.x() - ks) / (1.0f - ks);
        bRec.wo = Warp::squareToCosineHemisphere(Point2f(reuseSampleX, _sample.y()));
    }

    bRec.eta = 1.0f;

    float PDF = pdf(bRec);
    if (PDF == 0.0f)
    {
        return Color3f(0.0f);
    }

    float cosThetaT;
    Vector3f wh = (bRec.wi + bRec.wo).normalized();

    float D = beckmannD(wh, alpha);
    float F = fresnelDielectric(wh.dot(bRec.wi), m_eta, m_invEta, cosThetaT);
    float G = smithBeckmannG1(bRec.wi, wh, alpha) * smithBeckmannG1(bRec.wo, wh, alpha);

    Color3f specularTerm = ks * F * D * G / (4.0f * Frame::cosTheta(bRec.wi) * Frame::cosTheta(bRec.wo));
    if (!specularTerm.isValid())
    {
        specularTerm = Color3f(0.0f);
    }

    Color3f diffuseTerm = kd / float(M_PI);

    return (specularTerm + diffuseTerm) * Frame::cosTheta(bRec.wo) / PDF;
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
            "  alpha = %s,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s\n"
            "]",
            m_pAlpha->isConstant() ? std::to_string(m_pAlpha->getAverage()[0]) : indent(m_pAlpha->toString()),
            m_intIOR,
            m_extIOR,
            m_pKd->isConstant() ? m_pKd->getAverage().toString() : indent(m_pKd->toString())
    );
}


void MicrofacetBSDF::addChild(NoriObject * pChildObj, const std::string & name)
{
    if (pChildObj->getClassType() == EClassType::ETexture && name == XML_BSDF_MICROFACET_KD)
    {
        if (m_pKd->isConstant())
        {
            m_pKd.release();
            m_pKd.reset((Texture *)(pChildObj));
            if (m_pKd->isMonochromatic())
            {
                LOG(WARNING) << "Kd texture is monochromatic! Make sure that it is done intentionally.";
            }
        }
        else
        {
            throw NoriException("MicrofacetBSDF: tried to specify multiple kd texture");
        }
    }
    else if (pChildObj->getClassType() == EClassType::ETexture && name == XML_BSDF_MICROFACET_ALPHA)
    {
        if (m_pAlpha->isConstant())
        {
            m_pAlpha.release() ;
            m_pAlpha.reset((Texture *)(pChildObj));
            if (!m_pAlpha->isMonochromatic())
            {
                LOG(WARNING) << "Alpha texture is not monochromatic, only R channel will be used.";
            }
        }
        else
        {
            throw NoriException("MicrofacetBSDF: tried to specify multiple alpha texture");
        }
    }
    else
    {
        throw NoriException("MicrofacetBSDF::addChild(<%s>, <%s>) is not supported!",
                            classTypeName(pChildObj->getClassType()), name
        );
    }
}

void MicrofacetBSDF::activate()
{
    addBsdfType(EBSDFType::EDiffuseReflection);
    addBsdfType(EBSDFType::EGlossyReflection);
    if (!m_pAlpha->isConstant() || !m_pKd->isConstant())
    {
        addBsdfType(EBSDFType::EUVDependent);
    }
}

float MicrofacetBSDF::beckmannD(const Normal3f & M, float alpha)
{
    float Expon = Frame::tanTheta(M) / alpha;
    float CosTheta = Frame::cosTheta(M);
    float CosTheta2 = CosTheta * CosTheta;
    return std::exp(-Expon * Expon) / (float(M_PI) * alpha * alpha * CosTheta2 * CosTheta2);
}

float MicrofacetBSDF::smithBeckmannG1(const Vector3f & V, const Normal3f & M, float alpha)
{
    float TanTheta = Frame::tanTheta(V);

    // Perpendicular indidence
    if (TanTheta == 0.0f)
    {
        return 1.0f;
    }

    // Backside
    if (M.dot(V) * Frame::cosTheta(V) <= 0.0f)
    {
        return 0.0f;
    }

    float B = 1.0f / (alpha * TanTheta);
    if (B >= 1.6f)
    {
        return 1.0f;
    }

    float B2 = B * B;
    return (3.535f * B + 2.181f * B2) / (1.0f + 2.276f * B + 2.577f * B2);
}

NORI_REGISTER_CLASS(MicrofacetBSDF, XML_BSDF_MICROFACET);
NORI_NAMESPACE_END
