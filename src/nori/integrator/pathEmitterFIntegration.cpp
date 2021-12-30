//
// Created by superqqli on 2021/12/13.
//

#include <nori/integrator/pathEmitterIntegration.h>
#include <nori/core/intersection.h>
#include <nori/core/scene.h>
#include <nori/core/sampler.h>
#include <nori/core/emitterQueryRecord.h>
#include <nori/core/emitter.h>
#include <nori/core/bsdfQueryRecord.h>
#include <nori/core/primitiveShape.h>
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

PathEmitterIntegrator::PathEmitterIntegrator(const PropertyList & propList)
{
    m_depth = propList.getInteger(XML_INTEGRATOR_PATH_MATS_DEPTH, DEFAULT_PATH_TRACING_DEPTH);

}

Color3f PathEmitterIntegrator::li(const Scene * pScene, Sampler * pSampler, const Ray3f & ray) const
{
    Intersection its;
    Ray3f tracingRay(ray);
    Color3f li(0.0f);
    Color3f beta(1.0f);
    uint32_t Depth = 0;
    bool bLastPathSpecular = false;
    const Emitter * pLastEmitter = nullptr;
    const Emitter * pEnvironmentEmitter = pScene->getEnvironmentEmitter();
    Color3f Background = pScene->getBackground();
    bool bForceBackground = pScene->getForceBackground();

    while (Depth < m_depth)
    {
        if (!pScene->rayIntersect(tracingRay, its))
        {
            if (pEnvironmentEmitter != nullptr && !bForceBackground)
            {
                EmitterQueryRecord EmitterRecord;
                EmitterRecord.ref = tracingRay.o;
                EmitterRecord.wi = tracingRay.d;
                li += beta * pEnvironmentEmitter->eval(EmitterRecord) / 1.0f;
            }
            else if (Depth == 0)
            {
                return Background;
            }
            break;
        }

        Color3f Le(0.0f);

        // Only the first ray from the camera or the ray from the specular reflection
        // /refraction need to account for the emmiter term. In other cases, it has
        // been computed during the direct light computing part.
        // There also exists a special case such that the ray hit the emissive object
        // continuously.
        if (its.pShape->isEmitter() && (Depth == 0 || bLastPathSpecular || pLastEmitter == its.pEmitter))
        {
            EmitterQueryRecord EmitterRecord;
            EmitterRecord.ref = tracingRay.o;
            EmitterRecord.p = its.p;
            EmitterRecord.n = its.shFrame.n;
            EmitterRecord.wi = tracingRay.d;
            Le = its.pEmitter->eval(EmitterRecord);
            li += beta * Le;
        }

        const BSDF * pBSDF = its.pBSDF;

        if (pBSDF->isDiffuse())
        {
            bLastPathSpecular = false;
            // Direct light sampling
            for (Emitter * pEmitter : pScene->getEmitters())
            {
                if (pEmitter == its.pEmitter)
                {
                    continue;
                }

                EmitterQueryRecord emitterQueryRecord;
                emitterQueryRecord.ref = its.p;

                if (pEmitter->getEmitterType() == EEmitterType::EEnvironment || pEmitter->getEmitterType() == EEmitterType::EDirectional)
                {
                    emitterQueryRecord.distance = pScene->getBoundingBox().getRadius();
                }

                Color3f ldirect = pEmitter->sample(emitterQueryRecord, pSampler->next2D(), pSampler->next1D());
                if (!ldirect.isZero())
                {
                    Ray3f shadowRay = its.generateShadowRay(emitterQueryRecord.p);
                    if (!pScene->rayIntersect(shadowRay))
                    {
                        BSDFQueryRecord bsdfQueryRecord(its.toLocal(-1.0 * tracingRay.d), its.toLocal(emitterQueryRecord.wi), EMeasure::ESolidAngle, ETransportMode::ERadiance, pSampler, its);
                        li += beta * pBSDF->eval(bsdfQueryRecord) * std::abs(Frame::cosTheta(bsdfQueryRecord.wo)) * ldirect;
                    }
                }
            }
        }
        else
        {
            bLastPathSpecular = true;
        }

        BSDFQueryRecord BSDFRecord(its.toLocal(-1.0f * tracingRay.d), ETransportMode::ERadiance, pSampler, its);
        beta *= pBSDF->sample(BSDFRecord, pSampler->next2D());

        if (beta.isZero())
        {
            break;
        }

        // Russian roulette
        if (pSampler->next1D() < 0.95f)
        {
            constexpr float Inv = 1.0f / 0.95f;
            beta *= Inv;
        }
        else
        {
            break;
        }

        pLastEmitter = its.pEmitter;
        tracingRay = Ray3f(its.p, its.toWorld(BSDFRecord.wo));
        Depth++;
    }

    return li;
}

std::string PathEmitterIntegrator::toString() const
{
    return tfm::format("PathEmitterIntegrator[depth = %u]", m_depth);
}

NORI_REGISTER_CLASS(PathEmitterIntegrator, XML_INTEGRATOR_PATH_EMS);
NORI_NAMESPACE_END
