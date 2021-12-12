/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/core/scene.h>
#include <nori/core/integrator.h>
#include <nori/core/sampler.h>
#include <nori/core/camera.h>
#include <nori/core/emitter.h>
#include <nori/core/accel.h>
#include <nori/core/intersection.h>
#include <nori/core/mesh.h>

NORI_NAMESPACE_BEGIN

Scene::Scene(const PropertyList &) {

}

Scene::~Scene() {
    delete m_accel;
    delete m_sampler;
    delete m_camera;
    delete m_integrator;
    for(auto& pMesh: m_meshes)
    {
        delete pMesh;
    }
    m_meshes.clear();
}

const Accel* Scene::getAccel() const
{
    return m_accel;
}

const Integrator* Scene::getIntegrator() const
{
    return m_integrator;
}

Integrator* Scene::getIntegrator()
{
    return m_integrator;
}

const Camera* Scene::getCamera() const
{
    return m_camera;
}

const Sampler* Scene::getSampler() const
{
    return m_sampler;
}

Sampler* Scene::getSampler()
{
    return m_sampler;
}

const std::vector<Mesh *> & Scene::getMeshes() const
{
    return m_meshes;
}

bool Scene::rayIntersect(const Ray3f &ray, Intersection &its) const
{
    return m_accel->rayIntersect(ray, its, false);
}

bool Scene::rayIntersect(const Ray3f &ray) const
{
    Intersection its; /* Unused */
    return m_accel->rayIntersect(ray, its, true);
}

const BoundingBox3f& Scene::getBoundingBox() const
{
    return m_accel->getBoundingBox();
}

void Scene::activate() {
    if (m_accel == nullptr)
    {
        /* Create a default acceleration */
        m_accel = (Accel*)(NoriObjectFactory::createInstance(DEFAULT_SCENE_ACCELERATION, PropertyList()));
    }

    for (auto& pMesh : m_meshes)
    {
        m_accel->addMesh(pMesh);
    }

    m_accel->build();

    if (!m_integrator)
        throw NoriException("No integrator was specified!");
    if (!m_camera)
        throw NoriException("No camera was specified!");
    
    if (!m_sampler) {
        /* Create a default (independent) sampler */
        m_sampler = static_cast<Sampler*>(
            NoriObjectFactory::createInstance("independent", PropertyList()));
    }

    cout << endl;
    cout << "Configuration: " << toString() << endl;
    cout << endl;
}

void Scene::addChild(NoriObject *obj) {
    switch (obj->getClassType()) {
        case EAcceleration:
            if (m_accel != nullptr)
            {
                throw NoriException("There can only be one acceleration per scene!");
            }
            m_accel =  static_cast<Accel *>(obj);
            break;
        case EMesh: {
                Mesh *mesh = static_cast<Mesh *>(obj);
                m_meshes.push_back(mesh);
            }
            break;
        
        case EEmitter: {
                //Emitter *emitter = static_cast<Emitter *>(obj);
                /* TBD */
                throw NoriException("Scene::addChild(): You need to implement this for emitters");
            }
            break;

        case ESampler:
            if (m_sampler)
                throw NoriException("There can only be one sampler per scene!");
            m_sampler = static_cast<Sampler *>(obj);
            break;

        case ECamera:
            if (m_camera)
                throw NoriException("There can only be one camera per scene!");
            m_camera = static_cast<Camera *>(obj);
            break;
        
        case EIntegrator:
            if (m_integrator)
                throw NoriException("There can only be one integrator per scene!");
            m_integrator = static_cast<Integrator *>(obj);
            break;

        default:
            throw NoriException("Scene::addChild(<%s>) is not supported!",
                classTypeName(obj->getClassType()));
    }
}

std::string Scene::toString() const {
    std::string meshes;
    for (size_t i=0; i<m_meshes.size(); ++i) {
        meshes += std::string("  ") + indent(m_meshes[i]->toString(), 2);
        if (i + 1 < m_meshes.size())
            meshes += ",";
        meshes += "\n";
    }

    return tfm::format(
        "Scene[\n"
        "  integrator = %s,\n"
        "  sampler = %s\n"
        "  camera = %s,\n"
        "  acceleration = %s,\n"
        "  meshes = {\n"
        "  %s  }\n"
        "]",
        indent(m_integrator->toString()),
        indent(m_sampler->toString()),
        indent(m_camera->toString()),
        indent(m_accel->toString()),
        indent(meshes, 2)
    );
}

NoriObject::EClassType Scene::getClassType() const
{
    return EScene;
}

NORI_REGISTER_CLASS(Scene, "scene");
NORI_NAMESPACE_END
