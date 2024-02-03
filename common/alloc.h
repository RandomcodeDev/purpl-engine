/// @file alloc.h
///
/// @brief This module defines wrappers for various memory management functions.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "common.h"

/// @brief Get size of block allocated by CmnAlloc or similar
///
/// @param Block The block of memory to get the size of
///
/// @return The size of the block of memory
#ifdef PURPL_USE_MIMALLOC
#define CmnAllocSize(Block) mi_usable_size(Block)
#else
#ifdef PURPL_WIN32
#define CmnAllocSize(Block) _msize(Block)
#elif defined PURPL_MACOS
#define CmnAllocSize(Block) malloc_size(Block)
#else
#define CmnAllocSize(Block) malloc_usable_size(Block)
#endif
#endif

/// @brief Allocate memory
///
/// @param Count The number of elements to allocate
/// @param Size The size of an element
///
/// @return A block of memory
#ifdef PURPL_USE_MIMALLOC
#define CmnAlloc(Count, Size) mi_calloc(Count, Size)
#else
#define CmnAlloc(Count, Size) calloc(Count, Size)
#endif

/// @brief Resize an allocated block of memory
///
/// @param Block The block of memory to resize
/// @param Size The new size of the block
///
/// @return A block of memory with the requested size and
/// the same data as the old block
#ifdef PURPL_USE_MIMALLOC
#define CmnRealloc(Block, Size) mi_realloc(Block, Size)
#else
#define CmnRealloc(Block, Size) realloc(Block, Size)
#endif

/// @brief Allocate aligned memory
///
/// @param Alignment The alignment of the memory
/// @param Size The size of the memory
/// 
/// @return A block of memory
#ifdef PURPL_USE_MIMALLOC
#define CmnAlignedAlloc(Alignment, Size) mi_aligned_alloc(Alignment, Size)
#else
#ifdef PURPL_WIN32
#define CmnAlignedAlloc(Alignment, Size) _aligned_malloc(Alignment, Size)
#else
#define CmnAlignedAlloc(Alignment, Size) aligned_alloc(Alignment, Size)
#endif
#endif

/// @brief Free memory
/// 
/// @param Block The block of memory to free
#ifdef PURPL_USE_MIMALLOC
#define CmnFree(Block)                                                         \
    {                                                                          \
        (Block) ? mi_free(Block) : (VOID)0;                                    \
        (Block) = NULL;                                                        \
    }
#else
#define CmnFree(Block)                                                         \
    {                                                                          \
        (Block) ? free(Block) : (VOID)0;                                       \
        (Block) = NULL;                                                        \
    }
#endif

/// @brief Free aligned memory
/// 
/// @param Block The block of memory to free
#ifdef PURPL_USE_MIMALLOC
#define CmnAlignedFree(Block)                                                  \
    {                                                                          \
        (Block) ? mi_free(Block) : (VOID)0;                                    \
        (Block) = NULL;                                                        \
    }
#else
#ifdef PURPL_WIN32
#define CmnAlignedFree(Block)                                                  \
    {                                                                          \
        (Block) ? _aligned_free(Block) : (VOID)0;                              \
        (Block) = NULL;                                                        \
    }
#else
#define CmnAlignedFree(Block)                                                  \
    {                                                                          \
        (Block) ? free(Block) : (VOID)0;                                       \
        (Block) = NULL;                                                        \
    }
#endif
#endif

/// @brief Resize aligned memory
/// 
/// @param Block The block of memory to resize
/// @param Alignment The new alignment
/// @param Size The new size
/// 
/// @return A block of memory with the same data as the old block
#ifdef PURPL_USE_MIMALLOC
#define CmnAlignedRealloc(Block, Alignment, Size)                              \
    mi_aligned_recalloc(Block, 1, Size, Alignment)
#else
#ifdef PURPL_WIN32
#define CmnAlignedRealloc(Block, Alignment, Size)                              \
    _aligned_realloc(Block, 1, Size, Alignment)
#else
extern PVOID CmnAlignedRealloc(PVOID Block, SIZE_T Alignment, SIZE_T Size);
#endif
#endif
