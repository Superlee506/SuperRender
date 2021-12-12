/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/core/vector.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Stores a three-dimensional orthonormal coordinate frame
 *
 * This class is mostly used to quickly convert between different
 * cartesian coordinate systems and to efficiently compute certain
 * quantities (e.g. \ref cosTheta(), \ref tanTheta, ..).
 */
struct Frame {
    Vector3f s, t;
    Normal3f n;

    /// Default constructor -- performs no initialization!
    Frame();

    Frame(const Normal3f & n, const Vector3f & dpdu);

    /// Given a normal and tangent vectors, construct a new coordinate frame
    Frame(const Vector3f &s, const Vector3f &t, const Normal3f &n);

    /// Construct a frame from the given orthonormal vectors
    Frame(const Vector3f &x, const Vector3f &y, const Vector3f &z);

    /// Construct a new coordinate frame from a single vector
    Frame(const Vector3f &n);

    /// Convert from world coordinates to local coordinates
    Vector3f toLocal(const Vector3f &v) const;

    /// Convert from local coordinates to world coordinates
    Vector3f toWorld(const Vector3f &v) const;

    /** \brief Assuming that the given direction is in the local coordinate 
     * system, return the cosine of the angle between the normal and v */
    static float cosTheta(const Vector3f &v);

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the sine of the angle between the normal and v */
    static float sinTheta(const Vector3f &v);

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the tangent of the angle between the normal and v */
    static float tanTheta(const Vector3f &v);

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the squared sine of the angle between the normal and v */
    static float sinTheta2(const Vector3f &v);

    /** \brief Assuming that the given direction is in the local coordinate 
     * system, return the sine of the phi parameter in spherical coordinates */
    static float sinPhi(const Vector3f &v);

    /** \brief Assuming that the given direction is in the local coordinate 
     * system, return the cosine of the phi parameter in spherical coordinates */
    static float cosPhi(const Vector3f &v);

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the squared sine of the phi parameter in  spherical
     * coordinates */
    static float sinPhi2(const Vector3f &v);

    /** \brief Assuming that the given direction is in the local coordinate
     * system, return the squared cosine of the phi parameter in  spherical
     * coordinates */
    static float cosPhi2(const Vector3f &v);
    /// Equality test
    bool operator==(const Frame &frame) const;
    /// Inequality test
    bool operator!=(const Frame &frame) const;

    /// Return a human-readable string summary of this frame
    std::string toString() const;
};

NORI_NAMESPACE_END
