//
// Created by superqqli on 2021/12/8.
//

#pragma once
#include <nori/core/rfilter.h>
NORI_NAMESPACE_BEGIN
/// Box filter -- fastest, but prone to aliasing
class BoxFilter : public ReconstructionFilter {
public:
    BoxFilter(const PropertyList &);

    float eval(float) const;

    std::string toString() const override;
};
NORI_NAMESPACE_END
