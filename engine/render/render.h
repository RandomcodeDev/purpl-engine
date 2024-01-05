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

#ifdef PURPL_DIRECTX
#include "d3d12.h"
#endif

#ifdef PURPL_VULKAN
//#include "vk.h"
#endif

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
// API-specific render backend
//

typedef struct RENDER_BACKEND
{
    VOID
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
