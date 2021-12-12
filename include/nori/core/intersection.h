//
// Created by superqqli on 2021/12/12.
//
#include <nori/core/common.h>
#include <nori/core/vector.h>
#include <nori/core/frame.h>

NORI_NAMESPACE_BEGIN
/**
 * \brief Intersection data structure
 *
 * This data structure records local information about a ray-triangle intersection.
 * This includes the position, traveled ray distance, uv coordinates, as well
 * as well as two local coordinate frames (one that corresponds to the true
 * geometry, and one that is used for shading computations).
 */
struct Intersection {
    /// Position of the surface intersection
    Point3f p;
    /// Unoccluded distance along the ray
    float t;
    /** \brief UV coordinates, if any
     * (This parameter was used as barycentric coordinate in the 1st stage
     * of ray-triangle intersection and was transformed to UV coordinate in
     * the 2nd stage)
     */
    Point2f uv;

    bool bHasUVPartial = false;

    Vector3f dPdU;

    Vector3f dPdV;

    Vector3f dNdU;

    Vector3f dNdV;

    float dUdX = 0.0f;

    float dUdY = 0.0f;

    float dVdX = 0.0f;

    float dVdY = 0.0f;

    /// Shading frame (based on the shading normal)
    Frame shFrame;
    /// Geometric frame (based on the true geometry)
    Frame geoFrame;
    /// Pointer to the associated mesh
    const Mesh *mesh;
    /// Pointer to the associated mesh
    const PrimitiveShape * pShape = nullptr;

    /// Pointer to the associated Emitter (nullptr if not a emitter)
    const Emitter * pEmitter = nullptr;

    /// Pointer to the associated BSDF
    const BSDF * pBSDF = nullptr;

    /// Create an uninitialized intersection record
    Intersection();

    /// Transform a direction vector into the local shading frame
    Vector3f toLocal(const Vector3f &d) const;

    /// Transform a direction vector from local to world coordinates
    Vector3f toWorld(const Vector3f &d) const;

    /// Return a human-readable summary of the intersection record
    std::string toString() const;
};
NORI_NAMESPACE_END
