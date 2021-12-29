//
// Created by superqqli on 2021/10/26.
// Simple sample on point light
//
#include <nori/core/integrator.h>

NORI_NAMESPACE_BEGIN

class SimpleIntegrator : public Integrator {
public:
    SimpleIntegrator(const PropertyList &props);

    /// Compute the radiance value for a given ray. Just return green here
    Color3f li(const Scene *pScene, Sampler *pSampler, const Ray3f &ray) const override;

    /// Return a human-readable description for debugging purposes
    std::string toString() const override;

protected:
    Point3f m_lightPosition;
    Color3f m_lightEnerge;
};

NORI_NAMESPACE_END

