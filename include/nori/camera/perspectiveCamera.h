/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once
#include <nori/core/common.h>
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
    virtual void activate() override;

    virtual Color3f sampleRay(Ray3f &ray,
                      const Point2f &samplePosition,
                      const Point2f &apertureSample) const override;

    virtual void addChild(NoriObject *obj, const std::string & name) override;

    /// Return a human-readable summary
    virtual std::string toString() const override;
private:
    Vector2f m_invOutputSize;
    Transform m_sampleToCamera;
    Transform m_cameraToWorld;
    float m_fov;
    float m_nearClip;
    float m_farClip;
};

NORI_NAMESPACE_END
