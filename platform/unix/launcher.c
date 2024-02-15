/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    launcher.c

Abstract:

    This file implements the entrypoint for Unix-like operating systems.

--*/

#include "purpl/purpl.h"

#include "common/common.h"

extern VOID InitializeMainThread(_In_ PFN_THREAD_START ThreadStart);

INT main(INT argc, PCHAR *argv)
/*++

Routine:

    This routine is the entrypoint for Unix-like systems.

Arguments:

    argc - The number of arguments.

    argv - Array of command line arguments.

Return Value:

    An appropriate errno code.

--*/
{
    INT Result;

    // Get a ton of memory so it doesn't have to be requested from the OS later
    free(malloc(1 * 1024 * 1024 * 1024));

    InitializeMainThread((PFN_THREAD_START)main);

    Result = PurplMain(argv, argc);

    return Result;
}
