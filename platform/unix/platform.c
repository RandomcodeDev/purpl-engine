/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    platform.c

Abstract:

    This file implements miscellaneous Unix-like abstractions.

--*/

#include "common/common.h"

#include "platform/platform.h"

#include "GLFW/glfw3.h"

extern BOOLEAN WindowClosed;
static VOID SignalHandler(_In_ INT Signal, _In_ siginfo_t *SignalInformation, _In_opt_ PVOID UserData)
{
    PCSTR BaseErrorType = "Unknown error";
    PCSTR ErrorType = "unknown error";
    BOOLEAN Fatal = FALSE;

    switch (Signal)
    {
    case SIGILL:
        Fatal = TRUE;
        BaseErrorType = "Illegal operation";
        switch (SignalInformation->si_code)
        {
        case ILL_ILLOPC:
            ErrorType = "illegal opcode";
            break;
        case ILL_ILLOPN:
            ErrorType = "illegal operand";
            break;
        case ILL_ILLADR:
            ErrorType = "illegal addressing mode";
            break;
        case ILL_ILLTRP:
            ErrorType = "illegal trap";
            break;
        case ILL_PRVOPC:
            ErrorType = "privileged opcode";
            break;
        case ILL_PRVREG:
            ErrorType = "privileged register";
            break;
        case ILL_COPROC:
            ErrorType = "coprocessor error";
            break;
        case ILL_BADSTK:
            ErrorType = "internal stack error";
            break;
        }
        break;
    case SIGSEGV:
        Fatal = TRUE;
        BaseErrorType = "Segmentation fault";
        switch (SignalInformation->si_code)
        {
        case SEGV_MAPERR:
            ErrorType = "address not mapped to object";
            break;
        case SEGV_ACCERR:
            ErrorType = "invalid permissions for mapped object";
            break;
#ifdef PURPL_LINUX
        case SEGV_BNDERR:
            ErrorType = "failed bounds checks";
            break;
        case SEGV_PKUERR:
            ErrorType = "access was denied by memory protection keys";
            break;
#endif
        }
        break;
    case SIGFPE:
        Fatal = TRUE;
        BaseErrorType = "Arithmetic error";
        switch (SignalInformation->si_code)
        {
        case FPE_INTDIV:
            ErrorType = "integer divide by zero";
            break;
        case FPE_INTOVF:
            ErrorType = "integer overflow";
            break;
        case FPE_FLTDIV:
            ErrorType = "floating-point divide by zero";
            break;
        case FPE_FLTOVF:
            ErrorType = "floating-point overflow";
            break;
        case FPE_FLTUND:
            ErrorType = "floating-point underflow";
            break;
        case FPE_FLTRES:
            ErrorType = "floating-point inexact result";
            break;
        case FPE_FLTINV:
            ErrorType = "floating-point invalid operation";
            break;
        case FPE_FLTSUB:
            ErrorType = "subscript out of range";
            break;
        }
        break;
    case SIGBUS:
        Fatal = TRUE;
        BaseErrorType = "Bus error";
        switch (SignalInformation->si_code)
        {
        case BUS_ADRALN:
            ErrorType = "invalid address alignment";
            break;
        case BUS_ADRERR:
            ErrorType = "nonexistent physical address";
            break;
        case BUS_OBJERR:
            ErrorType = "object-specific hardware error";
            break;
        case BUS_MCEERR_AR:
            ErrorType = "hardware memory error consumed on a machine check; action required";
            break;
        case BUS_MCEERR_AO:
            ErrorType = "hardware memory error detected in process but not consumed; action optional";
            break;
        }
        break;
    case SIGINT:
    case SIGQUIT:
    case SIGTERM:
        LogInfo("Received quit signal %d, errno %d", Signal, SignalInformation->si_errno);
        WindowClosed = TRUE;
        break;
    default:
        LogInfo("Received signal %d", Signal);
        break;
    }

    CmnError("%s at 0x%llX: error %d: %s (si_code %d)", BaseErrorType, (UINT64)SignalInformation->si_addr,
             SignalInformation->si_errno, ErrorType, SignalInformation->si_code);
}

VOID PlatInitialize(VOID)
{
    LogDebug("Registering signal handler");
    struct sigaction SignalAction = {0};
    SignalAction.sa_sigaction = SignalHandler;
    sigaction(SIGILL, &SignalAction, NULL);
    sigaction(SIGSEGV, &SignalAction, NULL);
    sigaction(SIGFPE, &SignalAction, NULL);
    sigaction(SIGBUS, &SignalAction, NULL);
    sigaction(SIGINT, &SignalAction, NULL);
    sigaction(SIGQUIT, &SignalAction, NULL);
    sigaction(SIGTERM, &SignalAction, NULL);
}

VOID PlatShutdown(VOID)
/*++

Routine Description:

    This routine cleans up Unix-specific resources.

Arguments:

    None.

Return Value:

    None.

--*/
{
}

