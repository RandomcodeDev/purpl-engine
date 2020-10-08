#pragma once

#ifndef PURPL_WIN32_VULKAN_H
#define PURPL_WIN32_VULKAN_H

#include "inst.h"
#define gfx_inst win32_vulkan_inst

#include "util.h"

/* Validation layers, which are only needed if we're built in debug mode */
#ifndef NDEBUG
#include "validation.h"
#endif

#endif
