/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    async.h

Abstract:

    This module contains the async abstraction API.

--*/

#pragma once

#include "purpl/purpl.h"

//
// Thread start signature
//

typedef INT (*PFN_THREAD_START)(
    _In_opt_ PVOID UserData
    );

//
// A thread
//

typedef struct THREAD
{
    CHAR Name[32];
    PFN_THREAD_START ThreadStart;
    PVOID UserData;
    INT ReturnValue;
    PVOID Handle;
} THREAD, *PTHREAD;

//
// The current thread
//

extern _Thread_local PTHREAD AsCurrentThread;

//
// Create a thread
//

extern
PTHREAD
AsCreateThread(
    _In_opt_ PCSTR Name,
    _In_ SIZE_T StackSize,
    _In_ PFN_THREAD_START ThreadStart,
    _In_opt_ PVOID UserData
    );

//
// Wait for a thread to finish and get its return value
//

extern
INT
AsJoinThread(
    _In_ PTHREAD Thread
    );

//
// Allow a thread to run on its own, you have to join it to not leak the THREAD data
//

extern
VOID
AsDetachThread(
    _In_ PTHREAD Thread
    );

//
// A mutex
//

typedef PVOID PMUTEX;

//
// Create a mutex
//

extern
PMUTEX
AsCreateMutex(
    VOID 
    );

//
// Lock a mutex
//

extern
BOOLEAN
AsLockMutex(
    _In_ PMUTEX Mutex,
    _In_ BOOLEAN Wait
    );

//
// Unlock a mutex
//

extern
VOID
AsUnlockMutex(
    _In_ PMUTEX Mutex
    );

//
// A semaphore
//

typedef PVOID PSEMAPHORE;

//
// Create a semaphore
//

extern
PSEMAPHORE
AsCreateSemaphore(
    _In_ SIZE_T Max
    );

//
// Acquire a semaphore
//

extern
BOOLEAN
AsAcquireSemaphore(
    _In_ PSEMAPHORE Semaphore,
    _In_ BOOLEAN Wait
    );

//
// Release a semaphore
//

extern
BOOLEAN
AsReleaseSemaphore(
    _In_ PSEMAPHORE Semaphore,
    _In_ BOOLEAN Wait
    );
