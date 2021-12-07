/*
    This file is part of Nori, a simple educational ray tracer
    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/mesh/mesh.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Acceleration data structure for ray intersection queries
 *
 * The current implementation falls back to a brute force loop
 * through the geometry.
 */
class Accel {
public:
    Accel::Accel();
    virtual Accel::~Accel();
    /**
     * \brief Register a triangle mesh for inclusion in the acceleration
     * data structure
     *
     * This function can only be used before \ref build() is called
     */
    virtual void addMesh(Mesh *mesh);

    /// Build the acceleration data structure (currently a no-op)
    virtual void build();

    /// Return an axis-aligned box that bounds the scene
    virtual const BoundingBox3f &getBoundingBox() const { return m_bbox; }

    /**
     * \brief Intersect a ray against all triangles stored in the scene and
     * return detailed intersection information
     *
     * \param ray
     *    A 3-dimensional ray data structure with minimum/maximum extent
     *    information
     *
     * \param its
     *    A detailed intersection record, which will be filled by the
     *    intersection query
     *
     * \param shadowRay
     *    \c true if this is a shadow ray query, i.e. a query that only aims to
     *    find out whether the ray is blocked or not without returning detailed
     *    intersection information.
     *
     * \return \c true if an intersection was found
     */
    virtual bool rayIntersect(const Ray3f &ray, Intersection &its, bool shadowRay) const;

protected:
    struct BaseInternals {
    /**
     * \brief Compute the mesh and triangle indices corresponding to
     * a primitive index used by the underlying generic BVH implementation.
    */
    static uint32_t findMesh(Accel const &accel, uint32_t &idx);
    /**
     * \brief At this point, we now know that there is an intersection,
     * and we know the triangle index of the closest such intersection.
     * The following computes a number of additional properties which
     * characterize the intersection (normals, texture coordinates, etc..)
    */
    static void getHitAttributes(const uint32_t& hitIndexThisMesh, Intersection& its);
    //// Return an axis-aligned bounding box containing the given triangle
    static BoundingBox3f getBoundingBox(Accel const &accel, uint32_t index);
    //// Return the centroid of the given triangle
    static Point3f getCentroid(Accel const &accel, uint32_t index);

    };
    std::vector<uint32_t> m_meshOffset; ///< Index of the first triangle for each shape
    std::vector<Mesh *> m_meshes; ///< List of meshes
    BoundingBox3f m_bbox;           ///< Bounding box of the entire scene
};

NORI_NAMESPACE_END