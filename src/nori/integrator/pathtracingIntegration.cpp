//
// Created by superqqli on 2021/12/13.
//

#include <nori/integrator/pathtracingIntegration.h>
#include <nori/core/intersection.h>
#include <nori/core/scene.h>
#include <nori/core/warp.h>
#include <nori/core/sampler.h>
#include <nori/core/emitterQueryRecord.h>
#include <nori/core/emitter.h>
#include <nori/core/bsdfQueryRecord.h>
#include <nori/core/primitiveShape.h>
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

PathTracingIntegrator::PathTracingIntegrator(const PropertyList & propList)
{
    m_depth = propList.getInteger(XML_INTEGRATOR_PATH_MATS_DEPTH, DEFAULT_PATH_TRACING_DEPTH);

}

Color3f PathTracingIntegrator::Li(const Scene * pScene, Sampler * pSampler, const Ray3f & ray) const
{
    Intersection its;
    Ray3f tracingRay(ray);
    Color3f li(0.0f);
    Color3f beta(1.0f);
    uint32_t depth = 0;
    const Emitter * pEnvironmentEmitter = pScene->getEnvironmentEmitter();
    Color3f background = pScene->getBackground();
    bool bForceBackground = pScene->getForceBackground();

    while (depth < m_depth)
    {
        // miss hit, calculate the environment contribute
        if (!pScene->rayIntersect(tracingRay, its))
        {
            if (pEnvironmentEmitter != nullptr && !bForceBackground)
            {
                EmitterQueryRecord emitterQueryRecord;
                emitterQueryRecord.ref = tracingRay.o;
                emitterQueryRecord.wi = tracingRay.d;
                li += beta * pEnvironmentEmitter->eval(emitterQueryRecord) / 1.0f;
            }
            else if (depth == 0)
            {
                return background;
            }
            break;
        }
        /**     Close hit  process */
        if (its.pShape->isEmitter())
        {
            EmitterQueryRecord emitterQueryRecord;
            emitterQueryRecord.ref = tracingRay.o;
            emitterQueryRecord.p = its.p;
            emitterQueryRecord.n = its.shFrame.n;
            emitterQueryRecord.wi = tracingRay.d;
            Color3f Le = its.pEmitter->eval(emitterQueryRecord);
            li += Le * beta;
        }

        const BSDF * pBSDF = its.pBSDF;
        BSDFQueryRecord bsdfQueryRecord(its.toLocal(-1.0f * tracingRay.d), ETransportMode::ERadiance, pSampler, its);
        beta *= pBSDF->sample(bsdfQueryRecord, pSampler->next2D());

        if (beta.isZero())
        {
            break;
        }

        // Russian roulette
        if (pSampler->next1D() < 0.95f)
        {
            constexpr float inv = 1.0f / 0.95f;
            beta *= inv;
        }
        else
        {
            break;
        }

        tracingRay = Ray3f(its.p, its.toWorld(bsdfQueryRecord.wo));
        depth++;
    }

    return li;
}

std::string PathTracingIntegrator::toString() const
{
    return tfm::format("PathMATSIntegrator[depth = %u]", m_depth);
}

NORI_REGISTER_CLASS(PathTracingIntegrator, XML_INTEGRATOR_PATH_MATS);
NORI_NAMESPACE_END
