/*++

Copyright (c) 2023 MobSlicer152

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

#include "rendersystem/rendersystem.h"

#include "camera.h"
#include "mathutil.h"
#include "ui.h"

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
EngineInitialize(
    VOID
    );

//
// Update the engine
//

extern
VOID
EngineUpdate(
    VOID
    );

//
// Shut down the engine
//

extern
VOID
EngineShutdown(
    VOID
    );

//
// Get the main camera
//

extern
ecs_entity_t
EngineGetMainCamera(
    VOID
    );

//
// Set the main camera
//

extern
VOID
EngineSetMainCamera(
    _In_ ecs_entity_t Entity
    );

//
// Get the frame delta in seconds
//

extern
DOUBLE
EngineGetDelta(
    VOID
    );

//
// Get the framerate
//

extern
UINT32
EngineGetFramerate(
    VOID
    );

//
// Get the runtime of the engine
//

extern
UINT64
EngineGetRuntime(
    VOID
    );
