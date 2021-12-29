//
// Created by superqqli on 2021/12/13.
//

#include <nori/integrator/aoIntegration.h>
#include <nori/core/intersection.h>
#include <nori/core/scene.h>
#include <nori/core/warp.h>
#include <nori/core/sampler.h>

NORI_NAMESPACE_BEGIN

AoIntegrator::AoIntegrator(const PropertyList & propList)
{
    m_alpha = propList.getFloat(XML_INTEGRATOR_AO_ALPHA, DEFAULT_INTEGRATOR_AO_ALPHA);
    m_sampleCount = uint32_t(propList.getInteger(XML_INTEGRATOR_AO_SAMPLE_COUNT, DEFAULT_INTEGRATOR_AO_SAMPLE_COUNT));
    m_invSampleCount = 1.0f / float(m_sampleCount);
}

Color3f AoIntegrator::li(const Scene * pScene, Sampler * pSampler, const Ray3f & ray) const
{
    /* Find the surface that is visible in the requested direction */
    Intersection its;
    if (!pScene->rayIntersect(ray, its))
        return Color3f(0.0f);
    Color3f li(0.0f);
    for (uint32_t i = 0; i < m_sampleCount; i++)
    {
        Vector3f wo = Warp::squareToCosineHemisphere(pSampler->next2D());
        Vector3f destPoint = its.p + m_alpha * its.shFrame.toWorld(wo);

        Ray3f aoRay;
        aoRay.o = its.p;
        aoRay.d = destPoint - aoRay.o;
        aoRay.maxt = 1.0f;
        aoRay.mint = 0.0f;
        aoRay.update();
        aoRay.applyPositionBias(its.geoFrame.n, Epsilon);

        if (!pScene->rayIntersect(aoRay))
        {
            li += Color3f(1.0f);
        }
    }
    return li * m_invSampleCount;
}

std::string AoIntegrator::toString() const
{
    return tfm::format(
            "AoIntegrator[alpha = %.4f, sampleCount = %d]",
            m_alpha,
            m_sampleCount
    );
}

NORI_REGISTER_CLASS(AoIntegrator, "ao");
NORI_NAMESPACE_END
