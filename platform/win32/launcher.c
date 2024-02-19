/// @file launcher.c
///
/// @brief This file implements the Windows entry point.
///
/// @copyright (c) 2024 Randomcode Developers

#include "purpl/purpl.h"

#include "common/alloc.h"
#include "common/common.h"
#include "common/filesystem.h"

// hinting the nvidia driver to use the dedicated graphics card in an optimus
// configuration for more info, see:
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

// same thing for AMD GPUs using v13.35 or newer drivers
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

// argc and argv are already supplied in debug builds, but otherwise they have
// to be parsed. These store the arguments either way.

static INT ArgumentCount;
static PCHAR *Arguments;
static BOOLEAN ParsedArguments;

/// @brief This routine is a modified version of ReactOS's CommandLineToArgvW
///        that is adjusted to convert ASCII instead of Unicode command lines
///        and use CmnAlloc and CmnFree instead of LocalAlloc and LocalFree.
///
/// @param lpCmdline The command line to parse
/// @param numargs   This parameter receives the number of arguments parsed
///
/// @return An array of parsed arguments
LPSTR *WINAPI CommandLineToArgvA(_In_ LPCSTR lpCmdline, _Out_ PINT numargs)
{
    DWORD argc;
    LPSTR *argv;
    LPCSTR s;
    LPSTR d;
    LPSTR cmdline;
    int qcount, bcount;

    if (!numargs)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }

    if (*lpCmdline == 0)
    {
        /* Return the path to the executable */
        DWORD len, deslen = MAX_PATH, size;

        size = sizeof(LPSTR) * 2 + deslen * sizeof(CHAR);
        for (;;)
        {
            argv = CmnAlloc(size, 1);
            if (!argv)
                return NULL;
            len = GetModuleFileNameA(0, (LPSTR)(argv + 2), deslen);
            if (!len)
            {
                CmnFree(argv);
                return NULL;
            }
            if (len < deslen)
                break;
            deslen *= 2;
            size = sizeof(LPSTR) * 2 + deslen * sizeof(CHAR);
            CmnFree(argv);
        }
        argv[0] = (LPSTR)(argv + 2);
        argv[1] = NULL;
        *numargs = 1;

        return argv;
    }

    /* --- First count the arguments */
    argc = 1;
    s = lpCmdline;
    /* The first argument, the executable path, follows special rules */
    if (*s == '"')
    {
        /* The executable path ends at the next quote, no matter what */
        s++;
        while (*s)
            if (*s++ == '"')
                break;
    }
    else
    {
        /* The executable path ends at the next space, no matter what */
        while (*s && *s != ' ' && *s != '\t')
            s++;
    }
    /* skip to the first argument, if any */
    while (*s == ' ' || *s == '\t')
        s++;
    if (*s)
        argc++;

    /* Analyze the remaining arguments */
    qcount = bcount = 0;
    while (*s)
    {
        if ((*s == ' ' || *s == '\t') && qcount == 0)
        {
            /* skip to the next argument and count it if any */
            while (*s == ' ' || *s == '\t')
                s++;
            if (*s)
                argc++;
            bcount = 0;
        }
        else if (*s == '\\')
        {
            /* '\', count them */
            bcount++;
            s++;
        }
        else if (*s == '"')
        {
            /* '"' */
            if ((bcount & 1) == 0)
                qcount++; /* unescaped '"' */
            s++;
            bcount = 0;
            /* consecutive quotes, see comment in copying code below */
            while (*s == '"')
            {
                qcount++;
                s++;
            }
            qcount = qcount % 3;
            if (qcount == 2)
                qcount = 0;
        }
        else
        {
            /* a regular character */
            bcount = 0;
            s++;
        }
    }

    /* Allocate in a single lump, the string array, and the strings that go
     * with it. This way the caller can make a single LocalFree() call to free
     * both, as per MSDN.
     */
    argv = CmnAlloc((argc + 1) * sizeof(LPSTR) + (strlen(lpCmdline) + 1) * sizeof(CHAR), 1);
    if (!argv)
        return NULL;
    cmdline = (LPSTR)(argv + argc + 1);
    strcpy(cmdline, lpCmdline);

    /* --- Then split and copy the arguments */
    argv[0] = d = cmdline;
    argc = 1;
    /* The first argument, the executable path, follows special rules */
    if (*d == '"')
    {
        /* The executable path ends at the next quote, no matter what */
        s = d + 1;
        while (*s)
        {
            if (*s == '"')
            {
                s++;
                break;
            }
            *d++ = *s++;
        }
    }
    else
    {
        /* The executable path ends at the next space, no matter what */
        while (*d && *d != ' ' && *d != '\t')
            d++;
        s = d;
        if (*s)
            s++;
    }
    /* close the executable path */
    *d++ = 0;
    /* skip to the first argument and initialize it if any */
    while (*s == ' ' || *s == '\t')
        s++;
    if (!*s)
    {
        /* There are no parameters so we are all done */
        argv[argc] = NULL;
        *numargs = argc;
        return argv;
    }

    /* Split and copy the remaining arguments */
    argv[argc++] = d;
    qcount = bcount = 0;
    while (*s)
    {
        if ((*s == ' ' || *s == '\t') && qcount == 0)
        {
            /* close the argument */
            *d++ = 0;
            bcount = 0;

            /* skip to the next one and initialize it if any */
            do
            {
                s++;
            } while (*s == ' ' || *s == '\t');
            if (*s)
                argv[argc++] = d;
        }
        else if (*s == '\\')
        {
            *d++ = *s++;
            bcount++;
        }
        else if (*s == '"')
        {
            if ((bcount & 1) == 0)
            {
                /* Preceded by an even number of '\', this is half that
                 * number of '\', plus a quote which we erase.
                 */
                d -= bcount / 2;
                qcount++;
            }
            else
            {
                /* Preceded by an odd number of '\', this is half that
                 * number of '\' followed by a '"'
                 */
                d = d - bcount / 2 - 1;
                *d++ = '"';
            }
            s++;
            bcount = 0;
            /* Now count the number of consecutive quotes. Note that qcount
             * already takes into account the opening quote if any, as well as
             * the quote that lead us here.
             */
            while (*s == '"')
            {
                if (++qcount == 3)
                {
                    *d++ = '"';
                    qcount = 0;
                }
                s++;
            }
            if (qcount == 2)
                qcount = 0;
        }
        else
        {
            /* a regular character */
            *d++ = *s++;
            bcount = 0;
        }
    }
    *d = '\0';
    argv[argc] = NULL;
    *numargs = argc;

    return argv;
}

