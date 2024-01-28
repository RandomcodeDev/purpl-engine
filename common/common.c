/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    common.c

Abstract:

    This module implements assorted common functions.

--*/

#include "common.h"

#include "filesystem.h"

VOID
CmnInitialize(
    VOID
    )
{
    LOG_LEVEL Level;

    PlatInitialize();

#ifdef PURPL_USE_MIMALLOC
    LogInfo("Using mimalloc allocator");

    mi_option_set(mi_option_reserve_huge_os_pages, 2);
    mi_option_set(mi_option_show_errors, TRUE);
    mi_option_set(mi_option_show_stats, TRUE);
#else
    LogInfo("Using libc allocator");
#endif

    LogSetLock(
        NULL,
        NULL
        );

#ifdef PURPL_DEBUG
#ifdef PURPL_VERBOSE
    Level = LogLevelTrace;
#ifdef PURPL_USE_MIMALLOC
    mi_option_set(mi_option_verbose, TRUE);
#endif
#else
    Level = LogLevelDebug;
#endif
#elif defined PURPL_RELEASE
    Level = LogLevelInfo;
#endif
    LogSetLevel(Level);

    LogInfo("Common library initialized");
}

#ifdef PURPL_USE_MIMALLOC
static
VOID
MiMallocStatPrint(
    PCSTR Message,
    PVOID Argument
    )
{
    UNREFERENCED_PARAMETER(Argument);
    LogDebug("%.*s", strlen(Message) - 2, Message); // -2 for NUL and newline
}
#endif

VOID
CmnShutdown(
    VOID
    )
{
    PlatShutdown();

    LogInfo("Common library shut down");

#ifdef PURPL_USE_MIMALLOC
    // Some memory will still be in use because of the THREAD for the main thread,
    // which is managed by the launcher, and therefore can't be freed before this
    // function
    mi_stats_print_out(
        MiMallocStatPrint,
        NULL
        );
#endif
}

// Only on non-Windows, non-mimalloc
#ifndef CmnAlignedRealloc
PVOID
CmnAlignedRealloc(
    PVOID Block,
    SIZE_T Alignment,
    SIZE_T Size
    )
{
    PVOID NewBlock = CmnAlignedAlloc(
        Alignment,
        Size
        );
    memmove(
        NewBlock,
        Block,
        PURPL_MIN(Size, PURPL_MSIZE(Block))
        );
    CmnAlignedFree(Block);
    return NewBlock;
}
#endif

PCSTR
CmnFormatTempStringVarArgs(
    _In_ _Printf_format_string_ PCSTR Format,
    _In_ va_list Arguments
    )
/*++

Routine Description:

    This routine formats a printf format string into a static
    buffer for temporary usage.

Arguments:

    Format - The format string. You're making a bad decision if this
    parameter is not a string literal.

    Arguments - Arguments to the format string.

Return Value:

    A pointer to a static buffer with the formatted string.

--*/
{
    static CHAR Buffer[1024];
    va_list _Arguments;

    memset(
        Buffer,
        0,
        PURPL_ARRAYSIZE(Buffer)
        );

    va_copy(
        _Arguments,
        Arguments
        );
    vsnprintf(
        Buffer,
        PURPL_ARRAYSIZE(Buffer),
        Format,
        _Arguments
        );
    va_end(_Arguments);

    return Buffer;
}

PCSTR
CmnFormatTempString(
    _In_ _Printf_format_string_ PCSTR Format,
    ...
    )
/*++

Routine Description:

    This routine formats a printf format string into a static
    buffer for temporary usage.

Arguments:

    Format - The format string. You're making a bad decision if this
    parameter is not a string literal.

    ... - Arguments to the format string.

Return Value:

    A pointer to a static buffer with the formatted string.

--*/
{
    va_list Arguments;
    PCSTR Formatted;

    va_start(
        Arguments,
        Format
        );
    Formatted = CmnFormatTempStringVarArgs(
        Format,
        Arguments
        );
    va_end(Arguments);

    return Formatted;
}

PCHAR
CmnFormatStringVarArgs(
    _In_ _Printf_format_string_ PCSTR Format,
    _In_ va_list Arguments
    )
