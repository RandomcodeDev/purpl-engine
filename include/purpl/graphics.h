#pragma once

#ifndef PURPL_GRAPHICS_H
#define PURPL_GRAPHICS_H

#ifdef _WIN32
#include "win32/graphics.h"
#else
#error "Graphics are unsupported on this platform"
#endif

#endif
