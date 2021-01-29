#pragma once

#ifndef PURPL_VULKAN_QUEUEFAMILY_H
#define PURPL_VULKAN_QUEUEFAMILY_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "purpl/macro.h"

#include <vulkan/vulkan.h>

#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
/* A structure to hold the indices for the queue families that we need */
struct queue_family_indices {
	uint graphics_family; /* The index of the graphics family */
	uint has_graphics_family; /* Whether we have a graphics family (int rather than bool for Vulkan reasons) */
	uint present_family; /* The index of the present family */
	uint has_present_family; /* Whether we have a present family (int rather than bool for Vulkan reasons) */
};

/* Retrieves indices of the queue families listed in the queue_family_indices struct, which are necessary for the engine to work correctly. */
struct queue_family_indices find_queue_families(VkPhysicalDevice device,
						VkSurfaceKHR surface);
}

#endif /* !PURPL_VULKAN_QUEUEFAMILY_H */
