//
// Created by superqqli on 2021/12/13.
//

#include <nori/integrator/pathMISIntegration.h>
#include <nori/core/intersection.h>
#include <nori/core/scene.h>
#include <nori/core/sampler.h>
#include <nori/core/emitterQueryRecord.h>
#include <nori/core/emitter.h>
#include <nori/core/bsdfQueryRecord.h>
#include <nori/core/primitiveShape.h>
#include <nori/core/bsdf.h>

NORI_NAMESPACE_BEGIN

PathMISIntegrator::PathMISIntegrator(const PropertyList & propList)
{
    m_depth = propList.getInteger(XML_INTEGRATOR_PATH_MATS_DEPTH, DEFAULT_PATH_TRACING_DEPTH);

}

Color3f PathMISIntegrator::li(const Scene * pScene, Sampler * pSampler, const Ray3f & ray) const
{
    Intersection itsNext;
    bool bFoundIntersectionNext = false;

    Intersection its;
    Ray3f tracingRay(ray);
    Color3f li(0.0f);
    Color3f beta(1.0f);
    uint32_t depth = 0;
    float weightEms = 1.0f, weightMats = 1.0f;
    const Emitter * pEnvironmentEmitter = pScene->getEnvironmentEmitter();
    Color3f background = pScene->getBackground();
    bool bForceBackground = pScene->getForceBackground();

    while (depth < m_depth)
    {
        if (depth == 0)
        {
            // miss hit, calculate the environment contribute
            if (!pScene->rayIntersect(tracingRay, its))
            {
                if (pEnvironmentEmitter != nullptr && !bForceBackground)
                {
                    EmitterQueryRecord EmitterRecord;
                    EmitterRecord.ref = tracingRay.o;
                    EmitterRecord.wi = tracingRay.d;
                    li += beta * pEnvironmentEmitter->eval(EmitterRecord) / 1.0f;
                    break;
                }
                else
                {
                    return background;
                }
            }
        }
        else // Start from depth > 0
        {
            if (bFoundIntersectionNext)
            {
                its = itsNext;
            }
            else
            {
                if (pEnvironmentEmitter != nullptr)
                {
                    EmitterQueryRecord emitterRecord;
                    emitterRecord.ref = tracingRay.o;
                    emitterRecord.wi = tracingRay.d;
                    li += beta * pEnvironmentEmitter->eval(emitterRecord) / 1.0f;
                }
                break;
            }
        }

        /* Process the closest hit */
        float pdfLightEms = 0.0f, pdfBsdfEms = 0.0f;
        float pdfLightMats = 0.0f, pdfBsdfmats = 0.0f;

        const BSDF * pBSDF = its.pBSDF;

        if (its.pShape->isEmitter())
        {
            EmitterQueryRecord emitterQueryRecord(its.pEmitter, tracingRay.o, its.p, its.shFrame.n);

            Color3f Le = its.pEmitter->eval(emitterQueryRecord);
            li += beta * weightMats * Le;
        }

        // Sampling direct light
        for (Emitter * pEmitter : pScene->getEmitters())
        {
            EmitterQueryRecord emitterQueryRecord(its.p);

            if (pEmitter->getEmitterType() == EEmitterType::EEnvironment || pEmitter->getEmitterType() == EEmitterType::EDirectional)
            {
                emitterQueryRecord.distance = pScene->getBoundingBox().getRadius();
            }

            Color3f ldirect = pEmitter->sample(emitterQueryRecord, pSampler->next2D(), pSampler->next1D());
            pdfLightEms = emitterQueryRecord.pdf;

            if (!ldirect.isZero())
            {
                Ray3f shadowRay = its.generateShadowRay(emitterQueryRecord.p);
                if (!pScene->rayIntersect(shadowRay))
                {
                    // For some virtual light which are not in BVH, we can set BSDF to EMeasure::EDiscrete, so that the value of pdfBsdfEms will be 0
                    BSDFQueryRecord bsdfQueryRecord(its.toLocal(-1.0f * tracingRay.d), its.toLocal(emitterQueryRecord.wi), EMeasure::ESolidAngle, ETransportMode::ERadiance, pSampler, its);
                    pdfBsdfEms = pBSDF->pdf(bsdfQueryRecord);
                    if (pdfLightEms + pdfBsdfEms != 0.0f)
                    {
                        weightEms = pdfLightEms / (pdfLightEms + pdfBsdfEms);
                    }
                    li += beta * pBSDF->eval(bsdfQueryRecord) * std::abs(Frame::cosTheta(bsdfQueryRecord.wo)) * ldirect * weightEms;
                }
            }
        }

        BSDFQueryRecord bsdfQueryRecord(its.toLocal(-1.0f * tracingRay.d), ETransportMode::ERadiance, pSampler, its);
        Color3f F = pBSDF->sample(bsdfQueryRecord, pSampler->next2D());

        tracingRay = Ray3f(its.p, its.toWorld(bsdfQueryRecord.wo));
        beta *= F;

        if (beta.isZero())
        {
            break;
        }

        // Intersection test for nest bounce
        bFoundIntersectionNext = pScene->rayIntersect(tracingRay, itsNext);
        if (bFoundIntersectionNext && itsNext.pEmitter != nullptr) // update the mis weight of indirect light
        {
            EmitterQueryRecord EmitterRecord(itsNext.pEmitter, its.p, itsNext.p, itsNext.shFrame.n);

            pdfLightMats = itsNext.pEmitter->pdf(EmitterRecord);
            pdfBsdfmats = pBSDF->pdf(bsdfQueryRecord);

            if (pdfBsdfmats + pdfLightMats != 0.0f)
            {
                weightMats = pdfBsdfmats / (pdfBsdfmats + pdfLightMats);
            }
        }

        if (bsdfQueryRecord.measure == EMeasure::EDiscrete)
        {
            weightEms = 0.0f;
            weightMats = 1.0f;
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

        depth++;
    }

    return li;
}

std::string PathMISIntegrator::toString() const
{
    return tfm::format("PathMISIntegrator[depth = %u]", m_depth);
}

NORI_REGISTER_CLASS(PathMISIntegrator, XML_INTEGRATOR_PATH_MIS);
NORI_NAMESPACE_END
