/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    entity.h

Abstract:

    This module defines ECS components and helper functions for entities.

--*/

#pragma once

#include "purpl/purpl.h"

#include "util/mesh.h"
#include "util/texture.h"

//
// Initialize the ECS
//

extern
VOID
EngineEcsInitialize(
    VOID
    );

//
// Begin an ECS frame
//

extern
VOID
EngineEcsBeginFrame(
    _In_ UINT64 Delta
    );

//
// End an ECS frame
//

extern
VOID
EngineEcsEndFrame(
    VOID
    );

//
// Shut down the ECS
//

extern
VOID
EngineEcsShutdown(
    VOID
    );

//
// Create an entity
//

extern
ecs_entity_t
EngineCreateEntity(
    _In_opt_ PCSTR Name
    );

//
// Set the ECS world
//

extern
VOID
EngineSetEcsWorld(
    _In_ ecs_world_t* World
    );

//
// Get the ECS world
//

extern
ecs_world_t*
EngineGetEcsWorld(
    VOID
    );
