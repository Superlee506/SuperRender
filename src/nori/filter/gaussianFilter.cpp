//
// Created by superqqli on 2021/12/8.
//

#include <nori/filter/gaussianFilter.h>
NORI_NAMESPACE_BEGIN
GaussianFilter::GaussianFilter(const PropertyList &propList)
{
    /* Half filter size */
    m_radius = propList.getFloat("radius", 2.0f);
    /* Standard deviation of the Gaussian */
    m_stddev = propList.getFloat("stddev", 0.5f);
}

float GaussianFilter::eval(float x) const
{
    float alpha = -1.0f / (2.0f * m_stddev*m_stddev);
    return std::max(0.0f,
                    std::exp(alpha * x * x) -
                    std::exp(alpha * m_radius * m_radius));
}

std::string GaussianFilter::toString() const
{
    return tfm::format("GaussianFilter[radius=%f, stddev=%f]", m_radius, m_stddev);
}

NORI_REGISTER_CLASS(GaussianFilter, "gaussian");

NORI_NAMESPACE_END
