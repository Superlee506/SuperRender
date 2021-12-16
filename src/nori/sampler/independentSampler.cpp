/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <nori/sampler/independentSampler.h>
#include <nori/core/block.h>

NORI_NAMESPACE_BEGIN

IndependentSampler::IndependentSampler(const PropertyList &propList)
{
    m_sampleCount = (size_t) propList.getInteger("sampleCount", 1);
}

IndependentSampler::~IndependentSampler()
{

}

std::unique_ptr<Sampler> IndependentSampler::clone() const
{
    std::unique_ptr<IndependentSampler> cloned(new IndependentSampler());
    cloned->m_sampleCount = m_sampleCount;
    cloned->m_random = m_random;
    return std::move(cloned);
}

void IndependentSampler::prepare(const ImageBlock &block)
{
    m_random.seed(
            block.getOffset().x(),
            block.getOffset().y()
    );
}

void IndependentSampler::generate() { /* No-op for this sampler */ }
void IndependentSampler::advance()  { /* No-op for this sampler */ }

float IndependentSampler::next1D()
{
    return m_random.nextFloat();
}

Point2f IndependentSampler::next2D()
{
    return Point2f(
            m_random.nextFloat(),
            m_random.nextFloat()
    );
}

std::string IndependentSampler::toString() const
{
    return tfm::format("Independent[sampleCount=%i]", m_sampleCount);
}

IndependentSampler::IndependentSampler() { }


NORI_REGISTER_CLASS(IndependentSampler, "independent");
NORI_NAMESPACE_END
