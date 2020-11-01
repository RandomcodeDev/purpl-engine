#pragma once

#ifndef PURPL_VULKAN_PHYSICAL_DEVICE_H
#define PURPL_VULKAN_PHYSICAL_DEVICE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

#include "queuefamily.h"
#include "swapchain.h"

namespace purpl
{
/*
 * Scores a device based on its available features.
 * Defined in device.cc
 */
uint score_device(VkPhysicalDevice device);

/*
 * Returns a pointer to the highest ranking device capable of rendering, or NULL if one can't be fount.
 * Defined in device.cc
 */
VkPhysicalDevice
locate_suitable_device(VkInstance inst, VkSurfaceKHR surface,
			      struct queue_family_indices *indices,
			      struct swapchain_details *details);
}

#endif /* !PURPL_VULKAN_PHYSICAL_DEVICE_H */
