//
// Created by superqqli on 2021/12/13.
//
#pragma once
#include <nori/core/common.h>
#include <nori/core/object.h>
#include <nori/core/intersection.h>

NORI_NAMESPACE_BEGIN
/**
 * \brief Convenience data structure used to pass multiple
 * parameters to the evaluation and sampling routines in \ref BSDF
 */
struct BSDFQueryRecord {
    /// Incident direction (in the local frame)
    Vector3f wi;

    /// Outgoing direction (in the local frame)
    Vector3f wo;

    /// Relative refractive index in the sampled direction
    float eta;

    /// Measure associated with the sample
    EMeasure measure;

    /// The transport mode when sampling or evaluating a scattering function
    ETransportMode mode;

    /// The sampler currently used
    Sampler * pSampler;

    /// Reference to the underlying surface interaction
    Intersection its;

    /// Create a new record for sampling the BSDF
    BSDFQueryRecord(const Vector3f & wi );

    BSDFQueryRecord(const Vector3f & wi, const Vector3f & wo,  EMeasure measure);

    /// Create a new record for sampling the BSDF
    BSDFQueryRecord(const Vector3f & wi, ETransportMode mode, Sampler * pSampler, const Intersection & its);

    /// Create a new record for querying the BSDF
    BSDFQueryRecord(const Vector3f & wi, const Vector3f & wo,
                    EMeasure measure, ETransportMode mode, Sampler * pSampler, const Intersection & its);
};
NORI_NAMESPACE_END
