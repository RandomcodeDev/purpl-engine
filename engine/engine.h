/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    engine.h

Abstract:

    This module defines the engine API.

--*/

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/filesystem.h"
#include "common/log.h"

#include "platform/video.h"

#include "render/render.h"

#include "camera.h"
#include "entity.h"
#include "mathutil.h"

//
// The engine's data directory
//

extern PCHAR EngineDataDirectory;

//
// The subdirectories of the data directory
//

typedef enum ENGINE_DATA_DIRECTORY
{
    EngineDataDirectorySaves,
    EngineDataDirectoryLogs,
    EngineDataDirectoryCount
} ENGINE_DATA_DIRECTORY, *PENGINE_DATA_DIRECTORY;
extern CONST PCSTR EngineDataDirectories[EngineDataDirectoryCount];

//
// Initialize the engine
//

extern
VOID
EngInitialize(
    VOID
    );

//
// The main loop of the engine
//

extern
VOID
EngMainLoop(
    VOID
    );

//
// Start a frame
//

VOID
EngStartFrame(
    VOID
    );

//
// End a frame
//

extern
VOID
EngEndFrame(
    VOID
    );

//
// Shut down the engine
//

extern
VOID
EngShutdown(
    VOID
    );

//
// Get the main camera
//

extern
ecs_entity_t
EngGetMainCamera(
    VOID
    );

//
// Set the main camera
//

extern
VOID
EngSetMainCamera(
    _In_ ecs_entity_t Entity
    );

//
// Get the frame delta in seconds
//

extern
DOUBLE
EngGetDelta(
    VOID
    );

//
// Get the framerate
//

extern
UINT32
EngGetFramerate(
    VOID
    );

//
// Get the runtime of the engine
//

extern
UINT64
EngGetRuntime(
    VOID
    );
