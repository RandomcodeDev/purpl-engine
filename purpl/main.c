/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    main.c

Abstract:

    This module implements the cross platform entry point.

--*/

#include "purpl/purpl.h"

#include "common/common.h"

#include "engine/engine.h"

#include "platform/async.h"

INT
TestThread(
    _In_ PVOID UserData
    )
{
    LogInfo("0x%llX", (UINT64)AsCurrentThread->ThreadStart);
    return (INT)UserData;
}

INT
PurplMain(
    _In_ INT ArgumentCount,
    _In_ PCHAR* Arguments
    )
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

    PTHREAD Thread = AsCreateThread(
        "test",
        8192,
        TestThread,
        (PVOID)69420
        );
    LogInfo("0x%llX", (UINT64)AsCurrentThread->ThreadStart);
    AsDetachThread(Thread);

    EngMainLoop();

    EngShutdown();
    CmnShutdown();

    LogInfo("%d", AsJoinThread(Thread));

    return 0;
}
