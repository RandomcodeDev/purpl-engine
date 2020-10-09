#pragma once

#ifndef PURPL_GRAPHICS_H
#define PURPL_GRAPHICS_H

#ifdef _WIN32
#include "win32/graphics.h"

namespace purpl
{
typedef purpl::win32_gfx_inst gfx_inst;
}
#else
#error "Graphics are unsupported on this platform"
#endif

#endif
