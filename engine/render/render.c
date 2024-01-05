/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    render.c

Abstract:

    This module implements part of the render API.

--*/

#include "render.h"

ecs_entity_t ecs_id(RdrInitialize);
ecs_entity_t ecs_id(RdrBeginFrame);
ecs_entity_t ecs_id(RdrEndFrame);

static RENDER_API Api;
static RENDER_BACKEND Backend;

VOID
RdrInitialize(
    _In_ ecs_iter_t* Iterator
    )
{
    LogInfo("Initializing render system");

#ifdef PURPL_WIN32
    Api = RenderApiDirect3D12;
    D3d12SetupBackend(&Backend);
#endif

    if ( Backend.Initialize )
    {
        Backend.Initialize();
    }

    LogInfo("Render system initialization succeeded");
}

VOID
RdrBeginFrame(
    _In_ ecs_iter_t* Iterator
    )
{
    if ( Backend.BeginFrame )
    {
        Backend.BeginFrame();
    }
}

VOID
RdrEndFrame(
    _In_ ecs_iter_t* Iterator
    )
{
    if ( Backend.EndFrame )
    {
        Backend.EndFrame();
    }
}

VOID
RdrShutdown(
    VOID
    )
{
    LogInfo("Shutting down render system");

    if ( Backend.Shutdown )
    {
        Backend.Shutdown();
    }
    
    LogInfo("Render system shutdown succeeded");
}

VOID
RenderImport(
    _In_ ecs_world_t* World
    )
{
    LogTrace("Importing Render module");

    ECS_MODULE(
        World,
        Render
        );

    ECS_SYSTEM_DEFINE(
        World,
        RdrInitialize,
        EcsOnStart
        );
    ECS_SYSTEM_DEFINE(
        World,
        RdrBeginFrame,
        EcsPreUpdate
        );
    ECS_SYSTEM_DEFINE(
        World,
        RdrEndFrame,
        EcsPostUpdate
        );
}
