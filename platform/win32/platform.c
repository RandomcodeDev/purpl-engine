/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    platform.c

Abstract:

    This module implements miscellaneous Windows abstractions.

--*/

#include "common/alloc.h"
#include "common/common.h"

#include "platform/platform.h"

#ifdef PURPL_GDK
extern HRESULT XGameRuntimeInitialize(VOID);
#endif

VOID PlatInitialize(VOID)
{
    BOOL HaveConsole;
#ifdef PURPL_GDK
    HRESULT Result;
#endif

    HaveConsole = AttachConsole(ATTACH_PARENT_PROCESS);
    if (HaveConsole
#if defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO
        || (!HaveConsole && AllocConsole())
#endif
    )
    {
        FILE *File;
        DWORD Mode;

        LogDebug("Attaching console");

        freopen_s(&File, "CONIN$", "r", stdin);
        freopen_s(&File, "CONOUT$", "w", stdout);
        freopen_s(&File, "CONOUT$", "w", stderr);

#if _WIN32_WINNT > 0x502
        GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &Mode);
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                       Mode | ENABLE_VIRTUAL_TERMINAL_INPUT);

        GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &Mode);
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
                       Mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

        GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &Mode);
        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE),
                       Mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

        // In case running under cmd.exe, to not be on same line as prompt.
        printf("\n");
    }

    LogInfo("Performing Windows specific initialization");

#ifdef PURPL_GDK
    LogInfo("Initializing Xbox Gaming Runtime Services");
    Result = XGameRuntimeInitialize();
    if (!SUCCEEDED(Result))
    {
        CmnError(
            "Failed to initialize Xbox Gaming Runtime Services: HRESULT 0x%08X",
            Result);
    }
#endif

    LogInfo("Windows initialization completed");
}

VOID PlatShutdown(VOID)
{
    LogInfo("Deinitializing Windows resources");

#ifndef PURPL_GDKX
    // LogDebug("Unloading debug information");
    // SymCleanup(GetCurrentProcess());
#endif

    LogInfo("Windows deinitialization succeeded");
}

PCSTR PlatCaptureStackBackTrace(_In_ SIZE_T FramesToSkip, _In_ SIZE_T MaxFrames)
{
    static CHAR Buffer[UINT16_MAX];
    PVOID BackTrace[32] = {0};
    DWORD64 Displacement = 0;
#ifndef PURPL_GDKX
    PSYMBOL_INFOW Symbol;
    BYTE SymbolBuffer[sizeof(SYMBOL_INFOW) + 31 * sizeof(WCHAR)];
    IMAGEHLP_MODULEW64 ModuleInfo = {0};
#endif
    SIZE_T Offset;
    INT32 Written;
    PVOID ModuleAddress = NULL;
    // PVOID ModuleHandle = NULL;
    INT i;
    DWORD Error;
    SIZE_T Count;

    if (MaxFrames > 0)
    {
        Count = MaxFrames + FramesToSkip;
    }
    else
    {
        Count = PURPL_ARRAYSIZE(BackTrace);
    }

    RtlCaptureStackBackTrace(FramesToSkip + 1, PURPL_ARRAYSIZE(BackTrace),
                             BackTrace, NULL);

    Offset = 0;
    for (i = 0; i < Count && BackTrace[i]; i++)
    {
        // To the best of my knowledge, this isn't supported on Xbox
#ifndef PURPL_GDKX
        memset(SymbolBuffer, 0, sizeof(SymbolBuffer));
        Symbol = (PSYMBOL_INFOW)SymbolBuffer;
        Symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
        Symbol->MaxNameLen = 32;
        Displacement = 0;
        if (!SymFromAddrW(GetCurrentProcess(), (UINT64)BackTrace[i],
                          &Displacement, Symbol))
        {
            Error = GetLastError();
            LogDebug("Failed to get symbol for address 0x%llX: %d (0x%X)",
                     BackTrace[i], Error, Error);
        }

        ModuleAddress = (PVOID)SymGetModuleBase64(GetCurrentProcess(),
                                                  (UINT64)BackTrace[i]);
        if (!ModuleAddress)
        {
            Error = GetLastError();
            LogDebug("Failed to get module base for address 0x%llX: %d (0x%X)",
                     BackTrace[i], Error, Error);
        }

        memset(&ModuleInfo, 0, sizeof(IMAGEHLP_MODULEW64));
        ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
        if (!SymGetModuleInfoW64(GetCurrentProcess(), (UINT64)ModuleAddress,
                                 &ModuleInfo))
        {
            Error = GetLastError();
            LogDebug("Failed to get module for address 0x%llX: %d (0x%X)",
                     BackTrace[i], Error, Error);
        }
#endif

        Written = snprintf(Buffer + Offset, PURPL_ARRAYSIZE(Buffer) - Offset,
                           "\t%d: %ls!%ls+0x%llX (0x%llX)\n", i,
#ifdef PURPL_GDKX
                           L"<unknown>", L"<unknown>",
#else
                           wcslen(ModuleInfo.ImageName) ? ModuleInfo.ImageName
                                                        : L"<unknown>",
                           Symbol->NameLen ? Symbol->Name : L"<unknown>",
#endif
                           Displacement, (UINT64)BackTrace[i]);
        Offset += Written > 0 ? Written : 0;
    }

    return Buffer;
}

