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

Scene::Scene(const PropertyList &propList)
{
    /* Background of the image, i.e. the return value when the ray does not hit any object */
    m_background = propList.getColor(XML_SCENE_BACKGROUND, DEFAULT_SCENE_BACKGROUND);

    /* Forcely use the background color when the environment emitter is specified */
    m_bForceBackground = propList.getBoolean(XML_SCENE_FORCE_BACKGROUND, DEFAULT_SCENE_FORCE_BACKGROUND);
}

Scene::~Scene() {
    delete m_pAccel;
    delete m_pSampler;
    delete m_pCamera;
    delete m_pIntegrator;
    for(auto& pMesh: m_pMeshes)
    {
        delete pMesh;
    }

    m_pMeshes.clear();
    m_pMeshes.shrink_to_fit();

    for(auto& pEmitter: m_pEmitters)
    {
        delete pEmitter;
    }
    m_pEmitters.clear();
    m_pEmitters.shrink_to_fit();
}

const Accel* Scene::getAccel() const
{
    return m_pAccel;
}

const Integrator* Scene::getIntegrator() const
{
    return m_pIntegrator;
}

Integrator* Scene::getIntegrator()
{
    return m_pIntegrator;
}

const Camera* Scene::getCamera() const
{
    return m_pCamera;
}

const Sampler* Scene::getSampler() const
{
    return m_pSampler;
}

Sampler* Scene::getSampler()
{
    return m_pSampler;
}

const std::vector<Mesh *> & Scene::getMeshes() const
{
    return m_pMeshes;
}

const std::vector<Emitter*> & Scene::getEmitters() const
{
    return m_pEmitters;
}

const Emitter * Scene::getEnvironmentEmitter() const
{
    return m_pEnvironmentEmitter;
}

Emitter * Scene::getEnvironmentEmitter()
{
    return m_pEnvironmentEmitter;
}

const BoundingBox3f& Scene::getBoundingBox() const
{
    return m_bBox;
}

Color3f Scene::getBackground() const
{
    return m_background;
}

bool Scene::getForceBackground() const
{
    return m_bForceBackground;
}

bool Scene::rayIntersect(const Ray3f &ray, Intersection &its) const
{
    return m_pAccel->rayIntersect(ray, its, false);
}

bool Scene::rayIntersect(const Ray3f &ray) const
{
    Intersection its; /* Unused */
    return m_pAccel->rayIntersect(ray, its, true);
}

void Scene::activate() {
    if (m_pAccel == nullptr)
    {
        /* Create a default acceleration */
        LOG(WARNING) << "No acceleration was specified, create a default acceleration.";
        m_pAccel = (Accel*)(NoriObjectFactory::createInstance(DEFAULT_SCENE_ACCELERATION, PropertyList()));
    }

    for (auto& pMesh : m_pMeshes)
    {
        m_pAccel->addMesh(pMesh);
    }

    m_pAccel->build();
    m_bBox = m_pAccel->getBoundingBox();
    LOG(INFO) << "Memory used for Shape : " << memString(m_pAccel->getUsedMemoryForPrimitive());

    if (!m_pIntegrator)
        throw NoriException("No integrator was specified!");

    if (!m_pCamera)
        throw NoriException("No camera was specified!");
    
    if (!m_pSampler) {
        /* Create a default (independent) sampler */
        LOG(WARNING) << "No sampler was specified, create a default sampler.";
        m_pSampler = static_cast<Sampler*>(
            NoriObjectFactory::createInstance("independent", PropertyList()));
    }

    LOG(INFO) << endl;
    LOG(INFO) << "Configuration: " << toString() << endl;
    LOG(INFO) << endl;
}

void Scene::addChild(NoriObject *obj, const std::string & name) {
    switch (obj->getClassType()) {
        case EAcceleration:
            if (m_pAccel != nullptr)
            {
                throw NoriException("There can only be one acceleration per scene!");
            }
            m_pAccel =  static_cast<Accel *>(obj);
            break;
        case EMesh: {
                Mesh *mesh = static_cast<Mesh *>(obj);
            m_pMeshes.push_back(mesh);
            if (((Mesh*)(obj))->isEmitter())
            {
                m_pEmitters.push_back(((Mesh*)(obj))->getEmitter());
            }
            }
            break;
        
        case EEmitter: {
               /// Point light and directional light are added to the list of emitter
            if (((Emitter*)(obj))->getEmitterType() == EEmitterType::EPoint ||
                ((Emitter*)(obj))->getEmitterType() == EEmitterType::EDirectional)
            {
                m_pEmitters.push_back((Emitter*)(obj));
            }
            else if (((Emitter*)(obj))->getEmitterType() == EEmitterType::EEnvironment)
            {
                if (m_pEnvironmentEmitter == nullptr)
                {
                    m_pEnvironmentEmitter = (Emitter*)(obj);
                    m_pEmitters.push_back((Emitter*)(obj));
                }
                else
                {
                    throw NoriException("Scene::addChild(): Only one environment emiiter is allowed for the entire scene");
                }
            }
            else
            {
                /* TBD */
                throw NoriException("Scene::addChild(): You need to implement this for emitters");
            }
            }
            break;

        case ESampler:
            if (m_pSampler)
                throw NoriException("There can only be one sampler per scene!");
            m_pSampler = static_cast<Sampler *>(obj);
            break;

        case ECamera:
            if (m_pCamera)
                throw NoriException("There can only be one camera per scene!");
            m_pCamera = static_cast<Camera *>(obj);
            break;
        
        case EIntegrator:
            if (m_pIntegrator)
                throw NoriException("There can only be one integrator per scene!");
            m_pIntegrator = static_cast<Integrator *>(obj);
            break;

        default:
            throw NoriException("Scene::AddChild(<%s>, <%s>) is not supported!",
                classTypeName(obj->getClassType()), name);
    }
}

std::string Scene::toString() const
{
    std::string meshesStr;
    for (size_t i=0; i < m_pMeshes.size(); ++i) {
        meshesStr += std::string("  ") + indent(m_pMeshes[i]->toString(), 2);
        if (i + 1 < m_pMeshes.size())
            meshesStr += ",";
        meshesStr += "\n";
    }
    std::string emitterStr;
    for (size_t i = 0; i < m_pEmitters.size(); ++i)
    {
        emitterStr += std::string("  ") + indent(m_pEmitters[i]->toString(), 2);
        if (i + 1 < m_pEmitters.size())
        {
            emitterStr += ",";
        }
        emitterStr += "\n";
    }

    return tfm::format(
        "Scene[\n"
        "  background = %s,\n"
        "  forceBackground = %s,\n"
        "  integrator = %s,\n"
        "  sampler = %s\n"
        "  camera = %s,\n"
        "  acceleration = %s,\n"
        "  meshes = {\n"
        "  %s  }\n"
        "  emitters = {\n"
        "  %s  },\n"
        "]",
        m_background.toString(),
        m_bForceBackground ? "true" : "false",
        indent(m_pIntegrator->toString()),
        indent(m_pSampler->toString()),
        indent(m_pCamera->toString()),
        indent(m_pAccel->toString()),
        indent(meshesStr, 2),
        indent(emitterStr, 2)
    );
}

NoriObject::EClassType Scene::getClassType() const
{
    return EScene;
}

NORI_REGISTER_CLASS(Scene, "scene");
NORI_NAMESPACE_END
