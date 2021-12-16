//
// Created by superqqli on 2021/12/8.
//

#include <nori/filter/boxFilter.h>

NORI_NAMESPACE_BEGIN

BoxFilter::BoxFilter(const PropertyList &)
{
    m_radius = 0.5f;
}

float BoxFilter::eval(float) const
{
    return 1.0f;
}

std::string BoxFilter::toString() const
{
    return "boxFilter[]";
}

NORI_REGISTER_CLASS(BoxFilter, "box");

NORI_NAMESPACE_END
