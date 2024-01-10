/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    render.h

Abstract:

    This module declares the render API.

--*/

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/log.h"

//
// Graphics API
//

typedef enum RENDER_API
{
    RenderApiNone,
    RenderApiVulkan,
    RenderApiDirect3D12,
    RenderApiCount
} RENDER_API, *PRENDER_API;

//
// Graphics backend
//

typedef struct RENDER_BACKEND
{
    BOOLEAN
    (*Initialize)(
        VOID
        );
    VOID
    (*BeginFrame)(
        VOID
        );
    VOID
    (*EndFrame)(
        VOID
        );
    VOID
    (*Shutdown)(
        VOID
        );
} RENDER_BACKEND, *PRENDER_BACKEND;

//
// Model
//

typedef struct MODEL
{
    PVOID MeshHandle;
    PVOID MaterialHandle;
};
extern ECS_COMPONENT_DECLARE(MODEL);

extern
VOID
RdrInitialize(
    _In_ ecs_iter_t* Iterator
    );
extern ECS_SYSTEM_DECLARE(RdrInitialize);

extern
VOID
RdrBeginFrame(
    _In_ ecs_iter_t* Iterator
    );
extern ECS_SYSTEM_DECLARE(RdrBeginFrame);

extern
VOID
RdrEndFrame(
    _In_ ecs_iter_t* Iterator
    );
extern ECS_SYSTEM_DECLARE(RdrEndFrame);

extern
VOID
RdrShutdown(
    VOID
    );
