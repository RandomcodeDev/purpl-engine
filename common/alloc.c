/// @file alloc.c
///
/// @brief This file implements any wrappers from alloc.h that can't be
///  implemented with macros
///
/// @copyright (c) 2024 Randomcode Developers

#include "alloc.h"

// Only on non-Windows, non-mimalloc
#ifndef CmnAlignedRealloc
PVOID
CmnAlignedRealloc(PVOID Block, SIZE_T Alignment, SIZE_T Size)
{
    PVOID NewBlock = CmnAlignedAlloc(Alignment, Size);
    memmove(NewBlock, Block, PURPL_MIN(Size, CmnAllocSize(Block)));
    CmnAlignedFree(Block);
    return NewBlock;
}
#endif
