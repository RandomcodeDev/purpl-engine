#pragma once

#ifndef PURPL_GRAPHICS_H
#define PURPL_GRAPHICS_H

#ifdef P_USE_VULKAN_GFX
#include "vulkan/vulkan_graphics.h"

namespace purpl
{
typedef purpl::vulkan_inst gfx_inst;
}
#else
#error "Graphics are unsupported on this platform"
#endif

#endif
