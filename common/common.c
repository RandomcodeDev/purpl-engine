/// @file common.c
///
/// @brief This file implements assorted common functions.
///
/// @copyright (c) 2024 Randomcode Developers

#include "common.h"
#include "alloc.h"
#include "filesystem.h"

// TODO: implement mutexes
//static VOID LogLock(BOOLEAN Lock, PVOID Mutex)
//{
    // Lock ? AsLockMutex(Mutex, TRUE) : AsUnlockMutex(Mutex);
//}

VOID CmnInitialize(_In_opt_ PCHAR *Arguments, _In_opt_ UINT ArgumentCount)
{
    LOG_LEVEL Level;

    UNREFERENCED_PARAMETER(ArgumentCount);
    UNREFERENCED_PARAMETER(Arguments);

    PlatInitialize();

    CONST UINT8 HugePageCount = 2;
#if PURPL_USE_MIMALLOC
    LogInfo("Using mimalloc allocator");

    mi_option_set(mi_option_reserve_huge_os_pages, HugePageCount);
    mi_option_set(mi_option_show_errors, TRUE);
#else
    LogInfo("Using libc allocator");

    // Reserve 2 GiB of memory, just like mimalloc
    free(malloc(HugePageCount * 1024 * 1024 * 1024));
#endif

    // TODO: implement mutexes
    // LogSetLock(LogLock, AsCreateMutex());

#ifdef PURPL_DEBUG
#ifdef PURPL_VERBOSE
    Level = LogLevelTrace;
#if PURPL_USE_MIMALLOC
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

#if PURPL_USE_MIMALLOC
static VOID MiMallocStatPrint(PCSTR Message, PVOID Argument)
{
    UNREFERENCED_PARAMETER(Argument);
    // Don't want blanks between log messages, looks bad
    LogDebug("%.*s", strlen(Message) - (Message[strlen(Message) - 2] == '\n' ? 2 : 1), Message);
}
#endif

VOID CmnShutdown(VOID)
{
    PlatShutdown();

#if PURPL_USE_MIMALLOC
    // Some memory will still be in use because of the THREAD for the main
    // thread, which is managed by the launcher, and therefore can't be freed
    // before this function
    mi_stats_print_out(MiMallocStatPrint, NULL);
#endif

    LogInfo("Common library shut down");
}

PCSTR CmnFormatTempString(_In_ _Printf_format_string_ PCSTR Format, ...)
{
    va_list Arguments;
    PCSTR Formatted;

    va_start(Arguments, Format);
    Formatted = CmnFormatTempStringVarArgs(Format, Arguments);
    va_end(Arguments);

    return Formatted;
}

PCSTR CmnFormatTempStringVarArgs(_In_ _Printf_format_string_ PCSTR Format, _In_ va_list Arguments)
{
    static CHAR Buffer[1024];
    va_list _Arguments;

    memset(Buffer, 0, PURPL_ARRAYSIZE(Buffer));

    va_copy(_Arguments, Arguments);
    vsnprintf(Buffer, PURPL_ARRAYSIZE(Buffer), Format, _Arguments);
    va_end(_Arguments);

    return Buffer;
}

PCHAR CmnFormatStringVarArgs(_In_ _Printf_format_string_ PCSTR Format, _In_ va_list Arguments)
{
    PCHAR Buffer;
    INT Size;
    va_list CopiedArguments;

    va_copy(CopiedArguments, Arguments);
    Size = vsnprintf(NULL, 0, Format, CopiedArguments) + 1;
    Buffer = CmnAlloc(Size, 1);
    va_copy(CopiedArguments, Arguments);
    vsnprintf(Buffer, Size, Format, CopiedArguments);
    va_end(CopiedArguments);

    return Buffer;
}

PCHAR CmnFormatString(_In_ _Printf_format_string_ PCSTR Format, ...)
{
    va_list Arguments;
    PCHAR Formatted;

    va_start(Arguments, Format);
    Formatted = CmnFormatStringVarArgs(Format, Arguments);
    va_end(Arguments);

    return Formatted;
}

PCSTR CmnFormatSize(_In_ DOUBLE Size)
{
    static CHAR Buffer[64]; // Not gonna be bigger than this
    DOUBLE Value;
    UINT8 Prefix;

    CONST PCSTR Units[] = {"B", "kiB", "MiB", "GiB", "TiB", "PiB (damn)", "EiB (are you sure?)",
                           // NOTE: these don't all go in increments of 1024, but they're physically
                           // impossible and here as a joke anyway
                           "ZiB (who are you?)", "YiB (what are you doing?)", "RiB (why are you doing this?)",
                           "QiB (HOW ARE YOU DOING THIS?)", "?B (what did you do?)"};

    Value = Size;
    Prefix = 0;
    while (Value > 1024)
    {
        Value /= 1024;
        Prefix++;
    }

    // If close enough to 2 places of pi, use the character
    if (abs(Value - M_PI) < 1e-2)
    {
        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer), "π %s", Units[PURPL_MIN(Prefix, PURPL_ARRAYSIZE(Units) - 1)]);
    }
    else
    {
        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer), "%.02lf %s", Value,
                 Units[PURPL_MIN(Prefix, PURPL_ARRAYSIZE(Units) - 1)]);
    }

    return Buffer;
}

_Noreturn VOID CmnError(_In_ _Printf_format_string_ PCSTR Message, ...)
{
    va_list Arguments;
    PCSTR FormattedMessage;
    PCSTR Formatted;
    PCSTR BackTrace;

    CmnShutdown();

    va_start(Arguments, Message);
    FormattedMessage = CmnFormatStringVarArgs(Message, Arguments);
    va_end(Arguments);
    BackTrace = PlatCaptureStackBackTrace(1, // Don't include CmnError in the trace
#ifdef PURPL_VERBOSE
                                          0 // Everything
#elif defined PURPL_DEBUG
                                          5 // A bit more context
#else
                                          3 // Enough
#endif
    );
    Formatted = CmnFormatString("Fatal error: %s\nStack trace:\n%s", FormattedMessage, BackTrace);
    LogFatal("%s", Formatted);
    PlatError(Formatted);
}
