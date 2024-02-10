/// @file engine.h
///
/// @brief This file defines the engine API.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/alloc.h"
#include "common/common.h"
#include "common/filesystem.h"
#include "common/log.h"

#include "platform/video.h"

#include "render/render.h"

#include "camera.h"
#include "entity.h"
#include "mathutil.h"

#ifdef PURPL_DISCORD
#include "discord/discord.h"
#endif

/// @brief The engine's data directory
extern PCHAR EngineDataDirectory;

/// @brief The subdirectories of the data directory
typedef enum ENGINE_DATA_DIRECTORY
{
    EngineDataDirectorySaves,
    EngineDataDirectoryLogs,
    EngineDataDirectoryCount
} ENGINE_DATA_DIRECTORY, *PENGINE_DATA_DIRECTORY;
extern CONST PCSTR EngineDataDirectories[EngineDataDirectoryCount];

/// @brief Initialize the engine
extern VOID EngInitialize(VOID);

/// @brief The main loop of the engine
extern VOID EngMainLoop(VOID);

/// @brief Start a frame
VOID EngStartFrame(VOID);

/// @brief End a frame
extern VOID EngEndFrame(VOID);

/// @brief Shut down the engine
extern VOID EngShutdown(VOID);

/// @brief Get the main camera
///
/// @return The entity with a camera component that is the main camera
extern ecs_entity_t EngGetMainCamera(VOID);

/// @brief Set the main camera
///
/// @param[in] Entity The entity to set as the main camera
extern VOID EngSetMainCamera(_In_ ecs_entity_t Entity);

/// @brief Get the frame delta in seconds
extern DOUBLE EngGetDelta(VOID);

/// @brief Get the framerate
extern UINT32 EngGetFramerate(VOID);

/// @brief Get the runtime of the engine
extern UINT64 EngGetRuntime(VOID);

/// @brief Get whether the video has been resized
extern BOOLEAN EngHasVideoResized(VOID);
