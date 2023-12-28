/*++

Module Name:

    engine.c

Abstract:

    This module implements the engine API.

--*/

#include "engine.h"

PCHAR EngineDataDirectory;

CONST PCSTR EngineDataDirectories[EngineDataDirectoryCount] = {
    "saves/", // EngineDataDirectorySaves
    "logs/", // EngineDataDirectoryLogs
};

ecs_entity_t EngineMainCamera;

VOID
EngInitialize(
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

    EngineDataDirectory = CmnFormatString(
        "%s" GAME_EXECUTABLE_NAME "/",
        PlatGetUserDataDirectory()
        );

    LogInfo("Ensuring engine data directory %s exists", EngineDataDirectory);
    if ( !FsCreateDirectory(EngineDataDirectory) )
    {
        CmnError("Failed to create engine directory %s", EngineDataDirectory);
    }
    for ( i = 0; i < EngineDataDirectoryCount; i++ )
    {
        LogDebug("Creating directory %s%s", EngineDataDirectory, EngineDataDirectories[i]);
        Path = CmnFormatTempString(
            "%s%s",
            EngineDataDirectory,
            EngineDataDirectories[i]
            );
        if ( !FsCreateDirectory(Path) )
        {
            CmnError("Failed to create engine directory %s", Path);
        }
    }

    time_t RawTime;
    struct tm Time;
    RawTime = time(NULL);
    Time = *localtime(&RawTime);
    Path = CmnFormatTempString(
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
        CmnError("Failed to open log file %s: %s", Path, strerror(errno));
    }

    LogAddFile(
        LogFile,
        LogGetLevel()
        );

    LogInfo(PURPL_BUILD_TYPE " engine running on %s", PlatGetDescription());

    VidInitialize();

    EcsInitialize();

    LogInfo("Successfully initialized engine with data directory %s", EngineDataDirectory);
}

static UINT64 Start;
static DOUBLE Last;
static DOUBLE Now;
static DOUBLE Time;
static UINT FramesThisSecond;
static UINT FramesPerSecond;
static DOUBLE Delta;

DOUBLE
EngGetDelta(
    VOID
    )
{
    return Delta / 1000;
}

UINT32
EngGetFramerate(
    VOID
    )
{
    return FramesPerSecond;
}

UINT64
EngGetRuntime(
    VOID
    )
{
    return time(NULL) - Start;
}

VOID
EngMainLoop(
    VOID
    )
{
    BOOLEAN Running;

    Running = TRUE;
    while ( Running )
    {
        Running = VidUpdate();

        EngStartFrame();

        ecs_progress(
            EcsGetWorld(),
            (FLOAT)Delta
            );

        EngEndFrame();
    }
}

// TODO: integrate into EngMainLoop?
VOID
EngStartFrame(
    VOID
    )
{
    UINT64 Hours;
    UINT64 Minutes;
    UINT64 Seconds;

    Now = (DOUBLE)PlatGetMilliseconds();
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

    Seconds = EngGetRuntime();
    Hours = Seconds / 60 / 60;
    Seconds -= Hours * 60 * 60;
    Minutes = Seconds / 60;
    Seconds -= Minutes * 60;
}

// TODO: integrate into EngMainLoop?
VOID
EngEndFrame(
    VOID
    )
/*++

Routine Description:

    Finishes the frame.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Last = Now;
}

VOID
EngShutdown(
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

    EcsShutdown();
    VidShutdown();

    PURPL_FREE(EngineDataDirectory);

    LogInfo("Engine shutdown succeeded");
}

ecs_entity_t
EngGetMainCamera(
    VOID
    )
{
    return EngineMainCamera;
}

VOID
EngSetMainCamera(
    _In_ ecs_entity_t Camera
    )
{
    LogTrace("Setting main camera to %u", Camera);
    PCAMERA CameraComponent = ecs_get_mut(
        EcsGetWorld(),
        Camera,
        CAMERA
        );
    if ( CameraComponent )
    {
        EngineMainCamera = Camera;
        CalculateCameraMatrices(CameraComponent);
    }
}
