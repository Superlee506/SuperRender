/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once
#include <nori/core/common.h>
#include <nori/core/sampler.h>
#include <pcg32.h>

NORI_NAMESPACE_BEGIN

/**
 * Independent sampling - returns independent uniformly distributed
 * random numbers on <tt>[0, 1)x[0, 1)</tt>.
 *
 * This class is essentially just a wrapper around the pcg32 pseudorandom
 * number generator. For more details on what sample generators do in
 * general, refer to the \ref Sampler class.
 */
class IndependentSampler : public Sampler {
public:
    IndependentSampler(const PropertyList &propList);

    virtual ~IndependentSampler();

    virtual std::unique_ptr<Sampler> clone() const override;

    virtual void prepare(const ImageBlock &block) override;

    virtual void generate() override;
    virtual void advance() override;

    virtual float next1D() override;

    virtual Point2f next2D() override;

    virtual std::string toString() const override;
protected:
    IndependentSampler();

private:
    pcg32 m_random;
};

NORI_NAMESPACE_END
