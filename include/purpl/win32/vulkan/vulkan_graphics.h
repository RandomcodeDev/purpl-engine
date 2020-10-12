#pragma once

#ifndef PURPL_WIN32_VULKAN_GRAPHICS_H
#define PURPL_WIN32_VULKAN_GRAPHICS_H

/* Validation layers, which are only needed if we're built in debug mode */
#ifndef NDEBUG
#include "debug.h"
#endif

#include "inst.h"
#include "physical_device.h"
#include "util.h"

#endif
