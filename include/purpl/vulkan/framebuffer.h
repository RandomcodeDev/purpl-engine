#pragma once

#ifndef PURPL_VULKAN_FRAMEBUFFER_H
#define PURPL_VULKAN_FRAMEBUFFER_H

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
/*
 * Creates framebuffers from the supplied image views.
 * Defined in framebuffer.cc
 */
VkFramebuffer *create_framebuffers(VkDevice device, VkImageView *image_views,
				   uint image_view_count, VkExtent2D extent,
				   VkRenderPass render_pass);
}

#endif /* !PURPL_VULKAN_FRAMEBUFFER_H */
