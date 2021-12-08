//
// Created by superqqli on 2021/12/8.
//

#pragma once
#include <nori/core/rfilter.h>
NORI_NAMESPACE_BEGIN
/// Tent filter
class TentFilter : public ReconstructionFilter {
public:
    TentFilter(const PropertyList &);

    float eval(float x) const;

    std::string toString() const override;
};
NORI_NAMESPACE_END
