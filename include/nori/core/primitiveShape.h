//
// Created by superqqli on 2021/12/10.
//

#pragma once

#include <nori/core/common.h>
#include <nori/core/object.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Base class for diffrent primitives, e.g. triangle
 * Shape is the basic primitive of a mesh
 */
class PrimitiveShape : public NoriObject
{
public:
    /**
    * \brief Uniformly sample a position on the shape with
    * respect to surface area. Returns both position and normal
    */
    virtual void samplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const = 0;

    /// Return the surface area of the given shape
    virtual float surfaceArea() const = 0;

    /// Return an axis-aligned bounding box of the entire shape
    virtual BoundingBox3f getBoundingBox() const = 0;

    /// Return the centroid of the given shape
    virtual Point3f getCentroid() const = 0;

    /** \brief intersection test
    *
    * \param Ray
    *    The ray segment to be used for the intersection query
    * \param U
    *   Upon success, \c U will contain the 'U' component of the intersection
    *   in barycentric coordinates
    * \param V
    *   Upon success, \c V will contain the 'V' component of the intersection
    *   in barycentric coordinates
    * \param T
    *    Upon success, \a T contains the distance from the ray origin to the
    *    intersection point,
    * \return
    *   \c true if an intersection has been detected
    */
    virtual bool rayIntersect(const Ray3f & Ray, float & U, float & V, float & T) const = 0;

    /// After intersection test passed, compute the detail information of the intersection point.
    virtual void postIntersect(Intersection & Isect) const = 0;

    /**
    * \brief Return the pointer of the mesh that this shape attach
    * \return
    *   \c nullptr if the shape does not attach to any mesh
    */
    virtual Mesh * getMesh() const;

    /**
    * \brief Return the index of facet in the mesh that this shape attach
    * \return
    *   \c uint32_t(-1) if the shape does not attach to any mesh
    */
    virtual uint32_t getFacetIndex() const;

    /// Is this shape an area emitter?
    virtual bool isEmitter() const;

    /// Return a pointer to an attached area emitter instance
    virtual Emitter * getEmitter();

    /// Return a pointer to an attached area emitter instance (const version)
    virtual const Emitter * getEmitter() const;

    /// Return a pointer to the BSDF associated with this shape
    virtual const BSDF * getBSDF() const = 0;

    /// Compute mean curvature and Gaussian curvature at the specified intersection point
    void computeCurvature(const Intersection & Isect, float & H, float & K) const;

    /**
    * \brief Return the type of object (i.e. Mesh/BSDF/etc.)
    * provided by this instance
    * */
    virtual EClassType getClassType() const;
};

NORI_NAMESPACE_END