/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <nori/core/common.h>
#include <nori/core/object.h>
#include <hypothesis.h>
#include <pcg32.h>

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
 * Student's t-test for the equality of means
 *
 * This test analyzes whether the expected value of a random variable matches a
 * certain known value. When there is significant statistical "evidence"
 * against this hypothesis, the test fails.
 *
 * This is useful in checking whether a Monte Carlo method method converges
 * against the right value. Because statistical tests are able to handle the
 * inherent noise of these methods, they can be used to construct statistical
 * test suites not unlike the traditional unit tests used in software engineering.
 *
 * This implementation can be used to test two things:
 *
 * 1. that the illumination scattered by a BRDF model under uniform illumination
 *    into a certain direction matches a given value (modulo noise).
 *
 * 2. that the average radiance received by a camera within some scene
 *    matches a given value (modulo noise).
 */
class StudentsTTest : public NoriObject {
public:
    StudentsTTest(const PropertyList &propList);

    virtual ~StudentsTTest();

    void addChild(NoriObject *obj);

    /// Invoke a series of t-tests on the provided input
    void activate();

    std::string toString() const;

    EClassType getClassType() const;

private:
    std::vector<BSDF *> m_bsdfs;
    std::vector<Scene *> m_scenes;
    std::vector<float> m_angles;
    std::vector<float> m_references;
    float m_significanceLevel;
    int m_sampleCount;
};

NORI_NAMESPACE_END
