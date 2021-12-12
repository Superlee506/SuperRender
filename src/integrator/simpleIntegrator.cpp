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
    const auto& frame = its.shFrame;
    const Point3f& x = its.p;
    const Point3f& p =  m_lightPosition;
    auto shadowDir = p - x;
    float maxRayLength = shadowDir.norm();
    Ray3f shadowRay(x, shadowDir.normalized(), 0.001, maxRayLength);
    // avoid self intersection
    shadowRay.applyPositionBias(its.geoFrame.n, MAX_NORMAL_BIAS);
    float visiblity = 1.0f;
    if (scene->rayIntersect(shadowRay)) {
        visiblity = 0.0f;
    }

    auto cosTheta = frame.cosTheta(frame.toLocal(shadowRay.d));//frame的cosTheta函数是切线空间下计算
    auto squareDistance = std::pow((x - p).norm(), 2.0f);
    auto li = (m_lightEnerge / (4 * M_PI * M_PI)) * ((std::max(0.0f, cosTheta)) / squareDistance) * visiblity;
    return li;
}

/// Return a human-readable description for debugging purposes
std::string SimpleIntegrator::toString() const
{
    return tfm::format(
            "NormalIntegrator[\n"
            "  Position = \"%s\"\n"
            "  Energe = \"%s\"\n"
            "]",
            m_lightPosition.toString(),
            m_lightEnerge.toString()
    );
}

NORI_REGISTER_CLASS(SimpleIntegrator, "simple");
NORI_NAMESPACE_END

