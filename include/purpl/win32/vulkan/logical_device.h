#pragma once

#ifndef PURPL_WIN32_VULKAN_LOGICAL_DEVICE_H
#define PURPL_WIN32_VULKAN_LOGICAL_DEVICE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "queuefamily.h"

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
/*
 * Creates a logical device with the specified queue families.
 * Defined in logical_device.cc
 */
VkDevice create_logical_device(VkPhysicalDevice physical_device, struct queue_family_indices indices);
}

#endif /* !PURPL_WIN32_VULKAN_LOGICAL_DEVICE_H */
