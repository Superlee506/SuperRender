//
// Created by superqqli on 2021/12/10.
//
#pragma once
#include <nori/core/shape.h>

NORI_NAMESPACE_BEGIN

/**
* \brief Triangle shape used ONLY in mesh
*/
class Triangle : public Shape
{
public:
    Triangle();

    Triangle(Mesh * pMesh, uint32_t * pFacet, uint32_t iFacet);

    /**
    * \brief Uniformly sample a position on the mesh with
    * respect to surface area. Returns both position and normal
    * Note : Normally, we use Mesh::SamplePosition() instead of
    * this function.
    */
    virtual void samplePosition(const Point2f & Sample, Point3f & P, Normal3f & N) const override;

    /// Return the surface area of the given triangle
    virtual float surfaceArea() const override;

    /// Return an axis-aligned bounding box of the entire mesh
    virtual BoundingBox3f getBoundingBox() const override;

    /// Return the centroid of the given triangle
    virtual Point3f getCentroid() const override;

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
    virtual bool rayIntersect(const Ray3f & Ray, float & U, float & V, float & T) const override;

    /// After intersection test passed, compute the detail information of the intersection point.
    virtual void postIntersect(Intersection & Isect) const override;

    /**
    * \brief Return the pointer of the mesh that this shape attach
    * \return
    *   \c nullptr if the shape does not attach to any mesh
    */
    virtual Mesh * getMesh() const override;

    /**
    * \brief Return the index of facet in the mesh that this shape attach
    * \return
    *   \c uint32_t(-1) if the shape does not attach to any mesh
    */
    virtual uint32_t getFacetIndex() const override;

    /// Is this mesh an area emitter?
    virtual bool isEmitter() const override;

    /// Return a pointer to an attached area emitter instance
    virtual Emitter * getEmitter() override;

    /// Return a pointer to an attached area emitter instance (const version)
    virtual const Emitter * getEmitter() const override;

    /// Return a pointer to the BSDF associated with this mesh
    virtual const BSDF * getBSDF() const override;

    virtual std::string toString() const override;

    Mesh * m_pMesh = nullptr;
    uint32_t * m_pFacet = nullptr;
    uint32_t m_iFacet = 0;
};

NORI_NAMESPACE_END
