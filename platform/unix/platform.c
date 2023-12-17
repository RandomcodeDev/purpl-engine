/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    platform.c

Abstract:

    This module implements miscellaneous Unix-like abstractions.

--*/

#include "common/common.h"

#include "platform/platform.h"

#include "GLFW/glfw3.h"

VOID
PlatformInitialize(
    VOID
    )
/*++

Routine Description:

    Performs platform-specific initialization.

Arguments:

    None.

Return Value:

    None.

--*/
{

}

VOID
PlatformShutdown(
    VOID
    )
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
PlatformCaptureStackBackTrace(
    _In_ INT FramesToSkip
    )
/*++

Routine Description:

    Gets a stack trace in a static buffer.

Arguments:

    FramesToSkip - The number of stack frames to skip.

Return Value:

    The address of a static buffer containing a string with
    the formatted stack trace.

--*/
{
    static CHAR Buffer[2048];
    PVOID Frames[32];
    PCHAR* Symbols;
    SIZE_T Size;
    SIZE_T Offset;
    SIZE_T i;

    memset(
        Buffer,
        0,
        PURPL_ARRAYSIZE(Buffer)
        );

    Size = backtrace(
        Frames,
        PURPL_ARRAYSIZE(Frames)
        );
    Symbols = backtrace_symbols(
        Frames,
        Size
        );

    Offset = 0;
    for ( i = 0; i < Size; i++ )
    {
        Offset += snprintf(
            Buffer + Offset,
            PURPL_ARRAYSIZE(Buffer) - Offset,
            "\t%zu: %s (0x%llx)\n",
            i,
            Symbols[i],
            (UINT64)Frames[i]
            );
    }

    free(Symbols);

    return Buffer;
}

PCSTR
PlatformGetDescription(
    VOID
    )
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
    FILE* OsRelease;
    struct utsname UtsName = {0};

    uname(&UtsName);

    OsRelease = fopen(
        "/etc/os-release",
        "r"
        );
    if ( OsRelease )
    {
        fread(
            OsReleaseBuffer,
            1,
            1024,
            OsRelease
            );
        fclose(OsRelease);
        Name = strstr(
            OsReleaseBuffer,
            "NAME=\""
            ) + 6;
        if ( (UINT_PTR)Name == 6 )
            Name = "Unknown";
        End = strchr(
            Name,
            '"'
            );
        if ( End )
            *End = 0;
        BuildId = strstr(
            End + 1,
            "BUILD_ID="
            ) + 9;
        if ( (UINT_PTR)BuildId == 9 )
            BuildId = "unknown";
        End = strchr(
            BuildId,
            '\n'
            );
        if ( End )
            *End = 0;
        snprintf(
            Buffer,
            PURPL_ARRAYSIZE(Buffer),
            "%s %s, kernel %s %s %s",
            Name,
            BuildId,
            UtsName.sysname,
            UtsName.release,
            UtsName.machine
            );
    }
    else
    {
        snprintf(
            Buffer,
            PURPL_ARRAYSIZE(Buffer),
            "%s %s %s",
            UtsName.sysname,
            UtsName.release,
            UtsName.machine
            );
    }

    return Buffer;
}

_Noreturn
VOID
PlatformError(
    _In_ PCSTR Message
    )
{
    execlp(
        "zenity",
        "zenity",
        "--error",
        "--text",
        Message,
        "--title=Purpl Error",
        NULL
        );
    execlp(
        "notify-send",
        "notify-send",
        "Purpl Error",
        Message,
        NULL
        );

    while ( TRUE )
    {
        abort();
    }
}

PVOID
PlatformGetReturnAddress(
    VOID
    )
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

PCSTR
PlatformGetUserDataDirectory(
    VOID
    )
{
    static CHAR Directory[MAX_PATH + 1];

    if ( !strlen(Directory) )
    {
        if ( getenv("XDG_USER_DATA_HOME") && strlen(getenv("XDG_USER_DATA_HOME")) )
        {
            strncpy(
                Directory,
                getenv("XDG_USER_DATA_HOME"),
                MAX_PATH
                );
        }
        else if ( getenv("HOME") && strlen(getenv("HOME")) )
        {
            snprintf(
                Directory,
                PURPL_ARRAYSIZE(Directory),
                "%s/.local/share",
                getenv("HOME")
                );
        }
        else
        {
            // good enough
            strncpy(
                Directory,
                "/tmp",
                MAX_PATH
                );
        }
    }

    return Directory;
}

UINT64
PlatformGetMilliseconds(
    VOID
    )
{
    struct timespec Time = {0};

    clock_gettime(
        CLOCK_MONOTONIC,
        &Time
        );

    return Time.tv_sec * 1000 + Time.tv_nsec / 1000000;
}

BOOLEAN
PlatformCreateDirectory(
    _In_ PCSTR Path
    )
{
    // https://stackoverflow.com/questions/2336242/recursive-mkdir-system-call-on-unix

    CHAR TempPath[256];
    PCHAR p = NULL;
    SIZE_T Length;

    snprintf(
        TempPath,
        sizeof(TempPath),
        "%s",
        Path
        );
    Length = strlen(TempPath);
    if ( TempPath[Length - 1] == '/' )
    {
        TempPath[Length - 1] = 0;
    }
    for ( p = TempPath + 1; *p; p++ )
    {
        if ( *p == '/' ) {
            *p = 0;
            mkdir(
                TempPath,
                S_IRWXU
                );
            *p = '/';
        }
    }
    mkdir(
        TempPath,
        S_IRWXU
        );

    // should probably for real check return values
    return TRUE;
}

VOID
PlatformPrint(
    _In_ PCSTR String
    )
{
}

PCHAR
PlatformFixPath(
    _In_ PCSTR Path
    )
{
    return CommonFormatString(
        "%s",
	Path
	);
}
