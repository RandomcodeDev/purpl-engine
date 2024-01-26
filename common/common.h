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

#ifdef PURPL_WIN32
#define PURPL_MSIZE(Block) _msize(block);
#elif defined PURPL_MACOS
#define PURPL_MSIZE(Block) malloc_size(block);
#else
#define PURPL_MSIZE(Block) malloc_usable_size(block);
#endif

//
// Allocate memory
//

#ifdef PURPL_USE_MIMALLOC
#define PURPL_ALLOC(Count, Size) mi_calloc(Count, Size)
#else
#define PURPL_ALLOC(Count, Size) calloc(Count, Size)
#endif

//
// Reallocate memory
//

#ifdef PURPL_USE_MIMALLOC
#define PURPL_REALLOC(Block, Size) mi_realloc(Block, Size)
#else
#define PURPL_REALLOC(Block, Size) realloc(Block, Size)
#endif

//
// Allocate aligned memory
//

#ifdef PURPL_USE_MIMALLOC
#define PURPL_ALIGNED_ALLOC(Alignment, Size) mi_aligned_alloc(Alignment, Size)
#else
#ifdef PURPL_WIN32
#define PURPL_ALIGNED_ALLOC(Alignment, Size) _aligned_malloc(Alignment, Size)
#else
#define PURPL_ALIGNED_ALLOC(Alignment, Size) aligned_alloc(Alignment, Size)
#endif
#endif

//
// Reallocate aligned memory
//

#ifdef PURPL_USE_MIMALLOC
#define PURPL_ALIGNED_REALLOC(Block, Alignment, Size) mi_aligned_recalloc(Block, 1, Size, Alignment)
#else
#ifdef PURPL_WIN32
#define PURPL_ALIGNED_REALLOC(Block, Alignment, Size) _aligned_realloc(Block, 1, Size, Alignment)
#else
#define PURPL_ALIGNED_REALLOC(Block, Alignment, Size) \
{ \
    PVOID NewBlock = PURPL_ALIGNED_ALLOC(Alignment, Size); \
    memmove(
        NewBlock,
        Block,
        PURPL_MIN(Size, PURPL_MSIZE(Block))
        );
    PURPL_FREE(Block);
}
#endif
#endif

//
// Free memory
//

#ifdef PURPL_USE_MIMALLOC
#define PURPL_FREE(Block) { (Block) ? mi_free(Block) : (VOID)0; (Block) = NULL; }
#else
#define PURPL_FREE(Block) { (Block) ? free(Block) : (VOID)0; (Block) = NULL; }
#endif

//
// Free aligned memory
//

#ifdef PURPL_USE_MIMALLOC
#define PURPL_ALIGNED_FREE(Block) { (Block) ? mi_free(Block) : (VOID)0; (Block) = NULL; }
#else
#ifdef PURPL_WIN32
#define PURPL_ALIGNED_FREE(Block) { (Block) ? _aligned_free(Block) : (VOID)0; (Block) = NULL; }
#else
#define PURPL_ALIGNED_FREE(Block) { (Block) ? free(Block) : (VOID)0; (Block) = NULL; }
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
