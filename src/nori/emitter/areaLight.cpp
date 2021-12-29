//
// Created by superqqli on 2021/12/13.
//
#include <nori/emitter/areaLight.h>
#include <nori/core/mesh.h>
#include <nori/core/emitterQueryRecord.h>

NORI_NAMESPACE_BEGIN

AreaLight::AreaLight(const PropertyList & propList)
{
    m_radiance = propList.getColor(XML_EMITTER_AREA_LIGHT_RADIANCE);
    m_Type = EEmitterType::EArea;
}

Color3f AreaLight::sample(EmitterQueryRecord & record, const Point2f & sample2D, float sample1D) const
{
    if (m_pMesh == nullptr)
    {
        throw NoriException("There is no shape attached to this AreaLight!");
    }

    m_pMesh->samplePosition(sample1D, sample2D, record.p, record.n);

    Vector3f wi = record.p - record.ref;

    record.distance = wi.norm();
    record.wi = wi.normalized();
    record.pEmitter = this;
    record.pdf = pdf(record);

    if (record.pdf == 0.0f || std::isinf(record.pdf))
    {
        return Color3f(0.0f);
    }

    return eval(record) / record.pdf;
}

float AreaLight::pdf(const EmitterQueryRecord & record) const
{
    if (m_pMesh == nullptr)
    {
        throw NoriException("There is no shape attached to this AreaLight!");
    }

    /* Transform the integration variable from the position domain to solid angle domain */
    float gDenominator = std::abs((-1.0f * record.wi).dot(record.n));

    if (gDenominator == 0.0f)
    {
        return 0.0f;
    }

    float gNumerator = record.distance * record.distance;

    return m_pMesh->pdf() *
           (gNumerator / gDenominator);
}

Color3f AreaLight::eval(const EmitterQueryRecord & record) const
{
    if (m_pMesh == nullptr)
    {
        throw NoriException("There is no shape attached to this AreaLight!");
    }

    // Check if the associated normal in emitter query record
    // and incoming direction are not backfacing
    if (record.n.dot(record.wi) < 0.0f)
    {
        return m_radiance;
    }

    return Color3f(0.0f);
}

void AreaLight::setParent(NoriObject * pParentObj, const std::string &name)
{
    EClassType clzType = pParentObj->getClassType();
    if (clzType == EClassType::EMesh)
    {
        //setMesh((Mesh*)(pParentObj));
        m_pMesh = (Mesh*)(pParentObj);
    }
    else
    {
        throw NoriException("AreaLight::SetParent(<%s>, <%s>) is not supported!",
                              classTypeName(pParentObj->getClassType()), name
        );
    }
}

std::string AreaLight::toString() const
{
    return tfm::format(
            "AreaLight[radiance = %s]",
            m_radiance.toString()
    );
}

NORI_REGISTER_CLASS(AreaLight, XML_EMITTER_AREA_LIGHT);
NORI_NAMESPACE_END

