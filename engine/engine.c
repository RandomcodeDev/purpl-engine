/*++

Module Name:

    engine.c

Abstract:

    This module implements the engine API.

--*/

#include "common/common.h"

#include "platform/platform.h"

#include "engine.h"
#include "entity.h"

PCHAR EngineDataDirectory;

CONST PCSTR EngineDataDirectories[EngineDataDirectoryCount] = {
    "saves/", // EngineDataDirectorySaves
    "logs/", // EngineDataDirectoryLogs
};

ecs_entity_t EngineMainCamera;

VOID
EngineInitialize(
    VOID
    )
/*++

Routine Description:

    Initializes the engine.

Arguments:

    None.

Return Value:

    None.

--*/
{
    PCSTR Path;
    UINT i;

    LogInfo("Initializing engine");

    EngineDataDirectory = CommonFormatString(
        "%s/" GAME_EXECUTABLE_NAME "/",
        PlatformGetUserDataDirectory()
        );

    LogInfo("Ensuring engine data directory %s exists", EngineDataDirectory);
    if ( !FsCreateDirectory(EngineDataDirectory) )
    {
        CommonError("Failed to create engine directory %s", EngineDataDirectory);
    }
    for ( i = 0; i < EngineDataDirectoryCount; i++ )
    {
        LogDebug("Creating directory %s%s", EngineDataDirectory, EngineDataDirectories[i]);
        Path = CommonFormatTempString(
            "%s%s",
            EngineDataDirectory,
            EngineDataDirectories[i]
            );
        if ( !FsCreateDirectory(Path) )
        {
            CommonError("Failed to create engine directory %s", Path);
        }
    }

    time_t RawTime;
    struct tm Time;
    RawTime = time(NULL);
    Time = *localtime(&RawTime);
    Path = CommonFormatTempString(
        "%s%spurpl_%04d-%02d-%02d_%02d-%02d-%02d.log",
        EngineDataDirectory,
        EngineDataDirectories[EngineDataDirectoryLogs],
        Time.tm_year + 1900,
        Time.tm_mon + 1,
        Time.tm_mday,
        Time.tm_hour,
        Time.tm_min,
        Time.tm_sec
        );
    LogInfo("Opening log file %s", Path);
    FILE* LogFile = fopen(
        Path,
        "ab"
        );
    if ( !LogFile )
    {
        CommonError("Failed to open log file %s: %s", Path, strerror(errno));
    }

    LogAddFile(
        LogFile,
        LogGetLevel()
        );

    LogInfo(PURPL_BUILD_TYPE " engine running on %s", PlatformGetDescription());

    PlatformInitializeVideo();

    RenderInitialize();
    EngineEcsInitialize();
    UiInitialize();

    LogInfo("Successfully initialized engine with data directory %s", EngineDataDirectory);
}

static UINT64 Start;
static DOUBLE Last;
static DOUBLE Time;
static UINT FramesThisSecond;
static UINT FramesPerSecond;
static DOUBLE Delta;

DOUBLE
EngineGetDelta(
    VOID
    )
{
    return Delta / 1000;
}

UINT32
EngineGetFramerate(
    VOID
    )
{
    return FramesPerSecond;
}

UINT64
EngineGetRuntime(
    VOID
    )
{
    return time(NULL) - Start;
}

VOID
EngineUpdate(
    VOID
    )
/*++

Routine Description:

    Updates engine systems if the engine has focus.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DOUBLE Now;
    UINT64 Hours;
    UINT64 Minutes;
    UINT64 Seconds;

    Now = (DOUBLE)PlatformGetMilliseconds();
    Delta = Now - Last;
    if ( Last != 0 )
    {
        Time += Delta;
        FramesThisSecond++;
        if ( Time > 1000 )
        {
            FramesPerSecond = FramesThisSecond;
            Time = 0;
            FramesThisSecond = 0;
        }
    }
    else
    {
        Start = time(NULL);
    }

    Seconds = EngineGetRuntime();
    Hours = Seconds / 60 / 60;
    Seconds -= Hours * 60 * 60;
    Minutes = Seconds / 60;
    Seconds -= Minutes * 60;

    RenderBeginCommands();

    RenderDrawString(
        "purpl-regular",
        NULL,
        "asdf"
        );

    ecs_progress(
        EngineGetEcsWorld(),
        (FLOAT)Delta
        );

    RenderPresentFrame();

    Last = Now;
}

VOID
EngineShutdown(
    VOID
    )
/*++

Routine Description:

    Shuts down the engine.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogInfo("Shutting down engine");

    UiShutdown();
    EngineEcsShutdown();
    RenderShutdown();
    PlatformShutdownVideo();

    PURPL_FREE(EngineDataDirectory);

    LogInfo("Engine shutdown succeeded");
}

ecs_entity_t
EngineGetMainCamera(
    VOID
    )
{
    return EngineMainCamera;
}

VOID
EngineSetMainCamera(
    _In_ ecs_entity_t Camera
    )
{
    LogTrace("Setting main camera to %u", Camera);
    PCAMERA CameraComponent = ecs_get_mut(
        EngineGetEcsWorld(),
        Camera,
        CAMERA
        );
    if ( CameraComponent )
    {
        EngineMainCamera = Camera;
        CalculateCameraMatrices(CameraComponent);
    }
}
