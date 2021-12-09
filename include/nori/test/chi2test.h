/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/
#pragma once

#include <nori/core/common.h>
#include <nori/core/object.h>

/*
 * =======================================================================
 *   WARNING    WARNING    WARNING    WARNING    WARNING    WARNING
 * =======================================================================
 *   Remember to put on SAFETY GOGGLES before looking at this file. You
 *   are most certainly not expected to read or understand any of it.
 * =======================================================================
 */

NORI_NAMESPACE_BEGIN

/**
 * \brief Statistical test for validating that an importance sampling routine
 * (e.g. from a BSDF) produces a distribution that agrees with what the
 * implementation claims via its associated density function.
 */
class ChiSquareTest : public NoriObject {
public:
    ChiSquareTest(const PropertyList &propList);

    virtual ~ChiSquareTest();

    void addChild(NoriObject *obj);

    /// Execute the chi-square test
    void activate();

    std::string toString() const;

    EClassType getClassType() const;

private:
    int m_cosThetaResolution;
    int m_phiResolution;
    int m_minExpFrequency;
    int m_sampleCount;
    int m_testCount;
    float m_significanceLevel;
    std::vector<BSDF *> m_bsdfs;
};

NORI_NAMESPACE_END
