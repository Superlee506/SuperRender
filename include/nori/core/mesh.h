/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/core/object.h>
#include <nori/core/bbox.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Triangle mesh
 *
 * This class stores a triangle mesh object and provides numerous functions
 * for querying the individual triangles. Subclasses of \c Mesh implement
 * the specifics of how to create its contents (e.g. by loading from an
 * external file)
 */
class Mesh : public NoriObject {
public:
    /// Release all memory
    virtual ~Mesh();

    /// Initialize internal data structures (called once by the XML parser)
    virtual void activate();

    /// Return the total number of triangles in this shape
    uint32_t getTriangleCount() const { return (uint32_t) m_F.cols(); }

    /// Return the total number of vertices in this shape
    uint32_t getVertexCount() const { return (uint32_t) m_V.cols(); }

    /// Return the surface area of the given triangle
    float surfaceArea(uint32_t index) const;

    //// Return an axis-aligned bounding box of the entire mesh
    const BoundingBox3f &getBoundingBox() const { return m_bbox; }

    //// Return an axis-aligned bounding box containing the given triangle
    BoundingBox3f getBoundingBox(uint32_t index) const;

    //// Return the centroid of the given triangle
    Point3f getCentroid(uint32_t index) const;

    /** \brief Ray-triangle intersection test
     *
     * Uses the algorithm by Moeller and Trumbore discussed at
     * <tt>http://www.acm.org/jgt/papers/MollerTrumbore97/code.html</tt>.
     *
     * Note that the test only applies to a single triangle in the mesh.
     * An acceleration data structure like \ref BVH is needed to search
     * for intersections against many triangles.
     *
     * \param index
     *    Index of the triangle that should be intersected
     * \param ray
     *    The ray segment to be used for the intersection query
     * \param t
     *    Upon success, \a t contains the distance from the ray origin to the
     *    intersection point,
     * \param u
     *   Upon success, \c u will contain the 'U' component of the intersection
     *   in barycentric coordinates
     * \param v
     *   Upon success, \c v will contain the 'V' component of the intersection
     *   in barycentric coordinates
     * \return
     *   \c true if an intersection has been detected
     */
    bool rayIntersect(uint32_t index, const Ray3f &ray, float &u, float &v, float &t) const;

    /// Return a pointer to the vertex positions
    const MatrixXf &getVertexPositions() const;

    /// Return a pointer to the vertex normals (or \c nullptr if there are none)
    const MatrixXf &getVertexNormals() const;

    /// Return a pointer to the texture coordinates (or \c nullptr if there are none)
    const MatrixXf &getVertexTexCoords() const;

    /// Return a pointer to the triangle vertex index list
    const MatrixXu &getIndices() const;

    /// Is this mesh an area emitter?
    bool isEmitter() const;

    /// Return a pointer to an attached area emitter instance
    Emitter *getEmitter();

    /// Return a pointer to an attached area emitter instance (const version)
    const Emitter *getEmitter() const;

    /// Return a pointer to the BSDF associated with this mesh
    const BSDF *getBSDF() const;

    /// Register a pChildObj object (e.g. a BSDF) with the mesh
    virtual void addChild(NoriObject *pChildObj, const std::string & name);

    /// Return the name of this mesh
    const std::string &getName() const;

    /// Return a human-readable summary of this instance
    virtual std::string toString() const override;

    /**
     * \brief Return the type of object (i.e. Mesh/BSDF/etc.)
     * provided by this instance
     * */
    virtual EClassType getClassType() const override;

    /**
	* \brief Uniformly sample a position on the mesh with
	* respect to surface area. Returns both position and normal
     * *
	* \param Sample1D  Another optional sample that might be used in some scenarios.
     * \param Sample2D  A uniformly distributed sample on \f$[0,1]^2\f$
	*/
    void samplePosition(float sample1D, const Point2f & sample2D, Point3f & samplePoint, Normal3f & sampleNormal) const;

    ///  Compute the probability of sampling point on the mesh
    float pdf() const;

protected:
    /// Create an empty mesh
    Mesh();

protected:
    std::string m_name;                  ///< Identifying name
    MatrixXf      m_V;                   ///< Vertex positions
    MatrixXf      m_N;                   ///< Vertex normals
    MatrixXf      m_UV;                  ///< Vertex texture coordinates
    MatrixXu      m_F;                   ///< Faces
    BSDF         *m_bsdf = nullptr;      ///< BSDF of the surface
    Emitter    *m_emitter = nullptr;     ///< Associated emitter, if any
    BoundingBox3f m_bbox;                ///< Bounding box of the mesh
    std::unique_ptr<DiscretePDF1D> m_pPDF; /// < Used for sampling triangle of the mesh weighted by its area
    float m_meshArea = 0.0f;               ///< Total surface area of the mesh
    float m_invMeshArea = 0.0f;            ///< Probability of a sampling point on the mesh
};

NORI_NAMESPACE_END
