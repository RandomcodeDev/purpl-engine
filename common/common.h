/// @file common.h
///
/// @brief This file contains definitions for the common library.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "platform/platform.h"

#include "log.h"

//
// Quit if a condition is false
//

#define PURPL_ASSERT(Condition)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        if (!(Condition))                                                                                              \
            CmnError("Assertion failed: " #Condition);                                                                 \
    } while (0);

/// @brief Size of an array
#define PURPL_ARRAYSIZE(Array) (sizeof(Array) / sizeof((Array)[0]))

/// @brief Larger of two values
#define PURPL_MAX(A, B) ((A) > (B) ? (A) : (B))

/// @brief Smaller of two values
#define PURPL_MIN(A, B) ((A) < (B) ? (A) : (B))

/// @brief Clamp a value to a range
#define PURPL_CLAMP(Value, Min, Max) ((Value) > (Max) ? (Max) : (Value) < (Min) ? (Min) : (Value))

/// @brief Make a string
#define PURPL_STRINGIZE(X) #X

/// @brief Make a string, expanding one level of macros
#define PURPL_STRINGIZE_EXPAND(X) PURPL_STRINGIZE(X)

/// @brief Round a size to an aligment
///
/// @param[in] Alignment The alignment
/// @param[in] Size The unrounded size
///
/// @return The size rounded to be aligned to the alignment
#define PURPL_ALIGN(Alignment, Size) ((((Size) / (Alignment)) + 1) * (Alignment))

#define PURPL_SWAP(Type, A, B)                                                                                         \
    do                                                                                                                 \
    {                                                                                                                  \
        Type Swap_ = A;                                                                                                \
        A = B;                                                                                                         \
        B = A;                                                                                                         \
    } while (0)

/// @brief Initialize the common library
///
/// @param[in] Arguments The command line arguments from PurplMain
/// @param[in] ArgumentCount The number of command line arguments
extern VOID CmnInitialize(_In_opt_ PCHAR *Arguments, _In_opt_ UINT ArgumentCount);

/// @brief Shut down the common library
extern VOID CmnShutdown(VOID);

/// @brief This routine formats a printf format string into a static
///        buffer for temporary usage.
///
/// @param[in] Format     The format string. You're making a bad decision if this
///                   parameter is not a string literal.
/// @param[in] Arguments  Arguments to the format string.
///
/// @return A pointer to a static buffer with the formatted string.
extern PCSTR CmnFormatTempString(_In_ _Printf_format_string_ PCSTR Format, ...);

/// @brief This routine formats a printf format string into a static
///        buffer for temporary usage.
///
/// @param[in] Format  The format string. You're making a bad decision if this
///                parameter is not a string literal.
/// @param[in] ...     Arguments to the format string.
///
/// @return A pointer to a static buffer with the formatted string.
extern PCSTR CmnFormatTempStringVarArgs(_In_ _Printf_format_string_ PCSTR Format, _In_ va_list Arguments);

/// @brief This routine formats a printf format string into a dynamically
/// allocated
///        buffer.
///
/// @param[in] Format     The format string. You're making a bad decision if this
///                   parameter is not a string literal.
/// @param[in] Arguments  Arguments to the format string.
///
/// @return A pointer to a buffer with the formatted string and a NUL terminator
///         (size is strlen(Buffer) + 1).
extern PCHAR CmnFormatString(_In_ _Printf_format_string_ PCSTR Format, ...);

/// @brief This routine formats a printf format string into a dynamically
/// allocated
///        buffer.
///
/// @param[in] Format  The format string. You're making a bad decision if this
///                parameter is not a string literal.
/// @param[in] ...     Arguments to the format string.
///
/// @return A pointer to a buffer with the formatted string.
extern PCHAR CmnFormatStringVarArgs(_In_ _Printf_format_string_ PCSTR Format, _In_ va_list Arguments);

/// @brief This routine converts a size into a human-readable string, using the
///        most appropriate unit.
///
/// @param[in] Size The size to convert.
///
/// @return The address of a static buffer containing the string.
extern PCSTR CmnFormatSize(_In_ DOUBLE Size);

/// @brief This routine displays an error message and terminates the program.
///
/// @param[in] Message The error message.
/// @param[in] ...     The arguments to the error message.
_Noreturn extern VOID CmnError(_In_ _Printf_format_string_ PCSTR Message, ...);
