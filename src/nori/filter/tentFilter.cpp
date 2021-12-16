//
// Created by superqqli on 2021/12/8.
//

#include <nori/filter/tentFilter.h>

NORI_NAMESPACE_BEGIN

TentFilter::TentFilter(const PropertyList &)
{
    m_radius = 1.0f;
}

float TentFilter::eval(float x) const
{
    return std::max(0.0f, 1.0f - std::abs(x));
}

std::string TentFilter::toString() const
{
    return "TentFilter[]";
}

NORI_REGISTER_CLASS(TentFilter, "tent");

NORI_NAMESPACE_END
