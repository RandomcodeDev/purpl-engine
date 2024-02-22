/// @file alloc.h
///
/// @brief This file defines wrappers for various memory management functions.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "common.h"

/// @fn CmnAllocSize
///
/// @brief Get size of block allocated by CmnAlloc or similar
///
/// @param[in] Block The block of memory to get the size of
///
/// @return The size of the block of memory
#if PURPL_USE_MIMALLOC
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

/// @fn CmnAlloc
///
/// @brief Allocate memory
///
/// @param[in] Count The number of elements to allocate
/// @param[in] Size The size of an element
///
/// @return A block of memory
#if PURPL_USE_MIMALLOC
#define CmnAlloc(Count, Size) mi_calloc(Count, Size)
#else
#define CmnAlloc(Count, Size) calloc(Count, Size)
#endif

/// @fn CmnRealloc
///
/// @brief Resize an allocated block of memory
///
/// @param[in] Block The block of memory to resize
/// @param[in] Size The new size of the block
///
/// @return A block of memory with the requested size and
/// the same data as the old block
#if PURPL_USE_MIMALLOC
#define CmnRealloc(Block, Size) mi_realloc(Block, Size)
#else
#define CmnRealloc(Block, Size) realloc(Block, Size)
#endif

/// @fn CmnAlignedAlloc
///
/// @brief Allocate aligned memory
///
/// @param[in] Alignment The alignment of the memory
/// @param[in] Size The size of the memory
/// 
/// @return A block of memory
#if PURPL_USE_MIMALLOC
#define CmnAlignedAlloc(Alignment, Size) mi_aligned_alloc(Alignment, Size)
#else
#ifdef PURPL_WIN32
#define CmnAlignedAlloc(Alignment, Size) _aligned_malloc(Alignment, Size)
#else
#define CmnAlignedAlloc(Alignment, Size) aligned_alloc(Alignment, Size)
#endif
#endif

/// @fn CmnFree
///
/// @brief Free memory
/// 
/// @param[in,out] Block The block of memory to free
#if PURPL_USE_MIMALLOC
#define CmnFree(Block)                                                         \
    {                                                                          \
        (Block) ? mi_free((PVOID)(Block)) : (VOID)0;                           \
        (Block) = NULL;                                                        \
    }
#else
#define CmnFree(Block)                                                         \
    {                                                                          \
        (Block) ? free((PVOID)(Block)) : (VOID)0;                              \
        (Block) = NULL;                                                        \
    }
#endif

/// @fn CmnAlignedFree
///
/// @brief Free aligned memory
/// 
/// @param[in,out] Block The block of memory to free
#if PURPL_USE_MIMALLOC
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

/// @fn CmnAlignedRealloc
///
/// @brief Resize aligned memory
/// 
/// @param[in] Block The block of memory to resize
/// @param[in] Alignment The new alignment
/// @param[in] Size The new size
/// 
/// @return A block of memory with the same data as the old block
#if PURPL_USE_MIMALLOC
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
