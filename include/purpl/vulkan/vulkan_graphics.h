#pragma once

#ifndef PURPL_VULKAN_GRAPHICS_H
#define PURPL_VULKAN_GRAPHICS_H

#include "buffer.h"
#include "command.h"

/* Validation layers, which are only needed if we're built in debug mode */
#ifndef NDEBUG
#include "debug.h"
#endif

#include "framebuffer.h"
#include "image.h"
#include "inst.h"
#include "logical_device.h"
#include "macro.h"
#include "physical_device.h"
#include "pipeline.h"
#include "queuefamily.h"
#include "spirv.h"
#include "surface.h"
#include "swapchain.h"
#include "util.h"

#endif
