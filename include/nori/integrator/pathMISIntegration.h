//
// Created by superqqli on 2021/12/13.
//

#include <nori/core/common.h>
#include <nori/core/integrator.h>

NORI_NAMESPACE_BEGIN

/**
*\brief This integrator simulates a simple path tracing integrator.
*/
class PathMISIntegrator : public Integrator
{
public:
    PathMISIntegrator(const PropertyList & propList);

    /// Compute the radiance value for a given ray. Just return green here
    virtual Color3f li(const Scene * pScene, Sampler * pSampler, const Ray3f & ray) const override;

    /// Return a human-readable description for debugging purposes
    virtual std::string toString() const override;

protected:
    uint32_t m_depth;
};

NORI_NAMESPACE_END
