//
// Created by superqqli on 2021/12/8.
//
#pragma once
#include <nori/core/rfilter.h>
NORI_NAMESPACE_BEGIN
/**
* Windowed Gaussian filter with configurable extent
* and standard deviation. Often produces pleasing
* results, but may introduce too much blurring.
*/
class GaussianFilter : public ReconstructionFilter
{
public:
    GaussianFilter(const PropertyList &propList);

    virtual float eval(float x) const override;

    virtual std::string toString() const override;

protected:
    float m_stddev;
};

NORI_NAMESPACE_END