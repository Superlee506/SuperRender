//
// Created by superqqli on 2021/12/13.
//
#pragma once
#include <nori/core/common.h>
#include <nori/core/object.h>

NORI_NAMESPACE_BEGIN
/**
* \brief Convenience data structure used to pass multiple
* parameters to the evaluation and sampling routines in \ref Emitter
*/
struct EmitterQueryRecord
{
    /// Pointer to the sampled emitter
    const Emitter * pEmitter = nullptr;

    /// Origin point from which we sample the emitter
    Point3f ref;

    /// Sampled position on the light source
    Point3f p;

    /// Associated surface normal
    Normal3f n;

    /// Solid angle density with respect to Ref
    float pdf;

    /// Direction vector from 'Ref' to 'P'
    Vector3f wi;

    /// Distance between 'Ref' and 'P'.
    /// When the type of emitter is either Env or Directional, this
    /// value should be set as the radius of the bounding sphere of
    /// the whole scene before CALLING Sample().
    float distance;

    /// Create an unitialized query record
    EmitterQueryRecord();

    /// Create a new query record that can be used to sample a emitter
    EmitterQueryRecord(const Point3f & ref);

    /**
    * \brief Create a query record that can be used to query the
    * sampling density after having intersected an area emitter
    */
    EmitterQueryRecord(const Emitter * pEmitter, const Point3f & ref, const Point3f & p, const Normal3f & n);

    /// Return a human-readable string summary
    std::string toString() const;
};

NORI_NAMESPACE_END