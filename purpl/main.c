/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    main.c

Abstract:

    This module implements the cross platform entry point.

--*/

#include "purpl/purpl.h"

#include "common/common.h"

#include "engine/engine.h"

INT PurplMain(_In_ INT ArgumentCount, _In_ PCHAR *Arguments)
/*++

Routine Description:

    This routine is the entry point after the real entry point.
    It orchestrates everything else, including engine initialization,
    the main loop, and shutdown.

Arguments:

    ArgumentCount - The number of arguments.

    Arguments - List of arguments.

Return Value:

    An appropriate status code.

--*/
{
    UNREFERENCED_PARAMETER(ArgumentCount);
    UNREFERENCED_PARAMETER(Arguments);

    CmnInitialize();
    EngInitialize();

    EngMainLoop();

    EngShutdown();
    CmnShutdown();

    return 0;
}