PCSTR
PlatCaptureStackBackTrace(_In_ UINT32 FramesToSkip, _In_ UINT32 MaxFrames)
/*++

Routine Description:

    Gets a stack trace in a static buffer.

Arguments:

    FramesToSkip - The number of stack frames to skip.

    MaxFrames - The maximum number of frames to get.

Return Value:

    The address of a static buffer containing a string with
    the formatted stack trace.

--*/
{
    static CHAR Buffer[2048];
    PVOID Frames[32];
    PCHAR *Symbols;
    UINT64 Size;
    UINT64 Offset;
    UINT64 i;

    memset(Buffer, 0, PURPL_ARRAYSIZE(Buffer));

    Size = backtrace(Frames, PURPL_ARRAYSIZE(Frames));
    if (MaxFrames > 0 && Size > MaxFrames)
    {
        Size = MaxFrames + FramesToSkip;
    }
    Symbols = backtrace_symbols(Frames, Size);

    Offset = 0;
    for (i = FramesToSkip; i < Size; i++)
    {
        Offset += snprintf(Buffer + Offset, PURPL_ARRAYSIZE(Buffer) - Offset, "\t%zu: %s (0x%llx)\n", i, Symbols[i],
                           (UINT64)Frames[i]);
    }

    free(Symbols);

    return Buffer;
}

PCSTR
PlatGetDescription(VOID)
/*++

Routine Description:

    Retrieves a string with information about the system version.

Arguments:

    None.

Return Value:

    A static buffer containing the system description.

--*/
{
    static CHAR Buffer[128];
    PCHAR Name;
    PCHAR BuildId;
    PCHAR End;
    CHAR OsReleaseBuffer[1024];
    FILE *OsRelease;
    struct utsname UtsName = {0};

    uname(&UtsName);

    OsRelease = fopen("/etc/os-release", "r");
    if (OsRelease)
    {
        fread(OsReleaseBuffer, 1, 1024, OsRelease);
        fclose(OsRelease);
        Name = strstr(OsReleaseBuffer, "NAME=\"") + 6;
        if ((UINT_PTR)Name == 6)
            Name = "Unknown";
        End = strchr(Name, '"');
        if (End)
            *End = 0;
        BuildId = strstr(End + 1, "BUILD_ID=") + 9;
        if ((UINT_PTR)BuildId == 9)
            BuildId = "unknown";
        End = strchr(BuildId, '\n');
        if (End)
            *End = 0;
        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer), "%s %s, kernel %s %s %s", Name, BuildId, UtsName.sysname,
                 UtsName.release, UtsName.machine);
    }
    else
    {
        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer), "%s %s %s", UtsName.sysname, UtsName.release, UtsName.machine);
    }

    return Buffer;
}

_Noreturn VOID PlatError(_In_ PCSTR Message)
{
    execlp("zenity", "zenity", "--error", "--text", Message,
           "--no-markup", // Prevent any characters from being interpreted as
                          // formatting
           "--title=Purpl Error", NULL);
    execlp("notify-send", "notify-send", "Purpl Error", Message, NULL);

    while (TRUE)
    {
        abort();
    }
}

PVOID PlatGetReturnAddress(VOID)
/*++

Routine Description:

    Gets the return address.

Arguments:

    None.

Return Value:

    NULL - The return address could not be determined.

    non-NULL - The return address of the caller.

--*/
{
#ifdef PURPL_DEBUG
    return __builtin_return_address(1);
#else
    return NULL;
#endif
}

PCSTR PlatGetUserDataDirectory(VOID)
{
    static CHAR Directory[MAX_PATH + 1];

    if (!strlen(Directory))
    {
        if (getenv("XDG_USER_DATA_HOME") && strlen(getenv("XDG_USER_DATA_HOME")))
        {
            strncpy(Directory, getenv("XDG_USER_DATA_HOME"), MAX_PATH);
        }
        else if (getenv("HOME") && strlen(getenv("HOME")))
        {
            snprintf(Directory, PURPL_ARRAYSIZE(Directory), "%s/.local/share/", getenv("HOME"));
        }
        else
        {
            // good enough
            strncpy(Directory, "/tmp/", MAX_PATH);
        }
    }

    return Directory;
}

UINT64 PlatGetMilliseconds(VOID)
{
    struct timespec Time = {0};

    clock_gettime(CLOCK_MONOTONIC, &Time);

    return Time.tv_sec * 1000 + Time.tv_nsec / 1000000;
}

BOOLEAN PlatCreateDirectory(_In_ PCSTR Path)
{
    // https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix

    CHAR TempPath[256];
    PCHAR p = NULL;
    UINT64 Length;

    snprintf(TempPath, sizeof(TempPath), "%s", Path);
    Length = strlen(TempPath);
    if (TempPath[Length - 1] == '/')
    {
        TempPath[Length - 1] = 0;
    }
    for (p = TempPath + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = 0;
            mkdir(TempPath, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(TempPath, S_IRWXU);

    // should probably for real check return values
    return TRUE;
}

PCHAR PlatFixPath(_In_ PCSTR Path)
{
    return CmnFormatString("%s", Path);
}

UINT64 PlatGetFileSize(_In_ PCSTR Path)
{
    struct stat64 StatBuffer = {0};

    stat64(Path, &StatBuffer);
    return StatBuffer.st_size;
}
