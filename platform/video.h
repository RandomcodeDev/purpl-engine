/*++

Copyright (c) 2023 Randomcode Developers

Module Name:

    video.h

Abstract:

    This module contains the video abstraction API.

--*/

#pragma once

#include "purpl/purpl.h"

//
// Initialize platform video
//

extern
VOID
VidInitialize(
    VOID
    );

//
// Update video (and input)
//

extern
BOOLEAN
VidUpdate(
    VOID
    );

//
// Clean up video resources
//

extern
VOID
VidShutdown(
    VOID
    );

//
// Get the size of the video output
//

extern
VOID
VidGetSize(
    _Out_opt_ PUINT32 Width,
    _Out_opt_ PUINT32 Height
    );

//
// Determine whether the video output has been resized since the last call
//

extern
BOOLEAN
VidResized(
    VOID
    );

//
// Determine whether the video output is focused
//

extern
BOOLEAN
VidFocused(
    VOID
    );

//
// Get the window or similar object
//

extern
PVOID
VidGetObject(
    VOID
    );

//
// Get the current DPI
//

extern
FLOAT
VidGetDpi(
    VOID
    );

#ifdef PURPL_VULKAN
//
// Get the address of a Vulkan function
//

extern
PFN_vkVoidFunction
PlatGetVulkanFunction(
    _In_ VkInstance Instance,
    _In_ PCSTR Name
    );

//
// Create a Vulkan surface
//

extern
PVOID
PlatCreateVulkanSurface(
    _In_ VkInstance Instance,
    _In_opt_ PVOID WindowHandle
    );
#endif