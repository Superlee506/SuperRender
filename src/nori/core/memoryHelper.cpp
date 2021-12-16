//
// Created by superqqli on 2021/12/12.
//
#include <nori/core/memoryHelper.h>

NORI_NAMESPACE_BEGIN
void * allocAligned(size_t Size)
{
#if defined(PLATFORM_WINDOWS)
    return _aligned_malloc(Size, 64);
#else
    return memalign(64, Size);
#endif
}

void freeAligned(void * pPtr)
{
    if (pPtr != nullptr)
    {
#if defined(PLATFORM_WINDOWS)
        _aligned_free(pPtr);
#else
        free(pPtr);
#endif
    }
}


MemoryArena::MemoryArena(size_t BlockSize) : m_nBlockSize(BlockSize) { }

MemoryArena::~MemoryArena()
{
    release();
}

void * MemoryArena::alloc(size_t nBytes)
{
    // Round up _nBytes_ to minimum machine alignment
    const int Align = alignof(std::max_align_t);

    static_assert((Align && !(Align & (Align - 1))), "Minimum alignment not a power of two");

    nBytes = (nBytes + Align - 1) & ~(Align - 1);

    if (m_iCurrentBlockPos + nBytes > m_iCurrentAllocSize)
    {
        // Add current block to _usedBlocks_ list
        if (m_pCurrentBlock)
        {
            m_usedBlocks.push_back(std::make_pair(m_iCurrentAllocSize, m_pCurrentBlock));
            m_pCurrentBlock = nullptr;
            m_iCurrentAllocSize = 0;
        }

        // Get new block of memory for _MemoryArena_

        // Try to get memory block from _availableBlocks_
        for (auto Iter = m_availableBlocks.begin(); Iter != m_availableBlocks.end(); ++Iter)
        {
            if (Iter->first >= nBytes)
            {
                m_iCurrentAllocSize = Iter->first;
                m_pCurrentBlock = Iter->second;
                m_availableBlocks.erase(Iter);
                break;
            }
        }

        if (m_pCurrentBlock == nullptr)
        {
            m_iCurrentAllocSize = std::max(nBytes, m_nBlockSize);
            m_pCurrentBlock = allocAligned<uint8_t>(m_iCurrentAllocSize);
        }

        m_iCurrentBlockPos = 0;
    }

    void * pRet = m_pCurrentBlock + m_iCurrentBlockPos;
    m_iCurrentBlockPos += nBytes;
    return pRet;
}

void MemoryArena::reset()
{
    m_iCurrentBlockPos = 0;
    m_availableBlocks.splice(m_availableBlocks.begin(), m_usedBlocks);
}

void MemoryArena::release()
{
    freeAligned(m_pCurrentBlock);
    for (auto & block : m_usedBlocks)
    {
        freeAligned(block.second);
    }
    for (auto & Block : m_availableBlocks)
    {
        freeAligned(Block.second);
    }

    m_pCurrentBlock = nullptr;

    m_iCurrentBlockPos = 0;
    m_iCurrentAllocSize = 0;

    m_usedBlocks.clear();
    m_availableBlocks.clear();
}

size_t MemoryArena::totalAllocated() const
{
    size_t total = m_iCurrentAllocSize;
    for (const auto & alloc : m_usedBlocks)
    {
        total += alloc.first;
    }
    for (const auto & alloc : m_availableBlocks)
    {
        total += alloc.first;
    }
    return total;
}


NORI_NAMESPACE_END