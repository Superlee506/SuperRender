//
// Created by superqqli on 2021/12/8.
//

#include <nori/filter/mitchellNetravaliFilter.h>

NORI_NAMESPACE_BEGIN

MitchellNetravaliFilter::MitchellNetravaliFilter(const PropertyList &propList)
{
    /* Filter size in pixels */
    m_radius = propList.getFloat("radius", 2.0f);
    /* B parameter from the paper */
    m_B = propList.getFloat("B", 1.0f / 3.0f);
    /* C parameter from the paper */
    m_C = propList.getFloat("C", 1.0f / 3.0f);
}

float MitchellNetravaliFilter::eval(float x) const
{
    x = std::abs(2.0f * x / m_radius);
    float x2 = x*x, x3 = x2*x;

    if (x < 1) {
        return 1.0f/6.0f * ((12-9*m_B-6*m_C)*x3
                            + (-18+12*m_B+6*m_C) * x2 + (6-2*m_B));
    } else if (x < 2) {
        return 1.0f/6.0f * ((-m_B-6*m_C)*x3 + (6*m_B+30*m_C) * x2
                            + (-12*m_B-48*m_C)*x + (8*m_B + 24*m_C));
    } else {
        return 0.0f;
    }
}

std::string MitchellNetravaliFilter::toString() const
{
    return tfm::format("MitchellNetravaliFilter[radius=%f, B=%f, C=%f]", m_radius, m_B, m_C);
}

NORI_REGISTER_CLASS(MitchellNetravaliFilter, "mitchell");

NORI_NAMESPACE_END
