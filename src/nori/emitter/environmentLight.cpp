//
// Created by superqqli on 2021/12/13.
//
#include <nori/emitter/environmentLigh.h>
#include <nori/core/mesh.h>
#include <nori/core/emitterQueryRecord.h>
#include <nori/core/bitmap.h>
#include <nori/core/discretePDF.h>

NORI_NAMESPACE_BEGIN

EnvironmentLight::EnvironmentLight(const PropertyList & propList)
{
    m_scale = propList.getFloat(XML_EMITTER_ENVIRONMENT_LIGHT_SCALE, DEFAULT_EMITTER_ENVIRONMENT_SCALE);
    m_toWorld = propList.getTransform(XML_EMITTER_ENVIRONMENT_LIGHT_TO_WORLD, DEFAULT_EMITTER_ENVIRONMENT_TO_WORLD);
    m_type = EEmitterType::EEnvironment;
    filesystem::path Filename = getFileResolver()->resolve(propList.getString(XML_EMITTER_ENVIRONMENT_LIGHT_FILENAME));
    m_name = Filename.str();

    m_pEnvironmentMap.reset(new Bitmap(m_name));
    m_toLocal = m_toWorld.inverse();

    std::vector<float> luminance(m_pEnvironmentMap->size());

    for (std::ptrdiff_t y = 0; y < m_pEnvironmentMap->rows(); y++)
    {
        float theta = float(y + 0.5f) / m_pEnvironmentMap->rows() * float(M_PI);
        float sinTheta = std::sin(theta);
        for (std::ptrdiff_t x = 0; x < m_pEnvironmentMap->cols(); x++)
        {
            // Ref : PBRT P845-850
            luminance[y * m_pEnvironmentMap->cols() + x] = m_pEnvironmentMap->coeff(y, x).getLuminance() * sinTheta;
        }
    }

    m_pPdf.reset(new DiscretePDF2D(luminance.data(), int(m_pEnvironmentMap->cols()), int(m_pEnvironmentMap->rows())));
}

Color3f EnvironmentLight::sample(EmitterQueryRecord & record, const Point2f & sample2D, float sample1D) const
{
    // Ref : PBRT P845-850
    float mapPdf;
    Point2i idx;
    Point2f uv = m_pPdf->sampleContinuous(sample2D, &mapPdf, &idx);

    float theta = uv.y() * float(M_PI);
    float sinTheta = std::sin(theta);

    if (mapPdf == 0.0f)
    {
        return Color3f(0.0f);
    }

    if (sinTheta != 0.0f)
    {
        record.pdf = mapPdf / (2.0f * float(M_PI * M_PI) * sinTheta);
    }
    else
    {
        return Color3f(0.0f);
    }

    float phi = uv.x() * float(2.0 * M_PI);
    record.wi = m_toWorld * sphericalDirection(theta, phi);
    record.n = -record.wi;
    record.pEmitter = this;
    record.p = record.ref + record.wi * record.distance * 2.0f;
    Color3f radiance = m_pEnvironmentMap->coeff(idx.y(), idx.x());
    if (!radiance.isValid())
    {
        return Color3f(0.0f);
    }
    return radiance * m_scale / record.pdf;
}

float EnvironmentLight::pdf(const EmitterQueryRecord & record) const
{
    // Ref : PBRT P845-850
    Point2f spherical = sphericalCoordinates(m_toLocal * record.wi);
    float theta = spherical.x();
    float phi = spherical.y();
    float sinTheta = std::sin(theta);
    if (sinTheta == 0.0f)
    {
        return 0.0f;
    }
    return m_pPdf->pdf(Point2f(phi / float(2.0 * M_PI), theta / float(M_PI))) / (2.0f * float(M_PI * M_PI) * sinTheta);
}

Color3f EnvironmentLight::eval(const EmitterQueryRecord & record) const
{
    Point2f spherical = sphericalCoordinates(m_toLocal * record.wi);
    float theta = spherical.x();
    float phi = spherical.y();

    if (std::isnan(theta) || std::isnan(phi))
    {
        return Color3f(0.0f);
    }

    float x = clamp(phi / float(2.0 * M_PI) * m_pEnvironmentMap->cols(), 0.0f, float(m_pEnvironmentMap->cols() - 1.0f));
    float y = clamp(theta / float(M_PI) * m_pEnvironmentMap->rows(), 0.0f, float(m_pEnvironmentMap->rows() - 1.0f));
    Color3f radiance = m_pEnvironmentMap->coeff(int(y), int(x));
    if (!radiance.isValid())
    {
        return Color3f(0.0f);
    }
    return radiance * m_scale;
}

std::string EnvironmentLight::toString() const
{
    return tfm::format(
            "EnvironmentLight[\n"
            "  filename = %s,\n"
            "  scale = %f,\n"
            "  toWorld = %s\n"
            "]",
            m_name,
            m_scale,
            indent(m_toWorld.toString(), 12)
    );
}

NORI_REGISTER_CLASS(EnvironmentLight, XML_EMITTER_ENVIRONMENT_LIGHT);
NORI_NAMESPACE_END

