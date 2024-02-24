/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    platform.c

Abstract:

    This file implements miscellaneous Windows abstractions.

--*/

#include "purpl/purpl.h"

#ifdef PURPL_GDK
#include <XGameRuntimeInit.h>
#endif

BEGIN_EXTERN_C

#include "common/alloc.h"
#include "common/common.h"

#include "platform/platform.h"

DWORD InitialConsoleInputMode;
DWORD InitialConsoleOutputMode;
DWORD InitialConsoleErrorMode;

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

        LogDebug("Attaching console");

        freopen_s(&File, "CONIN$", "r", stdin);
        freopen_s(&File, "CONOUT$", "w", stdout);
        freopen_s(&File, "CONOUT$", "w", stderr);

#if _WIN32_WINNT > 0x502
        GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &InitialConsoleInputMode);
        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), InitialConsoleInputMode | ENABLE_VIRTUAL_TERMINAL_INPUT);

        GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &InitialConsoleOutputMode);
        SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), InitialConsoleOutputMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

        GetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), &InitialConsoleErrorMode);
        SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), InitialConsoleErrorMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif

        // In case running under cmd.exe, to not be on same line as prompt.
        printf("\n");
    }

    LogInfo("Performing Windows specific initialization");

#ifdef PURPL_GDK
    LogInfo("Initializing Xbox Gaming Runtime Services");
    Result = XGameRuntimeInitialize();
    if (FAILED(Result))
    {
    //    CmnError("Failed to initialize Xbox Gaming Runtime Services: HRESULT 0x%08X", Result);
    }
#endif

    LogInfo("Windows initialization completed");
}

VOID PlatShutdown(VOID)
{
    LogInfo("Deinitializing Windows resources");

#ifndef PURPL_GDKX
    LogDebug("Unloading debug information");
    SymCleanup(GetCurrentProcess());
#endif

    LogDebug("Shutting down Xbox Gaming Runtime Services");
    XGameRuntimeUninitialize();

    LogInfo("Windows deinitialization succeeded");
}

