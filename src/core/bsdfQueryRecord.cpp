//
// Created by superqqli on 2021/12/13.
//

#include <nori/core/bsdfQueryRecord.h>


NORI_NAMESPACE_BEGIN

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & wi) : wi(wi)
{

}

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & wi, const Vector3f & wo,
                EMeasure measure) :wi(wi), wo(wo)
{

}

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & wi, ETransportMode mode,
                                 Sampler * pSampler, const Intersection & its)
        : wi(wi), mode(mode), pSampler(pSampler), its(its),
        eta(1.f), measure(EUnknownMeasure) { }

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & wi, const Vector3f & wo,
                                 EMeasure measure, ETransportMode mode,
                                 Sampler * pSampler, const Intersection & its)
        : wi(wi), wo(wo), measure(measure), mode(mode),pSampler(pSampler),
          its(its),eta(1.f){ }

NORI_NAMESPACE_END
