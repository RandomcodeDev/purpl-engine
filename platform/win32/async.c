/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    async.c

Abstract:

    This file implements the Windows async backend.

--*/

#include "common/alloc.h"
#include "common/common.h"

#include "platform/async.h"

_Thread_local PTHREAD AsCurrentThread;

static VOID ThreadEntry(_In_ PVOID Thread)
{
    AsCurrentThread = Thread;
    AsCurrentThread->ReturnValue = AsCurrentThread->ThreadStart(AsCurrentThread->UserData);
    ExitThread(AsCurrentThread->ReturnValue);
}

VOID InitializeMainThread(_In_ PFN_THREAD_START StartAddress)
{
    AsCurrentThread = CmnAlloc(1, sizeof(THREAD));
    strncpy(AsCurrentThread->Name, "main", PURPL_ARRAYSIZE(AsCurrentThread->Name));
    AsCurrentThread->ThreadStart = StartAddress;
}

PTHREAD
AsCreateThread(_In_opt_ PCSTR Name, _In_ SIZE_T StackSize, _In_ PFN_THREAD_START ThreadStart, _In_opt_ PVOID UserData)
{
    PTHREAD Thread;
    DWORD Error;

    LogInfo("Creating thread %s with %zu-byte stack, entry point 0x%llX, and "
            "userdata 0x%llX",
            Name, StackSize, ThreadStart, UserData);

    Thread = CmnAlloc(1, sizeof(THREAD));
    if (!Thread)
    {
        LogError("Failed to allocate thread data: %s", strerror(errno));
        return NULL;
    }

    strncpy(Thread->Name, Name, PURPL_ARRAYSIZE(Thread->Name));
    Thread->ThreadStart = ThreadStart;
    Thread->UserData = UserData;

    Thread->Handle = CreateThread(NULL, StackSize, (LPTHREAD_START_ROUTINE)ThreadEntry, Thread, CREATE_SUSPENDED, NULL);
    if (!Thread->Handle)
    {
        Error = GetLastError();
        LogError("Failed to create thread: %d (0x%X)", Error, Error);
        CmnFree(Thread);
        return NULL;
    }

    return Thread;
}

INT AsJoinThread(_In_ PTHREAD Thread)
{
    INT ReturnValue;

    if (Thread->Handle)
    {
        WaitForSingleObject(Thread->Handle, INFINITE);
        // Kill the thread in case it didn't exit
        TerminateThread(Thread->Handle, Thread->ReturnValue);
    }

    ReturnValue = Thread->ReturnValue;

    CmnFree(Thread);

    return ReturnValue;
}

VOID AsDetachThread(_In_ PTHREAD Thread)
{
    ResumeThread(Thread->Handle);
}