PCSTR PlatCaptureStackBackTrace(_In_ UINT32 FramesToSkip, _In_ UINT32 MaxFrames)
{
    static CHAR Buffer[UINT16_MAX];
    PVOID BackTrace[32] = {0};
    DWORD64 Displacement = 0;
#ifndef PURPL_GDKX
    PSYMBOL_INFOW Symbol;
    BYTE SymbolBuffer[sizeof(SYMBOL_INFOW) + 31 * sizeof(WCHAR)];
    IMAGEHLP_MODULEW64 ModuleInfo = {0};
#endif
    UINT64 Offset;
    INT32 Written;
    PVOID ModuleAddress = nullptr;
    // PVOID ModuleHandle = nullptr;
    UINT64 i;
    DWORD Error;
    UINT64 Count;

    if (MaxFrames > 0)
    {
        Count = MaxFrames + FramesToSkip;
    }
    else
    {
        Count = PURPL_ARRAYSIZE(BackTrace);
    }

    RtlCaptureStackBackTrace(FramesToSkip + 1, PURPL_ARRAYSIZE(BackTrace), BackTrace, nullptr);

    Offset = 0;
    for (i = 0; i < Count && BackTrace[i]; i++)
    {
#ifndef PURPL_GDKX
        memset(SymbolBuffer, 0, sizeof(SymbolBuffer));
        Symbol = (PSYMBOL_INFOW)SymbolBuffer;
        Symbol->SizeOfStruct = sizeof(SYMBOL_INFOW);
        Symbol->MaxNameLen = 32;
        Displacement = 0;
        if (!SymFromAddrW(GetCurrentProcess(), (UINT64)BackTrace[i], &Displacement, Symbol))
        {
            Error = GetLastError();
            LogDebug("Failed to get symbol for address 0x%llX: %d (0x%X)", BackTrace[i], Error, Error);
        }

        ModuleAddress = (PVOID)SymGetModuleBase64(GetCurrentProcess(), (UINT64)BackTrace[i]);
        if (!ModuleAddress)
        {
            Error = GetLastError();
            LogDebug("Failed to get module base for address 0x%llX: %d (0x%X)", BackTrace[i], Error, Error);
        }

        memset(&ModuleInfo, 0, sizeof(IMAGEHLP_MODULEW64));
        ModuleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULEW64);
        if (!SymGetModuleInfoW64(GetCurrentProcess(), (UINT64)ModuleAddress, &ModuleInfo))
        {
            Error = GetLastError();
            LogDebug("Failed to get module for address 0x%llX: %d (0x%X)", BackTrace[i], Error, Error);
        }
#endif

        Written = snprintf(Buffer + Offset, PURPL_ARRAYSIZE(Buffer) - Offset, "\t%lld: %ls!%ls+0x%llX (0x%llX)\n", i,
#ifdef PURPL_GDKX
                           L"<unknown>", L"<unknown>",
#else
                           wcslen(ModuleInfo.ImageName) ? ModuleInfo.ImageName : L"<unknown>",
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

    // PCSTR Name = nullptr;

    BOOL IsWow64 = FALSE;
#ifdef _M_IX86
    IsWow64Process(INVALID_HANDLE_VALUE, &IsWow64);
#endif

    RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_QUERY_VALUE,
                  &CurrentVersionHandle);

    Size = sizeof(EditionId);
    RegQueryValueExA(CurrentVersionHandle, "EditionID", nullptr, nullptr, (LPBYTE)EditionId, &Size);

    Size = sizeof(ProductName);
    RegQueryValueExA(CurrentVersionHandle, "ProductName", nullptr, nullptr, (LPBYTE)ProductName, &Size);

    Size = sizeof(INT);
    if (RegQueryValueExA(CurrentVersionHandle, "CurrentMajorVersionNumber", nullptr, nullptr,
                         (LPBYTE)&CurrentMajorVersionNumber, &Size) == ERROR_SUCCESS)
    {
        Size = sizeof(INT);
        RegQueryValueExA(CurrentVersionHandle, "CurrentMinorVersionNumber", nullptr, nullptr,
                         (LPBYTE)&CurrentMinorVersionNumber, &Size);

        Size = sizeof(InstallationType);
        RegQueryValueExA(CurrentVersionHandle, "InstallationType", nullptr, nullptr, (LPBYTE)InstallationType, &Size);

        Size = sizeof(CurrentBuildNumber);
        RegQueryValueExA(CurrentVersionHandle, "CurrentBuildNumber", nullptr, nullptr, (LPBYTE)CurrentBuildNumber, &Size);

        Size = sizeof(INT);
        RegQueryValueExA(CurrentVersionHandle, "UBR", nullptr, nullptr, (LPBYTE)&UBR, &Size);

        Size = sizeof(BuildLabEx);
        RegQueryValueExA(CurrentVersionHandle, "BuildLabEx", nullptr, nullptr, (LPBYTE)&BuildLabEx, &Size);

        Size = sizeof(DisplayVersion);
        RegQueryValueExA(CurrentVersionHandle, "DisplayVersion", nullptr, nullptr, (LPBYTE)DisplayVersion, &Size);

        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer),
#ifdef _DEBUG
                 "%s %s %u.%u.%s %s%s",
#else
                 "%s %s %lu.%lu.%s.%lu %s%s",
#endif
                 strcmp(EditionId, "SystemOS") == 0 ? "Xbox System Software" : "Windows",
                 (strncmp(InstallationType, "Client", PURPL_ARRAYSIZE(InstallationType)) == 0) ? "Desktop"
                                                                                               : InstallationType,
                 CurrentMajorVersionNumber, CurrentMinorVersionNumber,
#ifdef _DEBUG
                 BuildLabEx, strcmp(EditionId, "SystemOS") == 0 ? "" : EditionId, IsWow64 ? " (WoW64)" : ""
#else
                 CurrentBuildNumber, UBR, strcmp(EditionId, "SystemOS") == 0 ? "" : EditionId, IsWow64 ? " (WoW64)" : ""
#endif
        );
    }
    else
    {
        Size = sizeof(CSDVersion);
        RegQueryValueExA(CurrentVersionHandle, "CSDVersion", nullptr, nullptr, (LPBYTE)BuildLab, &Size);
        Size = sizeof(BuildLab);
        RegQueryValueExA(CurrentVersionHandle, "BuildLab", nullptr, nullptr, (LPBYTE)BuildLab, &Size);

        snprintf(Buffer, PURPL_ARRAYSIZE(Buffer), "Windows %s %s %s (build lab %s%s)", ProductName, EditionId,
                 CSDVersion, BuildLab, IsWow64 ? ", WoW64" : "");
    }

    return Buffer;
}

