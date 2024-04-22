#include "engine/engine.h"

static PVOID EcsMalloc(ecs_size_t Size)
{
    return CmnAlloc(1, Size);
}

static PVOID EcsRealloc(PVOID Block, ecs_size_t Size)
{
    return CmnRealloc(Block, Size);
}

static VOID EcsFree(PVOID Block)
{
    CmnFree(Block);
}

static PSTR EcsStrdup(PCSTR String)
{
    return CmnDuplicateString(String, 0);
}

static ecs_os_thread_t EcsThreadNew(ecs_os_thread_callback_t Function, PVOID UserData)
{
    return (ecs_os_thread_t)AsCreateThread(NULL, PURPL_DEFAULT_THREAD_STACK_SIZE, (PFN_THREAD_START)Function, UserData);
}

static PVOID EcsThreadJoin(ecs_os_thread_t Thread)
{
    return (PVOID)AsJoinThread((PAS_THREAD)Thread);
}

static ecs_os_thread_id_t EcsThreadSelf(ecs_os_thread_t Thread)
{
    // Threads don't have IDs
    return 0;
}

static ecs_os_mutex_t EcsMutexNew(VOID)
{
    return (ecs_os_mutex_t)AsCreateMutex();
}

static VOID EcsMutexFree(ecs_os_mutex_t Mutex)
{
    AsDestroyMutex((PAS_MUTEX)Mutex);
}

static VOID EcsMutexLock(ecs_os_mutex_t Mutex)
{
    AsLockMutex((PAS_MUTEX)Mutex, TRUE);
}

static VOID EcsMutexUnlock(ecs_os_mutex_t Mutex)
{
    AsUnlockMutex((PAS_MUTEX)Mutex);
}

static VOID EcsSleep(UINT32 Seconds, UINT32 Nanoseconds)
{
    PlatSleep(Seconds * 1000 + Nanoseconds / 1000000);
}

static UINT64 EcsNow(VOID)
{
    return PlatGetMilliseconds() / 1000;
}

static VOID EcsGetTime(ecs_time_t* Time)
{
    UINT64 Now = PlatGetMilliseconds();
    Time->sec = Now / 1000;
    Time->nanosec = (Now - Time->sec * 1000) * 1000000;
}

static VOID EcsLog(_In_ INT32 Level, _In_z_ PCSTR File, _In_ INT32 Line, _In_z_ PCSTR Message)
{
    LOG_LEVEL RealLevel;

    switch (Level)
    {
    case 0:
        RealLevel = LogLevelDebug;
        break;
    case -2:
        RealLevel = LogLevelWarning;
        break;
    case -3:
        RealLevel = LogLevelError;
        break;
    case -4:
        RealLevel = LogLevelFatal;
        break;
    default:
        RealLevel = LogLevelTrace;
        break;
    }

    if (RealLevel == LogLevelFatal)
    {
        CmnError("ECS error at %s:%d: %s", File, Line, Message);
    }
    else
    {
        LogMessage(RealLevel, File, Line, FALSE, "%s", Message);
    }
}

VOID ecs_set_os_api_impl(VOID)
{
    ecs_os_api.malloc_ = EcsMalloc;
    ecs_os_api.calloc_ = EcsMalloc;
    ecs_os_api.realloc_ = EcsRealloc;
    ecs_os_api.free_ = EcsFree;

    ecs_os_api.strdup_ = EcsStrdup;

    ecs_os_api.thread_new_ = EcsThreadNew;
    ecs_os_api.thread_join_ = EcsThreadJoin;
    ecs_os_api.thread_self_ = EcsThreadSelf;

    ecs_os_api.task_new_ = EcsThreadNew;
    ecs_os_api.task_join_ = EcsThreadJoin;

    ecs_os_api.mutex_new_ = EcsMutexNew;
    ecs_os_api.mutex_free_ = EcsMutexFree;
    ecs_os_api.mutex_lock_ = EcsMutexLock;
    ecs_os_api.mutex_unlock_ = EcsMutexUnlock;

    ecs_os_api.sleep_ = EcsSleep;
    ecs_os_api.get_time_ = EcsGetTime;
    ecs_os_api.now_ = EcsNow;

    ecs_os_api.log_ = EcsLog;
}

