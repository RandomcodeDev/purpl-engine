#pragma once

#ifndef PURPL_VULKAN_IMAGE_H
#define PURPL_VULKAN_IMAGE_H

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
/*
 * Creates image views for the passed array of images.
 * Defined in image.cc
 */
VkImageView *create_image_views(VkDevice device, VkImage *images,
				uint image_count, VkFormat image_format);
}

#endif /* !PURPL_VULKAN_IMAGE_H */
