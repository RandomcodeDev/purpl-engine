#pragma once

#ifndef PURPL_WIN32_GRAPHICS_H
#define PURPL_WIN32_GRAPHICS_H

#ifdef P_USE_VULKAN_GFX
#include "vulkan/vulkan.h"
#else
#error "No/invalid graphics API specified. Graphics will not work."
#endif

#endif
