/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    render.c

Abstract:

    This module implements part of the render API.

--*/

#include "render.h"

ecs_entity_t ecs_id(RdrInitialize);

VOID
RdrInitialize(
    _In_ ecs_world_t* World
    )
{

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
}
