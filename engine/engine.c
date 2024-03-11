/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    engine.c

Abstract:

    This file implements the engine API.

--*/

#include "engine.h"

#define X(Kind)                                                                                                        \
    {                                                                                                                  \
        static CHAR Buffer[1024];                                                                                      \
                                                                                                                       \
        if (Directory >= EngDataDirectoryCount)                                                                        \
        {                                                                                                              \
            memset(Buffer, 0, PURPL_ARRAYSIZE(Buffer));                                                                \
            return NULL;                                                                                               \
        }                                                                                                              \
                                                                                                                       \
        PSTR FormattedName = "";                                                                                       \
        if (Name && strlen(Name))                                                                                      \
        {                                                                                                              \
            va_list Arguments;                                                                                         \
            va_start(Arguments, Name);                                                                                 \
            FormattedName = CmnFormatStringVarArgs(Name, Arguments);                                                   \
            va_end(Arguments);                                                                                         \
        }                                                                                                              \
                                                                                                                       \
        stbsp_snprintf(Buffer, PURPL_ARRAYSIZE(Buffer), "%s/%s", Eng##Kind##Directories[Directory], FormattedName);    \
                                                                                                                       \
        if (Name && strlen(Name))                                                                                      \
        {                                                                                                              \
            CmnFree(FormattedName);                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        return Buffer;                                                                                                 \
    }

PCHAR EngDataDirectory;

CONST PCSTR EngDataDirectories[EngDataDirectoryCount] = {
    "saves/", // EngDataDirectorySaves
    "logs/",  // EngDataDirectoryLogs
};

PCHAR EngGetDataPath(_In_ ENGINE_DATA_DIRECTORY Directory, _In_opt_ PCSTR Name, ...) X(Data);

CONST PCSTR EngAssetDirectories[EngAssetDirectoryCount] = {
    "models",   // EngAssetDirectoryModels
    "shaders",  // EngAssetDirectoryShaders
    "textures", // EngAssetDirectoryTextures
};

PCHAR EngGetAssetPath(_In_ ENGINE_ASSET_DIRECTORY Directory, _In_opt_ PCSTR Name, ...) X(Asset);

#undef X

ecs_entity_t EngineMainCamera;

VOID EngDefineVariables(VOID)
{
    EcsDefineVariables();
    RdrDefineVariables();
}

VOID EngInitialize(VOID)
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

    EngDataDirectory = CmnFormatString("%s" PURPL_EXECUTABLE_NAME "/", PlatGetUserDataDirectory());

    LogInfo("Ensuring engine data directory %s exists", EngDataDirectory);
    if (!FsCreateDirectory(EngDataDirectory))
    {
        CmnError("Failed to create engine directory %s", EngDataDirectory);
    }
    for (i = 0; i < EngDataDirectoryCount; i++)
    {
        LogDebug("Creating directory %s%s", EngDataDirectory, EngDataDirectories[i]);
        Path = CmnFormatTempString("%s%s", EngDataDirectory, EngDataDirectories[i]);
        if (!FsCreateDirectory(Path))
        {
            CmnError("Failed to create engine directory %s", Path);
        }
    }

    FsAddDirectorySource("assets");
#if defined PURPL_DEBUG && !defined PURPL_SWITCH
    FsAddDirectorySource("assets/out");
#endif

#ifndef PURPL_SWITCH
    time_t RawTime;
    struct tm Time;
    RawTime = time(NULL);
    Time = *localtime(&RawTime);
    Path = CmnFormatTempString("%s%spurpl_%04d-%02d-%02d_%02d-%02d-%02d.log", EngDataDirectory,
                               EngDataDirectories[EngDataDirectoryLogs], Time.tm_year + 1900, Time.tm_mon + 1,
                               Time.tm_mday, Time.tm_hour, Time.tm_min, Time.tm_sec);

    LogInfo("Opening log file %s", Path);
    FILE *LogFile = fopen(Path, "ab");
    if (!LogFile)
    {
        CmnError("Failed to open log file %s: %s", Path, strerror(errno));
    }

    LogAddFile(LogFile, LogGetLevel());
#endif

    LogInfo(PURPL_BUILD_TYPE " engine running on %s", PlatGetDescription());

    VidInitialize(FALSE);
    EcsInitialize();

    LogInfo("Successfully initialized engine, data directory is %s", EngDataDirectory);
}

static UINT64 Start;
static DOUBLE Last;
static DOUBLE Now;
static DOUBLE Time;
static UINT FramesThisSecond;
static UINT FramesPerSecond;
static DOUBLE Delta;
static BOOLEAN Resized;

DOUBLE EngGetDelta(VOID)
{
    return Delta / 1000.0;
}

UINT32
EngGetFramerate(VOID)
{
    return FramesPerSecond;
}

UINT64
EngGetRuntime(VOID)
{
    return time(NULL) - Start;
}

VOID EngMainLoop(VOID)
{
    BOOLEAN Running;

    Running = TRUE;
    while (Running)
    {
        Running = VidUpdate();

        EngStartFrame();
        ecs_progress(EcsGetWorld(), (FLOAT)Delta);
        EngEndFrame();
    }
}

// TODO: integrate into EngMainLoop?
VOID EngStartFrame(VOID)
{
    UINT64 Hours;
    UINT64 Minutes;
    UINT64 Seconds;

    Now = (DOUBLE)PlatGetMilliseconds();
    Delta = Now - Last;
    if (Last != 0)
    {
        Time += Delta;
        FramesThisSecond++;
        if (Time > 1000)
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

    Resized = VidResized();
}

// TODO: integrate into EngMainLoop?
VOID EngEndFrame(VOID)
{
    Last = Now;
}

VOID EngShutdown(VOID)
{
    LogInfo("Shutting down engine");

#ifdef PURPL_DISCORD
    DiscordShutdown();
#endif
    EcsShutdown();
    RdrShutdown();
    VidShutdown();

    CmnFree(EngDataDirectory);

    LogInfo("Successfully shut down engine");
}

ecs_entity_t EngGetMainCamera(VOID)
{
    return EngineMainCamera;
}

VOID EngSetMainCamera(_In_ ecs_entity_t Camera)
{
    LogTrace("Setting main camera to entity %u", Camera);
    PCAMERA CameraComponent = ecs_get_mut(EcsGetWorld(), Camera, CAMERA);
    if (CameraComponent)
    {
        EngineMainCamera = Camera;
        CalculateCameraMatrices(CameraComponent);
    }
}

BOOLEAN EngHasVideoResized(VOID)
{
    return Resized;
}
