#pragma once

#ifndef PURPL_WIN32_GRAPHICS_H
#define PURPL_WIN32_GRAPHICS_H

#ifdef P_USE_VULKAN_GFX
#include "vulkan/vulkan_graphics.h"

namespace purpl
{
typedef purpl::win32_vulkan_inst win32_gfx_inst;
}
#else
#error "No/invalid graphics API specified. Graphics will not work."
#endif

#endif
