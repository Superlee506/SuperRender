//
// Created by superqqli on 2021/12/13.
//

#include <nori/integrator/whittedIntergration.h>
#include <nori/core/scene.h>
#include <nori/core/intersection.h>
#include <nori/core/bsdf.h>
#include <nori/core/sampler.h>
#include <nori/core/emitterQueryRecord.h>
#include <nori/core/emitter.h>
#include <nori/core/primitiveShape.h>
#include <nori/core/bsdfQueryRecord.h>

NORI_NAMESPACE_BEGIN

WhittedIntegrator::WhittedIntegrator(const PropertyList & propList)
{
    m_depth = uint32_t(propList.getInteger(XML_INTEGRATOR_WHITTED_DEPTH, DEFAULT_INTEGRATOR_WHITTED_DEPTH));
}

Color3f WhittedIntegrator::li(const Scene * pScene, Sampler * pSampler, const Ray3f & ray) const
{
    return liRecursive(pScene, pSampler, ray, 0);
}

std::string WhittedIntegrator::toString() const
{
    return tfm::format("WhittedIntegrator[depth = %d]", m_depth);
}

Color3f WhittedIntegrator::liRecursive(const Scene * pScene, Sampler * pSampler, const Ray3f & ray, uint32_t depth) const
{
    const Emitter * pEnvironmentEmitter = pScene->getEnvironmentEmitter();
    Color3f background = pScene->getBackground();
    bool bForceBackground = pScene->getForceBackground();

    /* Find the surface that is visible in the requested direction */
    Intersection its;
    if (!pScene->rayIntersect(ray, its))
    {
        // Environment contribution
        if (pEnvironmentEmitter != nullptr && !bForceBackground)
        {
            EmitterQueryRecord emitterQueryRecord;
            emitterQueryRecord.ref = ray.o;
            emitterQueryRecord.wi = ray.d;
            return pEnvironmentEmitter->eval(emitterQueryRecord);
        }
        else
        {
            if (depth == 0)
            {
                return background;
            }
            else
            {
                return Color3f(0.0f);
            }
        }
    }
    // Emission contribution
    Color3f le(0.0f);
    if (its.pShape->isEmitter())
    {
        EmitterQueryRecord emitterQueryRecord;
        emitterQueryRecord.ref = ray.o;
        emitterQueryRecord.p = its.p;
        emitterQueryRecord.n = its.shFrame.n;
        emitterQueryRecord.wi = ray.d;
        le = its.pEmitter->eval(emitterQueryRecord);
    }

    Color3f lr(0.0f);
    const BSDF * pBSDF = its.pBSDF;
    // Diffuse material
    if (pBSDF->isDiffuse())
    {
        for (Emitter * pEmitter : pScene->getEmitters())
        {
            if (pEmitter == its.pEmitter)
            {
                BSDFQueryRecord bsdfQueryRecord(its.toLocal(-1.0f * ray.d),
                                                its.toLocal(its.shFrame.n),
                                                EMeasure::ESolidAngle
                                                , ETransportMode::ERadiance,
                                                pSampler,
                                                its);
                float pdf = pBSDF->pdf(bsdfQueryRecord);
                if (pdf > Epsilon)
                {
                    lr += pBSDF->eval(bsdfQueryRecord) / pdf * std::abs(Frame::cosTheta(bsdfQueryRecord.wo)) * le;
                }
                continue;
            }
            else
            {
                EmitterQueryRecord emitterQueryRecord;
                emitterQueryRecord.ref = its.p;

                if (pEmitter->getEmitterType() == EEmitterType::EEnvironment ||
                pEmitter->getEmitterType() == EEmitterType::EDirectional)
                {
                    emitterQueryRecord.distance = pScene->getBoundingBox().getRadius();
                }

                Color3f li = pEmitter->sample(emitterQueryRecord, pSampler->next2D(), pSampler->next1D());
                Ray3f shadowRay = its.generateShadowRay(emitterQueryRecord.p);

                if (!pScene->rayIntersect(shadowRay))
                {
                    BSDFQueryRecord bsdfQueryRecord(its.toLocal(-1.0 * ray.d), its.toLocal(emitterQueryRecord.wi), EMeasure::ESolidAngle, ETransportMode::ERadiance, pSampler, its);
                    lr += pBSDF->eval(bsdfQueryRecord) * std::abs(Frame::cosTheta(bsdfQueryRecord.wo)) * li;
                }
            }
        }
    }
  // Dielectric material
    else
    {
        if (pSampler->next1D() < 0.95f && depth < m_depth)
        {
            constexpr float inv = 1.0f / 0.95f;
            BSDFQueryRecord bsdfQueryRecord(its.toLocal(-1.0f * ray.d), ETransportMode::ERadiance, pSampler, its);
            Color3f color = pBSDF->sample(bsdfQueryRecord, pSampler->next2D());
            lr += color * liRecursive(pScene, pSampler, Ray3f(its.p, its.toWorld(bsdfQueryRecord.wo)), depth + 1) * inv;
        }
    }

    return lr + le;
}

NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END