/*++

Routine Description:

    This routine formats a printf format string into a dynamically allocated buffer.

Arguments:

    Format - The format string. You're making a bad decision if this
    parameter is not a string literal.

    Arguments - Arguments to the format string.

Return Value:

    A pointer to a buffer with the formatted string and a NUL terminator (size is strlen(Buffer) + 1).

--*/
{
    PCHAR Buffer;
    INT Size;
    va_list CopiedArguments;

    va_copy(
        CopiedArguments,
        Arguments
        );
    Size = vsnprintf(
        NULL,
        0,
        Format,
        CopiedArguments
        ) + 1;
    Buffer = CmnAlloc(
        Size,
        1
        );
    va_copy(
        CopiedArguments,
        Arguments
        );
    vsnprintf(
        Buffer,
        Size,
        Format,
        CopiedArguments
        );
    va_end(CopiedArguments);

    return Buffer;
}

PCHAR
CmnFormatString(
    _In_ _Printf_format_string_ PCSTR Format,
    ...
    )
/*++

Routine Description:

    This routine formats a printf format string into a dynamically allocated buffer.

Arguments:

    Format - The format string. You're making a bad decision if this
    parameter is not a string literal.

    ... - Arguments to the format string.

Return Value:

    A pointer to a buffer with the formatted string.

--*/
{
    va_list Arguments;
    PCHAR Formatted;

    va_start(
        Arguments,
        Format
        );
    Formatted = CmnFormatStringVarArgs(
        Format,
        Arguments
        );
    va_end(Arguments);

    return Formatted;
}

PCSTR
CmnFormatSize(
    _In_ DOUBLE Size
)
/*++

Routine Description:

    This routine converts a size into a human readable string, using the
    most appropriate unit.

Arguments:

    Size - The size to convert.

Return Value:

    The address of a static buffer containing the string.

--*/
{
    static CHAR Buffer[64]; // Not gonna be bigger than this
    DOUBLE Value;
    UINT8 Prefix;

    // TODO: if this is ever somehow a bottleneck, remove everything past tibibytes and make the buffer 16 bytes.

    CONST PCSTR Units[] = {
        "B",
        "kiB",
        "MiB",
        "GiB",
        "TiB",
        "PiB (damn)",
        "EiB (are you sure?)",
        "ZiB (who are you?)",
        "YiB (what are you doing?)",
        "RiB (why are you doing this?)",
        "QiB (HOW ARE YOU DOING THIS?)",
        "?B (what did you do?)"
    };

    Value = Size;
    Prefix = 0;
    while ( Value > 1024 )
    {
        Value /= 1024;
        Prefix++;
    }

    snprintf(
        Buffer,
        PURPL_ARRAYSIZE(Buffer),
        "%.02lf %s",
        Value,
        Units[PURPL_MIN(Prefix, PURPL_ARRAYSIZE(Units) - 1)]
        );

    return Buffer;
}

_Noreturn
VOID
CmnError(
    _In_ _Printf_format_string_ PCSTR Message,
    ...
    )
/*++

Routine Description:

    This routine displays an error message and nukes the program.

Arguments:

    Message - The error message.

    ... - The arguments to the error message.

Return Value:

    Does not return.

--*/
{
    va_list Arguments;
    PCSTR FormattedMessage;
    PCSTR Formatted;
    PCSTR BackTrace;

    CmnShutdown();

    va_start(
        Arguments,
        Message
        );
    FormattedMessage = CmnFormatStringVarArgs(
        Message,
        Arguments
        );
    va_end(Arguments);
    BackTrace = PlatCaptureStackBackTrace(
        1, // Don't include CmnError in the trace
#ifdef PURPL_VERBOSE
        0 // Everything
#elif defined PURPL_DEBUG
        5 // A bit more context
#else
        3 // Enough
#endif
        );
    Formatted = CmnFormatString(
        "Fatal error: %s\nStack trace:\n%s",
        FormattedMessage,
        BackTrace
        );
    LogFatal("%s", Formatted);
    PlatError(Formatted);

    // Just in case PlatformError doesn't kill the process
    while (1)
    {
        abort();
    }
}
