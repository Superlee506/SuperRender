//
// Created by superqqli on 2021/12/13.
//

#pragma once
#include <nori/core/common.h>

NORI_NAMESPACE_BEGIN

/**
* \brief Discrete probability distribution
*
* This data structure can be used to transform uniformly distributed
* samples to a stored discrete probability distribution.
*/

struct DiscretePDF1D
{
public:
    friend struct DiscretePDF2D;

    DiscretePDF1D(const float * pFunc, const int& count);

    int count() const;

    float sampleContinuous(const float& sample, float * pPdf = nullptr, int * pIdx = nullptr) const;

    int sampleDiscrete(const float& sample, float * pPdf = nullptr, float * pSampleRemapped = nullptr) const;

    float pdf(const int& index) const;

private:
    std::vector<float> m_cdf, m_func;
    float m_funcIntegral;
};


/**
* \brief Discrete probability distribution
*
* This data structure can be used to transform uniformly distributed
* samples to a stored discrete probability distribution.
*/
struct DiscretePDF2D
{
public:
    /// Construct a 2D distribution by the array data
    DiscretePDF2D(float * pDatas, const int& width, const int& height);

    Point2f sampleContinuous(const Point2f& sample, float * pPdf = nullptr, Point2i * idx = nullptr) const;

    float pdf(const Point2f& point) const;

private:
    std::vector<std::unique_ptr<DiscretePDF1D>> m_pConditionalRow;
    std::unique_ptr<DiscretePDF1D> m_pMarginalCol;
};

NORI_NAMESPACE_END
