/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/core/camera.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Perspective camera with depth of field
 *
 * This class implements a simple perspective camera model. It uses an
 * infinitesimally small aperture, creating an infinite depth of field.
 */
class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(const PropertyList &propList);
    void activate();

    Color3f sampleRay(Ray3f &ray,
                      const Point2f &samplePosition,
                      const Point2f &apertureSample) const;

    void addChild(NoriObject *obj);

    /// Return a human-readable summary
    std::string toString() const;
private:
    Vector2f m_invOutputSize;
    Transform m_sampleToCamera;
    Transform m_cameraToWorld;
    float m_fov;
    float m_nearClip;
    float m_farClip;
};

NORI_NAMESPACE_END
