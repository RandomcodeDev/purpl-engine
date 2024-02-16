/// @file render.h
///
/// @brief This file declares the render API.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/log.h"

#include "platform/video.h"

/// @brief Graphics API
typedef enum RENDER_API
{
    RenderApiNone,
    RenderApiVulkan,
    RenderApiDirect3D12,
    RenderApiCount
} RENDER_API, *PRENDER_API;
extern RENDER_API RenderApi;

/// @brief Renderer backend
typedef struct RENDER_BACKEND
{
    VOID (*Initialize)(VOID);
    VOID (*BeginFrame)(_In_ BOOLEAN WindowResized);
    VOID (*EndFrame)(VOID);
    VOID (*Shutdown)(VOID);
} RENDER_BACKEND, *PRENDER_BACKEND;

/// @brief  Model component, stores backend handles
typedef struct MODEL
{
    PVOID MeshHandle;
    PVOID MaterialHandle;
} MODEL, *PMODEL;
extern ECS_COMPONENT_DECLARE(MODEL);

/// @brief Initialize the render system
extern ECS_SYSTEM_DECLARE(RdrInitialize);

/// @brief Start recording a frame
extern ECS_SYSTEM_DECLARE(RdrBeginFrame);

/// @brief Finish recording a frame and present it
extern ECS_SYSTEM_DECLARE(RdrEndFrame);

/// @brief Shut down the render system
extern VOID RdrShutdown(VOID);

/// @brief Get the scale of the render output
///
/// @return The scale
extern FLOAT RdrGetScale(VOID);

/// @brief Set the scale of the render output
///
/// @param[in] NewScale The new scale of the render output
///
/// @return The old scale of the render output
extern FLOAT RdrSetScale(FLOAT NewScale);

/// @brief Get the width of the render output
///
/// @return The scaled width of the render output
extern UINT32 RdrGetWidth(VOID);

/// @brief Get the height of the render output
///
/// @return The scaled height of the render output
extern UINT32 RdrGetHeight(VOID);
