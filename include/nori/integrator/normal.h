//
// Created by superqqli on 2021/10/26.
//
#include <nori/core/common.h>
#include <nori/core/integrator.h>

NORI_NAMESPACE_BEGIN

class NormalIntegrator : public Integrator {
public:
    NormalIntegrator(const PropertyList &props);

    /// Compute the radiance value for a given ray. Just return green here
    Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const override;

    /// Return a human-readable description for debugging purposes
    std::string toString() const override;

protected:
    std::string m_myProperty;
};

NORI_NAMESPACE_END

