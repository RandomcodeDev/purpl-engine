/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    platform.c

Abstract:

    This module implements miscellaneous Windows abstractions.

--*/

#include "common/common.h"

#include "platform/platform.h"

#ifdef PURPL_GDK
extern
HRESULT
XGameRuntimeInitialize(
    VOID
    );
#endif

VOID
PlatInitialize(
        VOID
        )
/*++

Routine Description:

    This routine attaches to a console if it exists, and initializes
    Xbox Live on GDK builds, among other things.

Arguments:

    None.

Return Value:

    None.

--*/
{
    BOOL HaveConsole;
    DWORD Error;
#ifdef PURPL_GDK
//    HRESULT Result;
#endif

    HaveConsole = AttachConsole(ATTACH_PARENT_PROCESS);
    if ( HaveConsole
#if defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO
         || (!HaveConsole && AllocConsole())
#endif
        )
    {
        FILE* File;
        DWORD Mode;

        LogDebug("Attaching console");

        freopen_s(
            &File,
            "CONIN$",
            "r",
            stdin
            );
        freopen_s(
            &File,
            "CONOUT$",
            "w",
            stdout
            );
        freopen_s(
            &File,
            "CONOUT$",
            "w",
            stderr
            );

#if _WIN32_WINNT > 0x502
        GetConsoleMode(
            GetStdHandle(STD_INPUT_HANDLE),
            &Mode
            );
        SetConsoleMode(
            GetStdHandle(STD_INPUT_HANDLE),
            Mode | ENABLE_VIRTUAL_TERMINAL_INPUT
            );

        GetConsoleMode(
            GetStdHandle(STD_OUTPUT_HANDLE),
            &Mode
            );
        SetConsoleMode(
            GetStdHandle(STD_OUTPUT_HANDLE),
            Mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
            );

        GetConsoleMode(
            GetStdHandle(STD_ERROR_HANDLE),
            &Mode
            );
        SetConsoleMode(
            GetStdHandle(STD_ERROR_HANDLE),
            Mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
            );
#endif

        // In case running under cmd.exe, to not be on same line as prompt.
        printf("\n");
    }

    LogInfo("Performing Windows specific initialization");

#if !defined PURPL_GDKX && (defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO)
    LogDebug("Attempting to load debug info");
    SymSetOptions(SYMOPT_DEBUG | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    if ( !SymInitialize(
             GetCurrentProcess(),
             NULL,
             FALSE
             ) )
    {
        Error = GetLastError();
        LogError("Failed to initialize DbgHelp: %d (0x%X)", Error, Error);
    }
    if ( !SymLoadModuleEx(
             GetCurrentProcess(),
             NULL,
             GAME_EXECUTABLE_NAME ".exe",
             NULL,
             (UINT64)GetModuleHandleA(NULL),
             0,
             NULL,
             0
             ) )
    {
        Error = GetLastError();
        LogError("Failed to load symbols: %d (0x%X)", Error, Error);
    }
#endif

#if 0 //def PURPL_GDK
    LogInfo("Initializing Xbox Gaming Runtime Services");
    Result = XGameRuntimeInitialize();
    if (!SUCCEEDED(Result))
    {
        CommonError("Failed to initialize Xbox Gaming Runtime Services: HRESULT 0x%08X", Result);
    }
#endif

    LogInfo("Windows initialization completed");
}

VOID
PlatShutdown(
        VOID
        )
/*++

Routine Description:

    This routine cleans up Windows-specific resources.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogInfo("Deinitializing Windows resources");

#ifndef PURPL_GDKX
    //LogDebug("Unloading debug information");
    //SymCleanup(GetCurrentProcess());
#endif

    LogInfo("Windows deinitialization succeeded");
}

PCSTR
PlatCaptureStackBackTrace(
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
    static CHAR Buffer[UINT16_MAX];
    PVOID BackTrace[32] = {0};
    DWORD64 Displacement = 0;
#ifndef PURPL_GDKX
    PSYMBOL_INFOW Symbol;
    BYTE SymbolBuffer[sizeof(SYMBOL_INFOW) + 31 * sizeof(WCHAR)];
#endif
    SIZE_T Offset;
    UINT64 Written;
    IMAGEHLP_MODULEW64 ModuleInfo = {0};
    PVOID ModuleAddress = NULL;
    //PVOID ModuleHandle = NULL;
    INT i;
    DWORD Error;

    RtlCaptureStackBackTrace(
        FramesToSkip + 1,
        PURPL_ARRAYSIZE(BackTrace),
        BackTrace,
        NULL
        );

    Offset = 0;
    for ( i = 0; i < PURPL_ARRAYSIZE(BackTrace) && BackTrace[i]; i++ )
    {
        // Seems unlikely that the Xbox has dbghelp stuff
#ifndef PURPL_GDKX
        memset(
            SymbolBuffer,
            0,
            sizeof(SymbolBuffer)
            );
        Symbol = (PSYMBOL_INFOW)SymbolBuffer;
        Symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
        Symbol->MaxNameLen = 32;
        Displacement = 0;
        if ( !SymFromAddrW(
                 GetCurrentProcess(),
                 (UINT64)BackTrace[i],
                 &Displacement,
                 Symbol
                 ) )
        {
            Error = GetLastError();
            LogDebug("Failed to get symbol for address 0x%llX: %d (0x%X)", BackTrace[i], Error, Error);
        }

        ModuleAddress = (PVOID)SymGetModuleBase64(
            GetCurrentProcess(),
            (UINT64)BackTrace[i]
            );
        if ( !ModuleAddress )
        {
            Error = GetLastError();
            LogDebug("Failed to get module base for address 0x%llX: %d (0x%X)", BackTrace[i], Error, Error);
        }

        memset(
            &ModuleInfo,
            0,
            sizeof(IMAGEHLP_MODULEW64)
            );
        ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
        if ( !SymGetModuleInfoW64(
                 GetCurrentProcess(),
                 (UINT64)ModuleAddress,
                 &ModuleInfo
                 ) )
        {
            Error = GetLastError();
            LogDebug("Failed to get module for address 0x%llX: %d (0x%X)", BackTrace[i], Error, Error);
        }
#endif

        Written = snprintf(
            Buffer + Offset,
            PURPL_ARRAYSIZE(Buffer) - Offset,
            "\t%d: %ls!%ls+0x%llX (0x%llX)\n",
            i,
            wcslen(ModuleInfo.ImageName) ? ModuleInfo.ImageName : L"<unknown>",
#ifdef PURPL_GDKX
            L"<unknown>",
#else
            Symbol->NameLen ? Symbol->Name : L"<unknown>",
#endif
            Displacement,
            (UINT64)BackTrace[i]
            );
        Offset += Written > 0 ? Written : 0;
    }

    return Buffer;
}

PCSTR
PlatGetDescription(
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
    static CHAR Buffer[64];

    Buffer[PURPL_ARRAYSIZE(Buffer) - 1] = 0;
    if ( strlen(Buffer) )
    {
        return Buffer;
    }

    // All versions
    HKEY CurrentVersionHandle;
    CHAR EditionId[32] = {0};
    CHAR ProductName[32] = {0};
    DWORD Size = 0;

    // Windows 10 and above
    CHAR InstallationType[32] = {0};
    DWORD CurrentMajorVersionNumber = 0;
    DWORD CurrentMinorVersionNumber = 0;
    CHAR CurrentBuildNumber[8] = {0};
    DWORD UBR = 0;
    CHAR DisplayVersion[8] = {0};
    CHAR BuildLabEx[64] = {0};

    // Windows 8 and below
    //CHAR CurrentVersion[4] = {0};
    CHAR CSDVersion[8] = {0};
    CHAR BuildLab[64] = {0};

    //PCSTR Name = NULL;

    BOOL IsWow64 = FALSE;
#ifdef _M_IX86
    IsWow64Process(INVALID_HANDLE_VALUE, &IsWow64);
#endif

    RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0,
        KEY_QUERY_VALUE,
        &CurrentVersionHandle
        );

    Size = sizeof(EditionId);
    RegQueryValueExA(
        CurrentVersionHandle,
        "EditionID",
        NULL,
        NULL,
        (LPBYTE)EditionId,
        &Size
        );

    Size = sizeof(ProductName);
    RegQueryValueExA(
        CurrentVersionHandle,
        "ProductName",
        NULL,
        NULL,
        (LPBYTE)ProductName,
        &Size
        );

    Size = sizeof(INT);
    if ( RegQueryValueExA(
             CurrentVersionHandle,
             "CurrentMajorVersionNumber",
             NULL,
             NULL,
             (LPBYTE)&CurrentMajorVersionNumber,
             &Size) == ERROR_SUCCESS )
    {
        Size = sizeof(INT);
        RegQueryValueExA(
            CurrentVersionHandle,
            "CurrentMinorVersionNumber",
            NULL,
            NULL,
            (LPBYTE)&CurrentMinorVersionNumber,
            &Size
            );

        Size = sizeof(InstallationType);
        RegQueryValueExA(
            CurrentVersionHandle,
            "InstallationType",
            NULL,
            NULL,
            (LPBYTE)InstallationType,
            &Size
            );

        Size = sizeof(CurrentBuildNumber);
        RegQueryValueExA(
            CurrentVersionHandle,
            "CurrentBuildNumber",
            NULL,
            NULL,
            (LPBYTE)CurrentBuildNumber,
            &Size
            );

        Size = sizeof(INT);
        RegQueryValueExA(
            CurrentVersionHandle,
            "UBR",
            NULL,
            NULL,
            (LPBYTE)&UBR,
            &Size
            );

        Size = sizeof(BuildLabEx);
        RegQueryValueExA(CurrentVersionHandle, "BuildLabEx", NULL, NULL,
            (LPBYTE)&BuildLabEx, &Size);

        Size = sizeof(DisplayVersion);
        RegQueryValueExA(CurrentVersionHandle, "DisplayVersion", NULL,
            NULL, (LPBYTE)DisplayVersion, &Size);

        snprintf(
            Buffer,
            PURPL_ARRAYSIZE(Buffer),
#ifdef _DEBUG
            "%s %s %u.%u.%s %s%s",
#else
            "%s %s %lu.%lu.%s.%lu %s%s",
#endif
            strcmp(
                EditionId,
                "SystemOS"
                ) == 0 ? "Xbox System Software" : "Windows",
            ( strncmp(
                  InstallationType,
                  "Client",
                  PURPL_ARRAYSIZE(InstallationType)
                  ) == 0 ) ? "Desktop" : InstallationType,
            CurrentMajorVersionNumber,
            CurrentMinorVersionNumber,
#ifdef _DEBUG
            BuildLabEx,
            strcmp(
                EditionId,
                "SystemOS"
                ) == 0 ? "" : EditionId,
            IsWow64 ? " (WoW64)" : ""
#else
            CurrentBuildNumber,
            UBR,
            strcmp(
                EditionId,
                "SystemOS"
                ) == 0 ? "" : EditionId,
            IsWow64 ? " (WoW64)" : ""
#endif
            );
    }
    else
    {
        Size = sizeof(CSDVersion);
        RegQueryValueExA(CurrentVersionHandle, "CSDVersion", NULL, NULL,
            (LPBYTE)BuildLab, &Size);
        Size = sizeof(BuildLab);
        RegQueryValueExA(CurrentVersionHandle, "BuildLab", NULL, NULL,
            (LPBYTE)BuildLab, &Size);

        snprintf(
            Buffer,
            PURPL_ARRAYSIZE(Buffer),
            "Windows %s %s %s (build lab %s%s)",
            ProductName,
            EditionId,
            CSDVersion,
            BuildLab,
            IsWow64 ? ", WoW64" : ""
            );
    }

    return Buffer;
}

_Noreturn
VOID
PlatError(
    _In_ PCSTR Message
    )
/*++

Routine Description:

    Displays an error message box that optionally triggers a breakpoint.

Arguments:

    Message - The error message.

Return Value:

    None.

--*/
{
    INT Option;

    Option = MessageBoxA(
        NULL,
        Message,
        "Purpl Error",
        MB_ICONERROR | MB_ABORTRETRYIGNORE
        );
    switch ( Option )
    {
    case IDRETRY:
        DebugBreak();
    case IDIGNORE:
    case IDABORT:
        ExitProcess(1);
        break;
    }
}

PVOID
PlatGetReturnAddress(
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
    PVOID ReturnAddress;

    RtlCaptureStackBackTrace(
        2,
        1,
        &ReturnAddress,
        NULL
        );

    return ReturnAddress;
}

PCSTR
PlatGetUserDataDirectory(
    VOID
    )
/*++

Routine Description:

    Gets the path of the directory for user data to be stored in.

Arguments:

    None.

Return Value:

    The user data directory in a static buffer.

--*/
{
    static CHAR Directory[MAX_PATH + 1] = {0};
    PCSTR Path;

    if ( !strlen(Directory) )
    {
        SHGetFolderPathA(
            NULL,
            CSIDL_APPDATA | CSIDL_FLAG_CREATE,
            NULL,
            SHGFP_TYPE_CURRENT,
            Directory
            );
        Directory[PURPL_MIN(MAX_PATH, strlen(Directory))] = '/';

        Path = PlatFixPath(Directory);
        if ( Path )
        {
            strncpy(
                Directory,
                Path,
                PURPL_MIN(strlen(Path) + 1, PURPL_ARRAYSIZE(Directory))
                );
            PURPL_FREE((PCHAR)Path);
        }
    }

    return Directory;
}

UINT64
PlatGetMilliseconds(
    VOID
    )
/*++

Routine Description:

    Gets the number of milliseconds passed since an arbitrary point in time.

Arguments:

    None.

Return Value:

    A number of milliseconds.

--*/
{
#if _WIN32_WINNT <= 0x502
    return GetTickCount();
#else
    return GetTickCount64();
#endif
}

BOOLEAN
PlatCreateDirectory(
    _In_ PCSTR Path
    )
/*++

Routine Description:

    Recursively creates a directory.

Arguments:

    Path - The path of the directory to create.

Return Value:

    Whether the directory could be created.

--*/
{
    // literally Unix version but replace mkdir with CreateDirectoryA

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
            CreateDirectoryA(
                Path,
                NULL
                );
            *p = '/';
        }
    }
    CreateDirectoryA(
        Path,
        NULL
        );

    // should probably for real check return values
    return TRUE;
}

VOID
PlatPrint(
    _In_ PCSTR Text
    )
/*++

Routine Description:

    Uses OutputDebugStringA to output text.

Arguments:

    Text - The string to output.

Return Value:

    None.

--*/
{
    OutputDebugStringA(Text);
}

PCHAR
PlatFixPath(
    _In_ PCSTR Path
    )
/*++

Routine Description:

    Cleans up a path.

Arguments:

    Path - The path to clean up.

Return Value:

    The cleaned path in a heap buffer.

--*/
{
    PCHAR FixedPath;
    UINT i;

    if ( !Path || !strlen(Path) )
    {
        return NULL;
    }

    FixedPath = PURPL_ALLOC(
        1,
        strlen(Path) + 1
        );
    if ( !FixedPath )
    {
        return NULL;
    }

    for ( i = 0; i < strlen(Path); i++ )
    {
        if ( Path[i] == '\\' )
        {
            FixedPath[i] = '/';
        }
        else
        {
            FixedPath[i] = Path[i];
        }
    }

    return FixedPath;
}
