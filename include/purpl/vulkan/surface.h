#pragma once

#ifndef PURPL_VULKAN_SURFACE_H
#define PURPL_VULKAN_SURFACE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#include <xcb/xcb.h>

#define VK_USE_PLATFORM_XLIB_KHR
#endif

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/window.h"

#include "macro.h"

namespace purpl
{
/*
 * Handles the creation of a surface suitable for rendering to.
 * Defined in surface.cc
 */
VkSurfaceKHR create_surface(VkInstance instance, window *wnd);
}

#endif /* !PURPL_VULKAN_SURFACE_H */
