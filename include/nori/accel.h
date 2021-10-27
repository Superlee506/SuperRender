/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN
constexpr uint32_t MAX_PRIMITIVE_NUM = 16;//the max num of primitives in the leaf node
constexpr uint32_t MAX_TREE_DEPTH = 8;//the max depth of tree
constexpr uint32_t MAX_CHILD_NUM = 8; // the num of child nodes.
struct OctNode {
    size_t child = 0;//The start index of child node, other index are child + 1, child + 2...
    BoundingBox3f bbox;// Bounding box of this node
    std::vector<uint32_t> indice;//the indices of triangles in this node
};
/**
 * \brief Acceleration data structure for ray intersection queries
 *
 * The current implementation falls back to a brute force loop
 * through the geometry.
 */
class Accel {
public:
    /**
     * \brief Register a triangle mesh for inclusion in the acceleration
     * data structure
     *
     * This function can only be used before \ref build() is called
     */
    void addMesh(Mesh *mesh);

    /// Build the acceleration data structure (currently a no-op)
    void build();

    /// Return an axis-aligned box that bounds the scene
    const BoundingBox3f &getBoundingBox() const { return m_bbox; }

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
    bool rayIntersect(const Ray3f &ray, Intersection &its, bool shadowRay) const;

private:
    /***
     * Add primitive(e.g. Triangle) to Acceleration Structure
     * @param idx: the index of primitive
     */
    void Accel::addPrimitive(uint32_t idx);
    bool traverse(size_t nodeIndex, bool shadow, Ray3f& ray, Intersection& its, uint32_t& hitIdx) const;
private:
    Mesh         *m_mesh = nullptr; ///< Mesh (only a single one for now)
    BoundingBox3f m_bbox;           ///< Bounding box of the entire scene
    std::vector<OctNode> m_tree;
    size_t m_maxDepth = 0;//max depths of the Oct Tree
    size_t m_leafNum = 0;//number of leaf nodes
    size_t m_nodeNum = 0;//total number of nodes
};

NORI_NAMESPACE_END
