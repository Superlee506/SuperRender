//
// Created by superqqli on 2021/12/12.
//

#pragma once
#include <nori/core/common.h>
#include <list>

NORI_NAMESPACE_BEGIN
void * allocAligned(size_t Size);

template <typename T>
inline T * allocAligned(size_t Count)
{
    return reinterpret_cast<T*>(allocAligned(Count * sizeof(T)));
}

void freeAligned(void * pPtr);

class MemoryArena
{
public:
    MemoryArena(size_t blockSize = 262144/*256KB*/);
    MemoryArena(const MemoryArena &) = delete;
    MemoryArena& operator=(const MemoryArena &) = delete;

    ~MemoryArena();

    void * alloc(size_t nBytes);

    template <typename T>
    T * alloc(size_t N = 1, bool bRunConstructor = true)
    {
        T * pRet = reinterpret_cast<T*>(alloc(N * sizeof(T)));
        if (bRunConstructor)
        {
            for (size_t i = 0; i < N; ++i)
            {
                new (&pRet[i]) T();
            }
        }
        return pRet;
    }

    void reset();

    void release();

    size_t totalAllocated() const;

private:
    const size_t m_nBlockSize;
    size_t m_iCurrentBlockPos = 0, m_iCurrentAllocSize = 0;
    uint8_t * m_pCurrentBlock = nullptr;
    std::list<std::pair<size_t, uint8_t*>> m_usedBlocks, m_availableBlocks;
};


// Cache optimization for 2D array(e.g. texture etc.)
template <typename T, int LogBlockSize>
class BlockedArray
{
private:
    T * m_pData;
    const int m_URes, m_VRes, m_UBlocks;

public:
    BlockedArray(int URes, int VRes, const T * pData = nullptr) :
            m_URes(URes), m_VRes(VRes), m_UBlocks(RoundUp(m_URes) >> LogBlockSize)
    {
        // The block sizes are always powers of 2,
        int nAlloc = RoundUp(m_URes) * RoundUp(m_VRes);

        m_pData = AllocAligned<T>(nAlloc);
        for (int i = 0; i < nAlloc; ++i)
        {
            new (&m_pData[i]) T();
        }

        if (pData != nullptr)
        {
            for (int v = 0; v < m_VRes; ++v)
            {
                for (int u = 0; u < m_URes; ++u)
                {
                    (*this)(u, v) = pData[v * m_URes + u];
                }
            }
        }
    }

    ~BlockedArray()
    {
        for (int i = 0; i < m_URes * m_VRes; ++i)
        {
            m_pData[i].~T();
        }

        FreeAligned(m_pData);
    }

    constexpr int blockSize() const { return 1 << LogBlockSize; }

    int roundUp(int X) const
    {
        return (X + BlockSize() - 1) & ~(BlockSize() - 1);
    }

    int uSize() const
    {
        return m_URes;
    }

    int vSize() const
    {
        return m_VRes;
    }

    int block(int A) const
    {
        return A >> LogBlockSize;
    }

    int offset(int A) const
    {
        return (A & (BlockSize() - 1));
    }

    T & operator()(int U, int V)
    {
        int BU = Block(U), BV = Block(V);
        int OU = Offset(U), OV = Offset(V);
        int Offset = BlockSize() * BlockSize() * (m_UBlocks * BV + BU);
        Offset += BlockSize() * OV + OU;
        return m_pData[Offset];
    }

    const T & operator()(int U, int V) const
    {
        int BU = Block(U), BV = Block(V);
        int OU = Offset(U), OV = Offset(V);
        int Offset = BlockSize() * BlockSize() * (m_UBlocks * BV + BU);
        Offset += BlockSize() * OV + OU;
        return m_pData[Offset];
    }

    void getLinearArray(T * pArray) const
    {
        for (int v = 0; v < m_VRes; ++v)
        {
            for (int u = 0; u < m_URes; ++u)
            {
                *pArray++ = (*this)(u, v);
            }
        }
    }
};

NORI_NAMESPACE_END
