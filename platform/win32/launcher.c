/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    launcher.c

Abstract:

    This module implements the Windows entry point.

--*/

#include "purpl/purpl.h"

#include "common/common.h"

// hinting the nvidia driver to use the dedicated graphics card in an optimus configuration
// for more info, see:
// http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;

// same thing for AMD GPUs using v13.35 or newer drivers
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

//
// argc and argv are already supplied in debug builds
//

static INT ArgumentCount;
static PCHAR* Arguments;
static BOOLEAN ParsedArguments;

LPSTR*
WINAPI
CommandLineToArgvA(
    _In_ LPCSTR lpCmdline,
    _Out_ PINT numargs
    )
/*++

Routine Description:

    This routine is a modified version of ReactOS's CommandLineToArgvW
    that is adjusted to convert ASCII instead of Unicode command lines
    and use calloc and free instead of LocalAlloc and LocalFree.

Arguments:

    lpCmdline - The command line to parse.

    numargs - This parameter receives number of arguments parsed.

Return Value:

    NULL - Failed to parse command line.

    Array of arguments - Success.

--*/
{
    DWORD argc;
    LPSTR* argv;
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
            if (!(argv = CmnAlloc(size, 1))) return NULL;
            len = GetModuleFileNameA(0, (LPSTR)(argv + 2), deslen);
            if (!len)
            {
                CmnFree(argv);
                return NULL;
            }
            if (len < deslen) break;
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
            do {
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

//LONG
//ExceptionHandler(
//    _In_ PEXCEPTION_POINTERS ExceptionInformation
//    )
///*++
//
//Routine Description:
//
//    Catch-all exception handler to display exceptions.
//
//Arguments:
//
//    Exceptioninformation - Information about the exception that occured.
//
//Return Value:
//
//    None.
//
//--*/
//{
//    ULONG_PTR Arguments[EXCEPTION_MAXIMUM_PARAMETERS];
//    ULONG Response;
//    ULONG UnicodeParameters;
//
//    Response = 0;
//    Arguments[0] = (ULONG_PTR)ExceptionInformation->ExceptionRecord->ExceptionAddress;
//    memcpy(
//        Arguments + 1,
//        ExceptionInformation->ExceptionRecord->ExceptionInformation,
//        sizeof(ExceptionInformation->ExceptionRecord->ExceptionInformation)
//        );
//    if ( ExceptionInformation->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION ||
//         ExceptionInformation->ExceptionRecord->ExceptionCode == EXCEPTION_IN_PAGE_ERROR )
//    {
//        switch (Arguments[1])
//        {
//        case 0:
//            Arguments[2] = (ULONG_PTR)&(UNICODE_STRING)RTL_CONSTANT_STRING(L"reading");
//            break;
//        case 1:
//            Arguments[2] = (ULONG_PTR)&(UNICODE_STRING)RTL_CONSTANT_STRING(L"writing");
//            break;
//        case 8:
//            Arguments[2] = (ULONG_PTR)&(UNICODE_STRING)RTL_CONSTANT_STRING(L"executing");
//            break;
//        }
//
//        UnicodeParameters = 1 << 2;
//    }
//    else
//    {
//        UnicodeParameters = 0;
//    }
//
//    NtRaiseHardError(
//        ExceptionInformation->ExceptionRecord->ExceptionCode,
//        ExceptionInformation->ExceptionRecord->NumberParameters,
//        UnicodeParameters,
//        Arguments,
//        OptionAbortRetryIgnore,
//        &Response
//        );
//
//    switch (Response)
//    {
//    case ResponseAbort:
//        break;
//    case ResponseIgnore:
//    case ResponseRetry:
//        DbgBreakPoint();
//        break;
//    }
//
//    NtTerminateProcess(
//        NtCurrentProcess(),
//        ExceptionInformation->ExceptionRecord->ExceptionCode
//        );
//
//    return 0;
//}

extern
VOID
InitializeMainThread(
    _In_ PFN_THREAD_START StartAddress
    );

INT
WinMain(
    _In_ HINSTANCE Instance,
    _In_opt_ HINSTANCE PreviousInstance,
    _In_ PCHAR CommandLine,
    _In_ INT Show
    )
/*++

Routine Description:

    This routine is the entry point for non-debug Windows builds.

Arguments:

    Instance - Module handle.

    PreviousInstance - Not used.

    CommandLine - Command line.

    Show - Window show state.

Return Value:

    An appropriate status code.

--*/
{
    INT Result;
    // Don't care about checking for a console parent process on Xbox, since it won't be seen
#ifndef PURPL_GDKX
    DWORD Error;
    HANDLE Snapshot;
    PROCESSENTRY32 ProcessEntry = {0};
    UINT32 EngineProcessId;
    UINT32 ParentProcessId;
    HANDLE ParentProcess;
    CHAR ParentExeName[MAX_PATH] = {0};
    HMODULE* ParentModules = NULL;
    DWORD Size;
    DWORD i;
    CHAR ModuleName[MAX_PATH] = {0};
    MODULEINFO ModuleInfo = {0};
    IMAGE_DOS_HEADER ParentDosHeader = {0};
    IMAGE_NT_HEADERS ParentHeaders = {0};
    DWORD Mode;
#endif

    /*AddVectoredExceptionHandler(
        FALSE,
        ExceptionHandler
        );*/

    if ( !Arguments )
    {
        ArgumentCount = 0;
        Arguments = CommandLineToArgvA(
            CommandLine,
            &ArgumentCount
            );
        ParsedArguments = TRUE;
    }

    UNREFERENCED_PARAMETER(Instance);
    UNREFERENCED_PARAMETER(PreviousInstance);
    UNREFERENCED_PARAMETER(Show);

    // Get a ton of memory so it doesn't have to be requested from the OS later
    free(malloc(1 * 1024 * 1024 * 1024));

#ifndef PURPL_DEBUG
    InitializeMainThread(WinMain);
#endif

#if !defined PURPL_GDKX && (defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO)
    LogDebug("Attempting to load debug info");
    SymSetOptions(SYMOPT_DEBUG | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS);
    if ( !SymInitialize(
             GetCurrentProcess(),
             NULL,
             TRUE
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

    Result = PurplMain(
        ArgumentCount,
        Arguments
        );

    // No error checking because the program is done anyway
    // Check if the parent process is a console, and pause if it isn't

#ifndef PURPL_GDKX
    Snapshot = CreateToolhelp32Snapshot(
        TH32CS_SNAPPROCESS,
        0
        );
    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);
    ParentProcessId = 0;
    if ( Process32First(
             Snapshot,
             &ProcessEntry
             ) )
    {
        EngineProcessId = GetCurrentProcessId();
        do {
            if ( ProcessEntry.th32ProcessID == EngineProcessId )
            {
                ParentProcessId = ProcessEntry.th32ParentProcessID;
                break;
            }
        } while ( Process32Next(
                      Snapshot,
                      &ProcessEntry
                      ) );

        ParentProcess = OpenProcess(
            PROCESS_VM_READ,
            FALSE,
            ParentProcessId
            );
        if ( ParentProcess )
        {
            // TODO: Figure out why this doesn't work properly
            GetProcessImageFileNameA(
                ParentProcess,
                ParentExeName,
                PURPL_ARRAYSIZE(ParentExeName)
                );

            Size = 0;
            EnumProcessModules(
                ParentProcess,
                ParentModules,
                0,
                &Size
                );
            ParentModules = CmnAlloc(
                1,
                Size
                );
            EnumProcessModules(
                ParentProcess,
                ParentModules,
                Size,
                &Size
                );

            for ( i = 0; i < Size / sizeof(HMODULE); i++ )
            {
                GetModuleFileNameA(
                    ParentModules[i],
                    ModuleName,
                    PURPL_ARRAYSIZE(ModuleName)
                    );
                if ( strncmp(
                         ParentExeName,
                         ModuleName,
                         PURPL_ARRAYSIZE(ModuleName)
                         ) == 0 )
                {
                    GetModuleInformation(
                        ParentProcess,
                        ParentModules[i],
                        &ModuleInfo,
                        sizeof(ModuleInfo)
                        );
                    break;
                }
            }

            ReadProcessMemory(
                ParentProcess,
                ModuleInfo.lpBaseOfDll,
                &ParentDosHeader,
                sizeof(IMAGE_DOS_HEADER),
                NULL
                );
            if ( ParentDosHeader.e_magic == IMAGE_DOS_SIGNATURE )
            {
                ReadProcessMemory(
                    ParentProcess,
                    (PVOID)((UINT_PTR)ModuleInfo.lpBaseOfDll + ParentDosHeader.e_lfanew),
                    &ParentHeaders,
                    sizeof(IMAGE_NT_HEADERS),
                    NULL
                    );

                if ( ParentHeaders.Signature == IMAGE_NT_SIGNATURE &&
                     ParentHeaders.OptionalHeader.Subsystem != IMAGE_SUBSYSTEM_WINDOWS_CUI )
                {
                    printf("Engine (PID %llu, parent PID %llu) returned %d. Press any key to exit...", (UINT64)EngineProcessId, (UINT64)ParentProcessId, Result);

                    // Disable line input so any key works and not just Enter
                    Mode = 0;
                    GetConsoleMode(
                        GetStdHandle(STD_INPUT_HANDLE),
                        &Mode
                        );
                    SetConsoleMode(
                        GetStdHandle(STD_INPUT_HANDLE),
                        0
                        );
                    (VOID)getchar();
                    SetConsoleMode(
                        GetStdHandle(STD_INPUT_HANDLE),
                        Mode
                        );
                }
            }
        }
    }
#endif

    if ( ParsedArguments )
        CmnFree(Arguments);

    return Result;
}

#if defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO
INT
main(
    _In_ INT argc,
    _In_ PCHAR argv[]
    )
/*++

Routine Description:

    This routine is the entry point for debug Windows builds.
    It calls WinMain with the appropriate parameters.

--*/
{
    Arguments = argv;
    ArgumentCount = argc;
    ParsedArguments = FALSE;

    InitializeMainThread((PFN_THREAD_START)main);

    return WinMain(
        GetModuleHandleA(NULL),
        NULL,
        "",
        SW_SHOWNORMAL
        );
}
#endif