_Noreturn VOID PlatError(_In_ PCSTR Message)
{
    INT Option;

    Option = MessageBoxA(nullptr, Message, "Purpl Error", MB_ICONERROR | MB_ABORTRETRYIGNORE);
    switch (Option)
    {
    case IDRETRY:
        DebugBreak();
    case IDIGNORE:
    case IDABORT:
        ExitProcess(STATUS_FATAL_APP_EXIT);
        break;
    }
}

PVOID PlatGetReturnAddress(VOID)
{
    PVOID ReturnAddress;

    RtlCaptureStackBackTrace(2, 1, &ReturnAddress, nullptr);

    return ReturnAddress;
}

PCSTR PlatGetUserDataDirectory(VOID)
{
    static CHAR Directory[MAX_PATH + 1] = {0};
    PCSTR Path;

    if (!strlen(Directory))
    {
        SHGetFolderPathA(nullptr, CSIDL_APPDATA | CSIDL_FLAG_CREATE, nullptr, SHGFP_TYPE_CURRENT, Directory);
        Directory[PURPL_MIN(MAX_PATH, strlen(Directory))] = '/';

        Path = PlatFixPath(Directory);
        if (Path)
        {
            strncpy(Directory, Path, PURPL_MIN(strlen(Path) + 1, PURPL_ARRAYSIZE(Directory)));
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
    PCHAR p = nullptr;
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
            CreateDirectoryA(Path, nullptr);
            *p = '/';
        }
    }
    CreateDirectoryA(Path, nullptr);

    // should probably for real check return values
    return TRUE;
}

VOID PlatPrint(_In_ PCSTR Text)
{
    OutputDebugStringA(Text);
}

PCHAR PlatFixPath(_In_ PCSTR Path)
{
    PCHAR FixedPath;
    UINT i;

    if (!Path || !strlen(Path))
    {
        return nullptr;
    }

    FixedPath = (PCHAR)CmnAlloc(1, strlen(Path) + 1);
    if (!FixedPath)
    {
        return nullptr;
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

UINT64 PlatGetFileSize(_In_ PCSTR Path)
{
    HANDLE File;
    LARGE_INTEGER Size = {};
    OFSTRUCT OpenStruct = {};
    DWORD Error;

    OpenStruct.cBytes = sizeof(OFSTRUCT);
    File = (HANDLE)(UINT_PTR)OpenFile(Path, &OpenStruct, OF_READ);
    if (!File)
    {
        Error = OpenStruct.nErrCode;
        LogWarning("Failed to open file %s to get its size: error %d (0x%X)", Error, Error);
        return 0;
    }

    if (!GetFileSizeEx(File, &Size))
    {
        Error = GetLastError();
        LogWarning("Failed to get size of file %s: error %d (0x%X)", Error, Error);
        return 0;
    }

    CloseHandle(File);

    return (UINT64)Size.QuadPart;
}

END_EXTERN_C
