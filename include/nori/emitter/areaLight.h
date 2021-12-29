//
// Created by superqqli on 2021/12/13.
//
#pragma once
#include <nori/core/common.h>
#include <nori/core/emitter.h>

NORI_NAMESPACE_BEGIN
class AreaLight : public Emitter
{
public:
    AreaLight(const PropertyList & propList);

    virtual Color3f sample(EmitterQueryRecord & Record, const Point2f & Sample2D, float Sample1D) const override;

    virtual float pdf(const EmitterQueryRecord & record) const override;

    virtual Color3f eval(const EmitterQueryRecord & record) const override;

    virtual void setParent(NoriObject * pParentObj, const std::string &name) override;

    virtual std::string toString() const override;

protected:
    Color3f m_radiance;
};

NORI_NAMESPACE_END