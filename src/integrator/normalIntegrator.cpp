//
// Created by superqqli on 2021/10/26.
//
#include <nori/integrator/normalIntegrator.h>
#include <nori/core/mesh.h>
#include <nori/core/scene.h>

NORI_NAMESPACE_BEGIN

NormalIntegrator::NormalIntegrator(const PropertyList &props)
{

}

/// Compute the radiance value for a given ray. Just return green here
Color3f NormalIntegrator::Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const
{
    /* Find the surface that is visible in the requested direction */
    Intersection its;
    if (!scene->rayIntersect(ray, its))
        return Color3f(0.0f);

    /* Return the component-wise absolute
       value of the shading normal as a color */
    Normal3f n = its.shFrame.n.cwiseAbs();
    return Color3f(n.x(), n.y(), n.z());
}

/// Return a human-readable description for debugging purposes
std::string NormalIntegrator::toString() const {
    return tfm::format(
            "NormalIntegrator[\n"
            "  myProperty = \"%s\"\n"
            "]",
            m_myProperty
    );
}

NORI_REGISTER_CLASS(NormalIntegrator, "normals");
NORI_NAMESPACE_END

