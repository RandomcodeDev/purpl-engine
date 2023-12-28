/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    render.h

Abstract:

    This module declares the render API.

--*/

#pragma once

#include "purpl/purpl.h"

#include "common/log.h"

#include "platform/async.h"

extern
VOID
RdrInitialize(
    _In_ ecs_world_t* World
    );
extern ECS_SYSTEM_DECLARE(RdrInitialize);

extern
VOID
RdrShutdown(
    VOID
    );