extern VOID InitializeMainThread(_In_ PFN_THREAD_START StartAddress);

extern DWORD InitialConsoleInputMode;
extern DWORD InitialConsoleOutputMode;
extern DWORD InitialConsoleErrorMode;

/// @brief This routine is the entry point for non-debug Windows builds.
///
/// @param Instance          Module handle.
/// @param PreviousInstance  Not used.
/// @param CommandLine       Command line.
/// @param Show              Window show state.
///
/// @return An appropriate status code.
INT WinMain(_In_ HINSTANCE Instance, _In_opt_ HINSTANCE PreviousInstance, _In_ PCHAR CommandLine, _In_ INT Show)
{
    INT Result;
    // Don't care about checking for a console parent process on Xbox, since
    // the console window won't be seen in any circumstance
#ifndef PURPL_GDKX
    DWORD Error;
    HANDLE Snapshot;
    PROCESSENTRY32 ProcessEntry = {0};
    UINT32 EngineProcessId;
    UINT32 ParentProcessId;
    HANDLE ParentProcess;
    CHAR ParentExePath[MAX_PATH] = {0};
    CHAR ParentExeDevicePath[MAX_PATH] = {0};
    CHAR DriveLetter[] = " :";
    CHAR DevicePath[MAX_PATH] = {0};
    CHAR ParentExeDosPath[MAX_PATH] = {0};
    PIMAGE_DOS_HEADER ParentDosHeader = {0};
    PIMAGE_NT_HEADERS ParentHeaders = {0};
    SIZE_T Size;
    DWORD Mode;
#endif

    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(PreviousInstance);
    UNREFERENCED_PARAMETER(Show);

    if (!Arguments)
    {
        ArgumentCount = 0;
        Arguments = CommandLineToArgvA(CommandLine, &ArgumentCount);
        ParsedArguments = TRUE;
    }

#ifndef PURPL_DEBUG
    InitializeMainThread(WinMain);
#endif

#if !defined PURPL_GDKX && (defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO)
    LogDebug("Attempting to load debug info");
    SymSetOptions(SYMOPT_DEBUG | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    if (!SymInitialize(GetCurrentProcess(), NULL, TRUE))
    {
        Error = GetLastError();
        LogError("Failed to initialize DbgHelp: %d (0x%X)", Error, Error);
    }
    if (!SymLoadModuleEx(GetCurrentProcess(), NULL, PURPL_EXECUTABLE_NAME ".exe", NULL, (UINT64)GetModuleHandleA(NULL),
                         0, NULL, 0))
    {
        Error = GetLastError();
        LogError("Failed to load symbols: %d (0x%X)", Error, Error);
    }
#endif

    Result = PurplMain(Arguments, ArgumentCount);

    // Check if the parent process is a console, and pause if it isn't. No
    // error checking because the program is done anyway.

#ifndef PURPL_GDKX
#if _WIN32_WINNT > 0x502
    SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), InitialConsoleInputMode);
    SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), InitialConsoleOutputMode);
    SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE), InitialConsoleErrorMode);
