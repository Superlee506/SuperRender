//
// Created by superqqli on 2021/12/8.
//

#pragma once
#include <nori/core/rfilter.h>
NORI_NAMESPACE_BEGIN
/**
 * Separable reconstruction filter by Mitchell and Netravali
 *
 * D. Mitchell, A. Netravali, Reconstruction filters for computer graphics,
 * Proceedings of SIGGRAPH 88, Computer Graphics 22(4), pp. 221-228, 1988.
 */
class MitchellNetravaliFilter : public ReconstructionFilter {
public:
    MitchellNetravaliFilter(const PropertyList &propList);

    float eval(float x) const;

    std::string toString() const override;
protected:
    float m_B, m_C;
};
NORI_NAMESPACE_END