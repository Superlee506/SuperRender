//
// Created by superqqli on 2021/12/13.
//

#pragma once
#include <nori/core/discretePDF.h>
#include <nori/core/vector.h>

NORI_NAMESPACE_BEGIN

DiscretePDF1D::DiscretePDF1D(const float * pFunc, const int& count) :
        m_func(pFunc, pFunc + count), m_cdf(count + 1)
{
    m_cdf[0] = 0.0f;
    for (int i = 1; i < count + 1; i++)
    {
        m_cdf[i] = m_cdf[i - 1] + m_func[i - 1] / count;
    }

    m_funcIntegral = m_cdf[count];

    if (m_funcIntegral < Epsilon)
    {
        for (int i = 1; i < count + 1; i++)
        {
            m_cdf[i] = float(i) / float(count);
        }
    }
    else
    {
        for (int i = 1; i < count + 1; i++)
        {
            m_cdf[i] /= m_funcIntegral;
        }
    }
}

int DiscretePDF1D::count() const
{
    return int(m_func.size());
}

float DiscretePDF1D::sampleContinuous(const float& sample, float * pPdf, int * pIdx) const
{
    auto iter = std::lower_bound(m_cdf.begin(), m_cdf.end(), sample);
    int idx = int(std::min(
            size_t(std::max(std::ptrdiff_t(0), iter - m_cdf.begin() - 1)),
            m_cdf.size() - 2
    ));

    if (pIdx != nullptr)
    {
        *pIdx = idx;
    }

    float du = sample - m_cdf[idx];
    if (m_cdf[idx + 1] - m_cdf[idx] > 0.0f)
    {
        du /= (m_cdf[idx + 1] - m_cdf[idx]);
    }
    CHECK(!std::isnan(du));

    if (pPdf != nullptr)
    {
        *pPdf = m_func[idx] / m_funcIntegral;
    }

    return (float(idx) + du) / float(count());
}


int DiscretePDF1D::sampleDiscrete(const float& sample, float * pPdf, float * pSampleRemapped) const
{
    auto iter = std::lower_bound(m_cdf.begin(), m_cdf.end(), sample);
    int idx = int(std::min(
            size_t(std::max(std::ptrdiff_t(0), iter - m_cdf.begin() - 1)),
            m_cdf.size() - 2
    ));

    if (pPdf != nullptr)
    {
        *pPdf = m_func[idx] / (m_funcIntegral * float(count()));
    }

    if (pSampleRemapped != nullptr)
    {
        *pSampleRemapped = (sample - m_cdf[idx]) / (m_cdf[idx + 1] - m_cdf[idx]);
        CHECK(*pSampleRemapped <= 1.0f && *pSampleRemapped >= 0.0f);
    }

    return idx;
}

float DiscretePDF1D::pdf(const int& index) const
{
    CHECK(index >= 0 && index < count());
    return m_func[index] / (m_funcIntegral * float(count()));
}



DiscretePDF2D::DiscretePDF2D(float * pDatas, const int& width, const int& height)
{
    m_pConditionalRow.reserve(height);
    for (int i = 0; i < height; i++)
    {
        m_pConditionalRow.emplace_back(new DiscretePDF1D(&pDatas[i * width], width));
    }

    std::vector<float> marginal;
    marginal.reserve(height);
    for (int i = 0; i < height; i++)
    {
        marginal.push_back(m_pConditionalRow[i]->m_funcIntegral);
    }
    m_pMarginalCol.reset(new DiscretePDF1D(marginal.data(), height));
}

Point2f DiscretePDF2D::sampleContinuous(const Point2f& sample, float * pPdf, Point2i * idx) const
{
    float pdfX, pdfY;
    int idxX, idxY;
    float dy = m_pMarginalCol->sampleContinuous(sample.y(), &pdfY, &idxY);
    float dx = m_pConditionalRow[idxY]->sampleContinuous(sample.x(), &pdfX, &idxX);
    if (pPdf != nullptr)
    {
        *pPdf = pdfX * pdfY;
    }
    if (idx != nullptr)
    {
        *idx = Point2i(idxX, idxY);
    }
    return Point2f(dx, dy);
}

float DiscretePDF2D::pdf(const Point2f& point) const
{
    int x = clamp(int(point.x() * m_pConditionalRow[0]->count()), 0, m_pConditionalRow[0]->count() - 1);
    int y = clamp(int(point.y() * m_pMarginalCol->count()), 0, m_pMarginalCol->count() - 1);
    return m_pConditionalRow[y]->m_func[x] / m_pMarginalCol->m_funcIntegral;
}

NORI_NAMESPACE_END