PCSTR PlatGetDescription(VOID)
{
    static CHAR Buffer[64];

    Buffer[PURPL_ARRAYSIZE(Buffer) - 1] = 0;
    if (strlen(Buffer))
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
    // CHAR CurrentVersion[4] = {0};
    CHAR CSDVersion[8] = {0};
    CHAR BuildLab[64] = {0};

    // PCSTR Name = NULL;

    BOOL IsWow64 = FALSE;
#ifdef _M_IX86
    IsWow64Process(INVALID_HANDLE_VALUE, &IsWow64);
#endif

    RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                  "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0,
                  KEY_QUERY_VALUE, &CurrentVersionHandle);

    Size = sizeof(EditionId);
    RegQueryValueExA(CurrentVersionHandle, "EditionID", NULL, NULL,
                     (LPBYTE)EditionId, &Size);

    Size = sizeof(ProductName);
    RegQueryValueExA(CurrentVersionHandle, "ProductName", NULL, NULL,
                     (LPBYTE)ProductName, &Size);

    Size = sizeof(INT);
    if (RegQueryValueExA(CurrentVersionHandle, "CurrentMajorVersionNumber",
                         NULL, NULL, (LPBYTE)&CurrentMajorVersionNumber,
                         &Size) == ERROR_SUCCESS)
    {
        Size = sizeof(INT);
        RegQueryValueExA(CurrentVersionHandle, "CurrentMinorVersionNumber",
                         NULL, NULL, (LPBYTE)&CurrentMinorVersionNumber, &Size);

        Size = sizeof(InstallationType);
        RegQueryValueExA(CurrentVersionHandle, "InstallationType", NULL, NULL,
                         (LPBYTE)InstallationType, &Size);

        Size = sizeof(CurrentBuildNumber);
        RegQueryValueExA(CurrentVersionHandle, "CurrentBuildNumber", NULL, NULL,
                         (LPBYTE)CurrentBuildNumber, &Size);

        Size = sizeof(INT);
        RegQueryValueExA(CurrentVersionHandle, "UBR", NULL, NULL, (LPBYTE)&UBR,
                         &Size);

        Size = sizeof(BuildLabEx);
        RegQueryValueExA(CurrentVersionHandle, "BuildLabEx", NULL, NULL,
                         (LPBYTE)&BuildLabEx, &Size);

        Size = sizeof(DisplayVersion);
        RegQueryValueExA(CurrentVersionHandle, "DisplayVersion", NULL, NULL,
                         (LPBYTE)DisplayVersion, &Size);

        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer),
#ifdef _DEBUG
                 "%s %s %u.%u.%s %s%s",
#else
                 "%s %s %lu.%lu.%s.%lu %s%s",
#endif
                 strcmp(EditionId, "SystemOS") == 0 ? "Xbox System Software"
                                                    : "Windows",
                 (strncmp(InstallationType, "Client",
                          PURPL_ARRAYSIZE(InstallationType)) == 0)
                     ? "Desktop"
                     : InstallationType,
                 CurrentMajorVersionNumber, CurrentMinorVersionNumber,
#ifdef _DEBUG
                 BuildLabEx,
                 strcmp(EditionId, "SystemOS") == 0 ? "" : EditionId,
                 IsWow64 ? " (WoW64)" : ""
#else
                 CurrentBuildNumber, UBR,
                 strcmp(EditionId, "SystemOS") == 0 ? "" : EditionId,
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

        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer),
                 "Windows %s %s %s (build lab %s%s)", ProductName, EditionId,
                 CSDVersion, BuildLab, IsWow64 ? ", WoW64" : "");
    }

    return Buffer;
}

_Noreturn VOID PlatError(_In_ PCSTR Message)
{
    INT Option;

    Option = MessageBoxA(NULL, Message, "Purpl Error",
                         MB_ICONERROR | MB_ABORTRETRYIGNORE);
    switch (Option)
    {
    case IDRETRY:
        DebugBreak();
    case IDIGNORE:
    case IDABORT:
        ExitProcess(1);
        break;
    }
}

PVOID PlatGetReturnAddress(VOID)
{
    PVOID ReturnAddress;

    RtlCaptureStackBackTrace(2, 1, &ReturnAddress, NULL);

    return ReturnAddress;
}

PCSTR PlatGetUserDataDirectory(VOID)
{
    static CHAR Directory[MAX_PATH + 1] = {0};
    PCSTR Path;

    if (!strlen(Directory))
    {
        SHGetFolderPathA(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL,
                         SHGFP_TYPE_CURRENT, Directory);
        Directory[PURPL_MIN(MAX_PATH, strlen(Directory))] = '/';

        Path = PlatFixPath(Directory);
        if (Path)
        {
            strncpy(Directory, Path,
                    PURPL_MIN(strlen(Path) + 1, PURPL_ARRAYSIZE(Directory)));
            CmnFree(Path);
        }
    }

    return Directory;
}

UINT64 PlatGetMilliseconds(VOID)
{
#if _WIN32_WINNT <= 0x502
    return GetTickCount();
#else
    return GetTickCount64();
#endif
}

BOOLEAN PlatCreateDirectory(_In_ PCSTR Path)
{
    // literally Unix version but replace mkdir with CreateDirectoryA

    CHAR TempPath[256];
    PCHAR p = NULL;
    SIZE_T Length;

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
            CreateDirectoryA(Path, NULL);
            *p = '/';
        }
    }
    CreateDirectoryA(Path, NULL);

    // should probably for real check return values
    return TRUE;
}

VOID PlatPrint(_In_ PCSTR Text)
{
    OutputDebugStringA(Text);
}

PCHAR
PlatFixPath(_In_ PCSTR Path)
{
    PCHAR FixedPath;
    UINT i;

    if (!Path || !strlen(Path))
    {
        return NULL;
    }

    FixedPath = CmnAlloc(1, strlen(Path) + 1);
    if (!FixedPath)
    {
        return NULL;
    }

    for (i = 0; i < strlen(Path); i++)
    {
        if (Path[i] == '\\')
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
