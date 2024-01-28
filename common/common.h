/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    common.h

Abstract:

    This module contains definitions for the common library.

--*/

#pragma once

#include "purpl/purpl.h"

#include "platform/platform.h"

#include "log.h"

//
// Assert that a condition is true or kill the program
//

#define PURPL_ASSERT(Condition)                       \
do                                                    \
{                                                     \
    if (!(Condition))                                 \
        CmnError("Assertion failed: " #Condition); \
} while (0);

//
// Size of an array
//

#define PURPL_ARRAYSIZE(Array) (sizeof(Array) / sizeof((Array)[0]))

//
// Maximum of two values
//

#define PURPL_MAX(A, B) ((A) > (B) ? (A) : (B))

//
// Minimum of two values
//

#define PURPL_MIN(A, B) ((A) < (B) ? (A) : (B))

//
// Clamp a value to a range
//

#define PURPL_CLAMP(Value, Min, Max) ((Value) > (Max) ? (Max) : (Value) < (Min) ? (Min) : (Value))

//
// Make a string
//

#define PURPL_STRINGIZE(X) #X
#define PURPL_STRINGIZE_EXPAND(X) PURPL_STRINGIZE(X)

//
// Get size of block allocated by malloc
//

#ifdef PURPL_USE_MIMALLOC
#define PURPL_MSIZE(Block) mi_usable_size(Block)
#else
#ifdef PURPL_WIN32
#define PURPL_MSIZE(Block) _msize(Block)
#elif defined PURPL_MACOS
#define PURPL_MSIZE(Block) malloc_size(Block)
#else
#define PURPL_MSIZE(Block) malloc_usable_size(Block)
#endif
#endif

//
// Allocate memory
//

#ifdef PURPL_USE_MIMALLOC
#define CmnAlloc(Count, Size) mi_calloc(Count, Size)
#else
#define CmnAlloc(Count, Size) calloc(Count, Size)
#endif

//
// Reallocate memory
//

#ifdef PURPL_USE_MIMALLOC
#define CmnRealloc(Block, Size) mi_realloc(Block, Size)
#else
#define CmnRealloc(Block, Size) realloc(Block, Size)
#endif

//
// Allocate aligned memory
//

#ifdef PURPL_USE_MIMALLOC
#define CmnAlignedAlloc(Alignment, Size) mi_aligned_alloc(Alignment, Size)
#else
#ifdef PURPL_WIN32
#define CmnAlignedAlloc(Alignment, Size) _aligned_malloc(Alignment, Size)
#else
#define CmnAlignedAlloc(Alignment, Size) aligned_alloc(Alignment, Size)
#endif
#endif

//
// Free memory
//

#ifdef PURPL_USE_MIMALLOC
#define CmnFree(Block) { (Block) ? mi_free(Block) : (VOID)0; (Block) = NULL; }
#else
#define CmnFree(Block) { (Block) ? free(Block) : (VOID)0; (Block) = NULL; }
#endif

//
// Free aligned memory
//

#ifdef PURPL_USE_MIMALLOC
#define CmnAlignedFree(Block) { (Block) ? mi_free(Block) : (VOID)0; (Block) = NULL; }
#else
#ifdef PURPL_WIN32
#define CmnAlignedFree(Block) { (Block) ? _aligned_free(Block) : (VOID)0; (Block) = NULL; }
#else
#define CmnAlignedFree(Block) { (Block) ? free(Block) : (VOID)0; (Block) = NULL; }
#endif
#endif

//
// Reallocate aligned memory
//

#ifdef PURPL_USE_MIMALLOC
#define CmnAlignedRealloc(Block, Alignment, Size) mi_aligned_recalloc(Block, 1, Size, Alignment)
#else
#ifdef PURPL_WIN32
#define CmnAlignedRealloc(Block, Alignment, Size) _aligned_realloc(Block, 1, Size, Alignment)
#else
extern
PVOID
CmnAlignedRealloc(
    PVOID Block,
    SIZE_T Alignment,
    SIZE_T Size
    );
#endif
#endif

//
// Align a size
//

#define PURPL_ALIGN(Alignment, Size) ((((Size) / (Alignment)) + 1) * (Alignment))

//
// Initialize the common library
//

extern
VOID
CmnInitialize(
    VOID
    );

//
// Shut down the common library
//

extern
VOID
CmnShutdown(
    VOID
    );

//
// Format a string into a temporary buffer
//

extern
PCSTR
CmnFormatTempString(
    _In_ _Printf_format_string_ PCSTR Format,
    ...
    );

//
// Format a string into a temporary buffer
//

extern
PCSTR
CmnFormatTempStringVarArgs(
    _In_ _Printf_format_string_ PCSTR Format,
    _In_ va_list Arguments
    );

//
// Format a string
//

extern
PCHAR
CmnFormatString(
    _In_ _Printf_format_string_ PCSTR Format,
    ...
    );

//
// Format a string
//

extern
PCHAR
CmnFormatStringVarArgs(
    _In_ _Printf_format_string_ PCSTR Format,
    _In_ va_list Arguments
    );

//
// Format a size
//

extern
PCSTR
CmnFormatSize(
    _In_ DOUBLE Size
    );

//
// Display an error message and exit
//

extern
_Noreturn
VOID
CmnError(
    _In_ _Printf_format_string_ PCSTR Message,
    ...
    );
