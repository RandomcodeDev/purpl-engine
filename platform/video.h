/*++

Copyright (c) 2023 Randomcode Developers

Module Name:

    video.h

Abstract:

    This file contains the video abstraction API.

--*/

#pragma once

#include "purpl/purpl.h"

/// @brief Initialize platform video
extern VOID VidInitialize(VOID);

/// @brief Update video (and input)
///
/// @return Whether the window is open
extern BOOLEAN VidUpdate(VOID);

/// @brief Shut down platform video
extern VOID VidShutdown(VOID);

/// @brief Get the size of the platform video output
///
/// @param[out] Width Receives the width
/// @param[out] Height Receives the height
extern VOID VidGetSize(_Out_opt_ PUINT32 Width, _Out_opt_ PUINT32 Height);

/// @brief Get whether the video output has been resized since the last call
extern BOOLEAN VidResized(VOID);

/// @brief Get whether the video output is focused
extern BOOLEAN VidFocused(VOID);

/// @brief Get the equivalent of a window handle (HWND, GLFWwindow, etc)
///
/// @return The video output object
extern PVOID VidGetObject(VOID);

/// @brief Get the current DPI
///
/// @return The current DPI
extern FLOAT VidGetDpi(VOID);

#ifdef PURPL_VULKAN
/// @brief Get the address of a Vulkan symbol
///
/// @param[in] Instance The Vulkan instance
/// @param[in] Name The name of the symbol
///
/// @return The address of the symbol
extern PVOID PlatGetVulkanFunction(_In_ PVOID Instance, _In_ PCSTR Name);

/// @brief Create a Vulkan surface
///
/// @param[in] Instance The Vulkan instance
/// @param[in] AllocationCallbacks Allocation functions (should be the return
/// value of VlkGetAllocationCallbacks)
/// @param[in] WindowHandle The handle of the window to use, if not the engine's
///
/// @return A Vulkan surface
extern VkSurfaceKHR VidCreateVulkanSurface(_In_ VkInstance Instance, _In_ PVOID AllocationCallbacks,
                                           _In_opt_ PVOID WindowHandle);
#endif
