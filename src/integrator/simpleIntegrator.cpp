//
// Created by superqqli on 2021/10/26.
// Simple sample on point light
//
#include <nori/integrator/simpleIntegrator.h>
#include <nori/core/intersection.h>

NORI_NAMESPACE_BEGIN

SimpleIntegrator::SimpleIntegrator(const PropertyList &props)
{
    m_lightPosition = props.getPoint("position", Point3f(0.0));
    m_lightEnerge = props.getColor("energy", Color3f(1.0));
}

/// Compute the radiance value for a given ray. Just return green here
Color3f SimpleIntegrator::Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const
{
    /* Find the surface that is visible in the requested direction */
    Intersection its;
    if (!scene->rayIntersect(ray, its))
        return Color3f(0.0f);
    Ray3f shadowRay = its.generateShadowRay(m_lightPosition);
    // avoid self intersection
    shadowRay.applyPositionBias(its.geoFrame.n, MAX_NORMAL_BIAS);
    if (scene->rayIntersect(shadowRay))
    {
        return Color3f(0.0f);
    }

    auto cosTheta = its.shFrame.cosTheta(its.shFrame.toLocal(shadowRay.d.normalized()));
    return  m_lightEnerge / ((4 * M_PI * M_PI) * shadowRay.d.squaredNorm()) * std::max(0.0f, cosTheta);
}

/// Return a human-readable description for debugging purposes
std::string SimpleIntegrator::toString() const
{
    return tfm::format(
            "SimpleIntegrator[\n"
            "  Position = \"%s\"\n"
            "  Energe = \"%s\"\n"
            "]",
            m_lightPosition.toString(),
            m_lightEnerge.toString()
    );
}

NORI_REGISTER_CLASS(SimpleIntegrator, "simple");
NORI_NAMESPACE_END

