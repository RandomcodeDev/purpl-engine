/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    async.c

Abstract:

    This module implements the Windows async backend.

--*/

#include "common/common.h"

#include "platform/async.h"

_Thread_local PTHREAD AsCurrentThread;

static
VOID
ThreadEntry(
    _In_ PVOID Thread
    )
{
    AsCurrentThread = Thread;
    AsCurrentThread->ReturnValue =
        AsCurrentThread->ThreadStart(AsCurrentThread->UserData);
    ExitThread(AsCurrentThread->ReturnValue);
}

VOID
InitializeMainThread(
    _In_ PFN_THREAD_START StartAddress
    )
{
    AsCurrentThread = PURPL_ALLOC(
        1,
        sizeof(THREAD)
        );
    strncpy(
        AsCurrentThread->Name,
        "main",
        PURPL_ARRAYSIZE(AsCurrentThread->Name)
        );
    AsCurrentThread->ThreadStart = StartAddress;
}

PTHREAD
AsCreateThread(
    _In_opt_ PCSTR Name,
    _In_ SIZE_T StackSize,
    _In_ PFN_THREAD_START ThreadStart,
    _In_opt_ PVOID UserData
    )
{
    PTHREAD Thread;
    DWORD Error;

    LogInfo("Creating thread %s with %zu-byte stack, entry point 0x%llX, and userdata 0x%llX", Name, StackSize, ThreadStart, UserData);

    Thread = PURPL_ALLOC(
        1,
        sizeof(THREAD)
        );
    if ( !Thread )
    {
        LogError("Failed to allocate thread data: %s", strerror(errno));
        return NULL;
    }

    strncpy(
        Thread->Name,
        Name,
        PURPL_ARRAYSIZE(Thread->Name)
        );
    Thread->ThreadStart = ThreadStart;
    Thread->UserData = UserData;

    Thread->Handle = CreateThread(
        NULL,
        StackSize,
        ThreadEntry,
        Thread,
        CREATE_SUSPENDED,
        NULL
        );
    if ( !Thread->Handle )
    {
        Error = GetLastError();
        LogError("Failed to create thread: %d (0x%X)", Error, Error);
        PURPL_FREE(Thread);
        return NULL;
    }

    return Thread;
}

INT
AsJoinThread(
    _In_ PTHREAD Thread
    )
{
    INT ReturnValue;
    
    if ( Thread->Handle )
    {
        WaitForSingleObject(
            Thread->Handle,
            INFINITE
            );
        TerminateThread(
            Thread->Handle,
            Thread->ReturnValue
            );
    }

    ReturnValue = Thread->ReturnValue;

    PURPL_FREE(Thread);

    return ReturnValue;
}

VOID
AsDetachThread(
    _In_ PTHREAD Thread
    )
{
    ResumeThread(Thread->Handle);
}