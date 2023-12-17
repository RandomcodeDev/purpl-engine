/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    platform.h

Abstract:

    This module contains the platform abstraction API.

--*/

#pragma once

#include "purpl/purpl.h"

//
// Initialize the platform layer
//

extern
VOID
PlatformInitialize(
    VOID
    );

//
// Clean up platform-specific resources
//

extern
VOID
PlatformShutdown(
    VOID
    );

//
// Initialize platform video
//

extern
VOID
PlatformInitializeVideo(
    VOID
    );

//
// Update video and input
//

extern
BOOLEAN
PlatformUpdate(
    VOID
    );

//
// Clean up video resources
//

extern
VOID
PlatformShutdownVideo(
    VOID
    );

//
// Get the size of the video output
//

extern
VOID
PlatformGetVideoSize(
    _Out_opt_ PUINT32 Width,
    _Out_opt_ PUINT32 Height
    );

//
// Determine whether the video output has been resized since the last call
//

extern
BOOLEAN
PlatformVideoResized(
    VOID
    );

//
// Determine whether the video output is focused
//

extern
BOOLEAN
PlatformVideoFocused(
    VOID
    );

//
// Get the window or similar object
//

extern
PVOID
PlatformGetVideoObject(
    VOID
    );

//
// Get the current DPI
//

extern
FLOAT
PlatformVideoGetDpi(
    VOID
    );

//
// Get a stack backtrace string
//

extern
PCSTR
PlatformCaptureStackBackTrace(
    _In_ INT FramesToSkip
    );

//
// Display an error message and terminate the process
//

extern
_Noreturn
VOID
PlatformError(
    _In_ PCSTR Message
    );

//
// Get a description of the operating system
//

extern
PCSTR
PlatformGetDescription(
    VOID
    );

//
// Get the return address
//

extern
PVOID
PlatformGetReturnAddress(
    VOID
    );

#ifdef PURPL_VULKAN
//
// Create a Vulkan surface
//

extern
PVOID
PlatformCreateVulkanSurface(
    _In_ PVOID Instance,
    _In_opt_ PVOID WindowHandle
    );
#endif

//
// Get the user data directory (AppData, XDG_DATA_HOME, ~/.local, etc)
//

extern
PCSTR
PlatformGetUserDataDirectory(
    VOID
    );

//
// Get the current time in milliseconds
//

extern
UINT64
PlatformGetMilliseconds(
    VOID
    );

//
// Create a directory
//

extern
BOOLEAN
PlatformCreateDirectory(
    _In_ PCSTR Path
    );

//
// Output text in a platform specific way like OutputDebugString
//

extern
VOID
PlatformPrint(
    _In_ PCSTR Text
    );

//
// Fix a path (if necessary)
//

extern
PCHAR
PlatformFixPath(
    _In_ PCSTR Path
    );
