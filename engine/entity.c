/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    entity.c

Abstract:

    This module implements functions related to the entity component system.

--*/

#include "flecs.h"
#include "rendersystem/rendersystem.h"


extern
VOID
CameraImport(
    _In_ ecs_world_t* World
    );

extern
VOID
MathImport(
    _In_ ecs_world_t* World
    );

extern
VOID
RenderImport(
    _In_ ecs_world_t* World
    );

//
// ECS world
//

static ecs_world_t* EngineEcsWorld;

static
VOID
EcsLog(
    _In_ INT Level,
    _In_ PCSTR File,
    _In_ INT Line,
    _In_ PCSTR Message
    )
{
    LOG_LEVEL RealLevel;

    switch ( Level )
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

    if ( RealLevel == LogLevelFatal )
    {
        CommonError("%s", Message);
    }
    else
    {
        LogMessage(
            RealLevel,
            File,
            Line,
            FALSE,
            "%s",
            Message
            );
    }
}

VOID
EngineEcsInitialize(
    VOID
    )
{
    LogInfo("Initializing ECS");

    LogTrace("Putting log callback in flecs OS API");
    ecs_os_set_api_defaults();
    ecs_os_api_t OsApi = ecs_os_api;
    OsApi.log_ = EcsLog;
    ecs_os_set_api(&OsApi);

    LogTrace("Creating ECS world");
    EngineEcsWorld = ecs_init();

    //ecs_set_target_fps(
    //    EngineEcsWorld,
    //    60
    //    );

#if defined(PURPL_DEBUG) || defined(PURPL_RELWITHDEBINFO)
    LogTrace("Initializing ECS REST");
    ECS_IMPORT(
        EngineEcsWorld,
        FlecsMonitor
        );
    ecs_singleton_set(
        EngineEcsWorld,
        EcsRest,
        {0}
        );
#endif

    LogTrace("Importing components and systems");
    ECS_IMPORT(
        EngineEcsWorld,
        Camera
        );
    ECS_IMPORT(
        EngineEcsWorld,
        Math
        );
    ECS_IMPORT(
        EngineEcsWorld,
        Render
        );
}

VOID
EngineEcsBeginFrame(
    _In_ UINT64 Delta
    )
{
    ecs_frame_begin(
        EngineEcsWorld,
        (FLOAT)Delta / 1000
        );
}

VOID
EngineEcsEndFrame(
    VOID
    )
{
    ecs_frame_end(
        EngineEcsWorld
        );
}

VOID
EngineEcsShutdown(
    VOID
    )
{
    LogInfo("Shutting down ECS");
    ecs_fini(EngineEcsWorld);
}

ecs_entity_t
EngineCreateEntity(
    _In_opt_ PCSTR Name
    )
{
    LogTrace("Creating entity with name %s", Name);
    return ecs_entity(
        EngineEcsWorld,
        {
            .name = Name
        }
        );
}

VOID
EngineSetEcsWorld(
    _In_ ecs_world_t* World
    )
{
    LogInfo("Setting ECS world");

    ecs_fini(EngineEcsWorld);
    EngineEcsWorld = World;
}

ecs_world_t*
EngineGetEcsWorld(
    VOID
    )
{
    return EngineEcsWorld;
}