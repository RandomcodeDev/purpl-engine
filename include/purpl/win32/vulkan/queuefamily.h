#pragma once

#ifndef PURPL_WIN32_VULKAN_QUEUEFAMILY_H
#define PURPL_WIN32_VULKAN_QUEUEFAMILY_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
/* A structure to hold the indices for the queue families that we need */
struct queue_family_indices {
	uint graphics_family; /* The index of the graphics family */
	bool has_graphics_family; /* Whether we have a graphics family */
};

/*
 * Retrieves indices of the queue families listed in the queue_family_indices struct.
 * Defined in queuefamily.cc
 */
struct queue_family_indices find_queue_families(VkPhysicalDevice device);
}

#endif /* !PURPL_WIN32_VULKAN_QUEUEFAMILY_H */