#endif

    Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
    ParentProcessId = 0;
    if (Process32First(Snapshot, &ProcessEntry))
    {
        EngineProcessId = GetCurrentProcessId();
        do
        {
            if (ProcessEntry.th32ProcessID == EngineProcessId)
            {
                ParentProcessId = ProcessEntry.th32ParentProcessID;
                break;
            }
        } while (Process32Next(Snapshot, &ProcessEntry));

        ParentProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, ParentProcessId);
        if (ParentProcess)
        {
            GetProcessImageFileNameA(ParentProcess, ParentExePath, PURPL_ARRAYSIZE(ParentExePath));
            CloseHandle(ParentProcess);

            if (strncmp(ParentExePath, "\\Device\\", 8) == 0)
            {
                for (CHAR c = 'A'; c <= 'Z'; c++)
                {
                    strncpy(ParentExeDevicePath, ParentExePath, strchr(ParentExePath + 8, '\\') - ParentExePath);
                    DriveLetter[0] = c;
                    QueryDosDeviceA(DriveLetter, DevicePath, PURPL_ARRAYSIZE(DevicePath));
                    if (strcmp(DevicePath, ParentExeDevicePath) == 0)
                    {
                        snprintf(ParentExeDosPath, PURPL_ARRAYSIZE(ParentExeDosPath), "%c:%s", c,
                                 ParentExePath + strlen(ParentExeDevicePath));
                        break;
                    }
                }
            }

            if (!strlen(ParentExeDosPath))
            {
                strncpy(ParentExeDosPath, ParentExePath, PURPL_ARRAYSIZE(ParentExePath));
            }

            ParentDosHeader = (PIMAGE_DOS_HEADER)FsReadFile(ParentExeDosPath, 0, sizeof(IMAGE_DOS_HEADER), &Size, 0);
            if (ParentDosHeader && ParentDosHeader->e_magic == IMAGE_DOS_SIGNATURE)
            {
                ParentHeaders = (PIMAGE_NT_HEADERS)FsReadFile(ParentExeDosPath, ParentDosHeader->e_lfanew,
                                                              sizeof(IMAGE_NT_HEADERS), &Size, 0);
                if (ParentHeaders && ParentHeaders->Signature == IMAGE_NT_SIGNATURE)
                {
                    // Check bitness, offset of Subsystem is different
                    if ((ParentHeaders->FileHeader.Machine & IMAGE_FILE_32BIT_MACHINE &&
                         ((PIMAGE_NT_HEADERS32)ParentHeaders)->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_WINDOWS_CUI) ||
                        !(ParentHeaders->FileHeader.Machine & IMAGE_FILE_32BIT_MACHINE) &&
                            ((PIMAGE_NT_HEADERS64)ParentHeaders)->OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_WINDOWS_CUI)
                    {
                        printf("Engine (PID %llu, parent PID %llu) returned %d.\n"
                               "Press any key to exit...",
                               (UINT64)EngineProcessId, (UINT64)ParentProcessId, Result);

                        // Disable line input so any key works and not just Enter
                        Mode = 0;
                        GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &Mode);
                        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), 0);
                        (VOID) getchar();
                        SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), Mode);
                    }
                }
            }
        }
    }
#endif

    if (ParsedArguments)
        CmnFree(Arguments);

    return Result;
}

#if defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO
/// @brief This routine is the entry point for debug Windows builds.
///        It calls WinMain with the appropriate parameters.
///
/// @param[in] argc The number of command line arguments.
/// @param[in] argv The command line arguments.
///
/// @return The return value of PurplMain
INT main(_In_ INT argc, _In_ PCHAR argv[])
{
    CHAR DummyCommandline;

    Arguments = argv;
    ArgumentCount = argc;
    ParsedArguments = FALSE;

    InitializeMainThread((PFN_THREAD_START)main);

    DummyCommandline = 0;
    return WinMain(GetModuleHandleA(NULL), NULL, &DummyCommandline, SW_SHOWNORMAL);
}
#endif
