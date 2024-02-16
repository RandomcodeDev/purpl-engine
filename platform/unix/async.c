/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    async.c

Abstract:

    This file implements the Unix async backend.

--*/

#include "common/alloc.h"
#include "common/common.h"

#include "platform/async.h"

_Thread_local PTHREAD AsCurrentThread;

static PVOID ThreadEntry(_In_ PVOID Thread)
{
    AsCurrentThread = Thread;
    AsCurrentThread->ReturnValue =
        AsCurrentThread->ThreadStart(AsCurrentThread->UserData);
    return (PVOID)AsCurrentThread->ReturnValue;
}

VOID InitializeMainThread(_In_ PFN_THREAD_START StartAddress)
{
    AsCurrentThread = CmnAlloc(1, sizeof(THREAD));
    strncpy(AsCurrentThread->Name, "main",
            PURPL_ARRAYSIZE(AsCurrentThread->Name));
    AsCurrentThread->ThreadStart = StartAddress;
}

PTHREAD
AsCreateThread(_In_opt_ PCSTR Name, _In_ UINT64 StackSize,
               _In_ PFN_THREAD_START ThreadStart, _In_opt_ PVOID UserData)
{
    PTHREAD Thread;
    INT Error;
    pthread_attr_t Attributes;

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

    Error = pthread_attr_init(&Attributes);
    if (Error != 0)
    {
        LogError("Failed to initialize thread attributes: %s", strerror(Error));
        CmnFree(Thread);
        return NULL;
    }

    Error = pthread_attr_setstacksize(&Attributes, StackSize);
    if (Error != 0)
    {
        LogError("Failed to set thread stack size: %s", strerror(Error));
        CmnFree(Thread);
        return NULL;
    }

    Error = pthread_create((pthread_t *)&Thread->Handle, &Attributes,
                           ThreadEntry, Thread);
    if (Error != 0)
    {
        LogError("Failed to create thread: %s", strerror(Error));
        CmnFree(Thread);
        return NULL;
    }

    // For logging purposes, this doesn't matter. It's just for debuggers, so
    // the return value isn't checked.
    pthread_setname_np((pthread_t)Thread->Handle, Thread->Name);

    pthread_attr_destroy(&Attributes);

    return Thread;
}

INT AsJoinThread(_In_ PTHREAD Thread)
{
    PVOID ReturnValue;

    if (Thread->Handle)
    {
        ReturnValue = NULL;
        pthread_join((pthread_t)Thread->Handle, &ReturnValue);
    }

    ReturnValue = (PVOID)Thread->ReturnValue;

    CmnFree(Thread);

    return (INT)ReturnValue;
}

VOID AsDetachThread(_In_ PTHREAD Thread)
{
    pthread_detach((pthread_t)Thread->Handle);
}
