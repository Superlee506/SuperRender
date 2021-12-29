//
// Created by superqqli on 2021/12/13.
//
#pragma once
#include <nori/core/common.h>
#include <nori/core/emitter.h>

NORI_NAMESPACE_BEGIN
class EnvironmentLight : public Emitter
{
public:
    EnvironmentLight(const PropertyList & propList);

    virtual Color3f sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const override;

    virtual float pdf(const EmitterQueryRecord & record) const override;

    virtual Color3f eval(const EmitterQueryRecord & record) const override;

    virtual std::string toString() const override;

protected:
    std::string m_name;
    float m_scale;
    Transform m_toWorld;
    Transform m_toLocal;
    std::unique_ptr<Bitmap> m_pEnvironmentMap = nullptr;
    std::unique_ptr<DiscretePDF2D> m_pPdf = nullptr;
};

NORI_NAMESPACE_END