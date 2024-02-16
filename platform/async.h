/// @file async.h
///
/// @brief This file contains the async abstraction API.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

/// @brief A pointer to a thread start function
typedef INT (*PFN_THREAD_START)(_In_opt_ PVOID UserData);

/// @brief Data about a thread
typedef struct THREAD
{
    CHAR Name[32];
    PFN_THREAD_START ThreadStart;
    PVOID UserData;
    INT ReturnValue;
    PVOID Handle;
} THREAD, *PTHREAD;

/// @brief The current thread (not fully valid for the main thread)
extern _Thread_local PTHREAD AsCurrentThread;

/// @brief Create a suspended thread
/// 
/// @param[in] Name The name of the thread
/// @param[in] StackSize The size of the thread's stack
/// @param[in] ThreadStart The start function for the thread
/// @param[in] UserData Data for the thread
/// 
/// @return The new thread
extern PTHREAD AsCreateThread(_In_opt_ PCSTR Name, _In_ UINT64 StackSize,
                              _In_ PFN_THREAD_START ThreadStart,
                              _In_opt_ PVOID UserData);

/// @brief Wait for a thread to finish, get its return value, and clean up the
/// THREAD structure
/// 
/// @param[in] Thread The thread to join
/// 
/// @return The thread's return value
extern INT AsJoinThread(_In_ PTHREAD Thread);

/// @brief Run a thread, use AsJoinThread to clean up its data at some point
/// 
/// @param[in] Thread The thread to detach
extern VOID AsDetachThread(_In_ PTHREAD Thread);

/// @brief A mutex
typedef PVOID PMUTEX;

/// @brief Create a mutex
/// 
/// @return A mutex
extern PMUTEX AsCreateMutex(VOID);

/// @brief Lock a mutex
/// 
/// @param[in] Mutex The mutex to lock
/// @param[in] Wait Whether to wait for it to be unlocked
/// 
/// @return Whether the mutex was locked
extern BOOLEAN AsLockMutex(_In_ PMUTEX Mutex, _In_ BOOLEAN Wait);

/// @brief Unlock a mutex
/// 
/// @param[in] Mutex The mutex to unlock
extern VOID AsUnlockMutex(_In_ PMUTEX Mutex);

/// @brief A semaphore
typedef PVOID PSEMAPHORE;

/// @brief Create a semaphore
/// 
/// @param[in] Max The maximum reference count for the semaphore
/// 
/// @return A semaphore
extern PSEMAPHORE AsCreateSemaphore(_In_ UINT64 Max);

/// @brief Increase the reference count of a semaphore
/// 
/// @param[in] Semaphore The semaphore to acquire
/// @param[in] Wait Whether to wait
/// 
/// @return Whether the semaphore was acquired
extern BOOLEAN AsAcquireSemaphore(_In_ PSEMAPHORE Semaphore, _In_ BOOLEAN Wait);

/// @brief Decrease the reference count of a semaphore
/// 
/// @param[in] Semaphore The semaphore to release
/// @param[in] Wait Whether to wait
/// 
/// @return Whether the semaphore was acquired
extern BOOLEAN AsReleaseSemaphore(_In_ PSEMAPHORE Semaphore, _In_ BOOLEAN Wait);
