/*++

Copyright (c) 2024 Randomcode Developers

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
EcsInitialize(
    VOID
    );

//
// Begin an ECS frame
//

extern
VOID
EcsBeginFrame(
    _In_ UINT64 Delta
    );

//
// End an ECS frame
//

extern
VOID
EcsEndFrame(
    VOID
    );

//
// Shut down the ECS
//

extern
VOID
EcsShutdown(
    VOID
    );

//
// Create an entity
//

extern
ecs_entity_t
EcsCreateEntity(
    _In_opt_ PCSTR Name
    );

//
// Set the ECS world
//

extern
VOID
EcsSetWorld(
    _In_ ecs_world_t* World
    );

//
// Get the ECS world
//

extern
ecs_world_t*
EcsGetWorld(
    VOID
    );
