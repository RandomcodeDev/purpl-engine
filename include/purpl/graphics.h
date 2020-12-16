#pragma once

#ifndef PURPL_GRAPHICS_H
#define PURPL_GRAPHICS_H

#ifdef P_USE_VULKAN_GFX
#include "vulkan/vulkan_graphics.h"

namespace purpl
{
typedef purpl::vulkan_inst gfx_inst;
}
#elif defined(P_USE_OPENGL_GRAPHICS)
#ifdef _WIN32
#include "win32/opengl.h"

namespace purpl
{
typedef purpl::opengl_inst gfx_inst;
}
#else
#error "OpenGL-based graphics are unsupported on this platform"
#endif
#else
#error "Graphics are unsupported on this platform"
#endif

#endif
