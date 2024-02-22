/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    entity.c

Abstract:

    This file implements functions related to the entity component system.

--*/

#include "flecs.h"

#include "entity.h"

static ecs_world_t *EngineEcsWorld;

static VOID EcsLog(_In_ INT Level, _In_ PCSTR File, _In_ INT Line, _In_ PCSTR Message)
{
    LOG_LEVEL RealLevel;

    switch (Level)
    {
    case 0:
        RealLevel = LogLevelTrace;
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
        RealLevel = LogLevelDebug;
        break;
    }

    RealLevel = LogLevelTrace;

    if (RealLevel == LogLevelFatal)
    {
        CmnError("%s", Message);
    }
    else
    {
        LogMessage(RealLevel, File, Line, FALSE, "%s", Message);
    }
}

VOID EcsInitialize(VOID)
{
    LogInfo("Initializing ECS");

    ecs_os_set_api_defaults();
    ecs_os_api_t OsApi = ecs_os_api;
    OsApi.log_ = EcsLog;
#if PURPL_USE_MIMALLOC
    // Technically parameters differ, but every architecture where size_t isn't 32-bit uses registers, which means it
    // should be OK (and it should be easy enough to figure out whether this is causing problems if not)
    OsApi.malloc_ = mi_malloc;
    OsApi.calloc_ = mi_calloc;
    OsApi.realloc_ = mi_realloc;
    OsApi.free_ = mi_free;
#endif
    ecs_os_set_api(&OsApi);

    LogTrace("Creating ECS world");
    EcsSetWorld(ecs_init());
}

VOID EcsBeginFrame(_In_ UINT64 Delta)
{
    ecs_frame_begin(EngineEcsWorld, (FLOAT)Delta / 1000);
}

VOID EcsEndFrame(VOID)
{
    ecs_frame_end(EngineEcsWorld);
}

VOID EcsShutdown(VOID)
{
    LogInfo("Shutting down ECS");
    ecs_fini(EngineEcsWorld);
}

ecs_entity_t EcsCreateEntity(_In_opt_ PCSTR Name)
{
    LogTrace("Creating entity with name %s", Name);
    return ecs_entity(EngineEcsWorld, {.name = Name});
}

extern VOID CameraImport(_In_ ecs_world_t *World);
#ifdef PURPL_DISCORD
extern VOID DiscordImport(_In_ ecs_world_t *World);
#endif
extern VOID MathImport(_In_ ecs_world_t *World);
extern VOID RenderImport(_In_ ecs_world_t *World);

VOID EcsSetWorld(_In_ ecs_world_t *World)
{
    LogInfo("Setting ECS world");

    if (EngineEcsWorld)
    {
        ecs_fini(EngineEcsWorld);
    }
    EngineEcsWorld = World;

#if (defined(PURPL_DEBUG) || defined(PURPL_RELWITHDEBINFO)) && !defined(PURPL_SWITCH)
    LogTrace("Initializing ECS REST");
    ECS_IMPORT(EngineEcsWorld, FlecsMonitor);
    ecs_singleton_set(EngineEcsWorld, EcsRest, {0});
#endif

    LogTrace("Importing components and systems");
    ECS_IMPORT(EngineEcsWorld, Camera);
#ifdef PURPL_DISCORD
    ECS_IMPORT(EngineEcsWorld, Discord);
#endif
    ECS_IMPORT(EngineEcsWorld, Math);
    ECS_IMPORT(EngineEcsWorld, Render);
}

ecs_world_t *EcsGetWorld(VOID)
{
    return EngineEcsWorld;
}
