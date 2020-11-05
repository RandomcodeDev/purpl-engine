#pragma once

#ifndef PURPL_VULKAN_COMMAND_POOL_H
#define PURPL_VULKAN_COMMAND_POOL_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <cglm/call.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

#include "macro.h"
#include "queuefamily.h"

namespace purpl
{
/*
 * Creates a command pool for rendering.
 * Defined in command.cc
 */
VkCommandPool create_command_pool(VkDevice device,
				  struct queue_family_indices indices);

/*
 * Allocates command buffers for rendering.
 * Defined in command.cc
 */
VkCommandBuffer *
allocate_command_buffers(VkDevice device, VkCommandPool command_pool,
			 uint buffer_count, VkRenderPass render_pass,
			 VkFramebuffer *framebuffers, VkExtent2D extent,
			 VkPipeline pipeline);

/*
 * Creates the semaphores for synchronizing presentation steps.
 * Defined in command.cc
 */
uint create_sync_objects(VkDevice device, VkSemaphore **image_available,
			 VkSemaphore **render_finished, VkFence **in_flight,
			 VkFence **images_in_flight, uint image_count);

/*
 * Renders a buffer.
 * Defined in command.cc
 */
//uint draw_buffer(VkRenderPass render_pass, VkExtent2D extent, VkFramebuffer *framebuffers, VkCommandBuffer *command_buffers, VkPipeline pipeline, vec4 *vert_buffer, vec4 *color_buffer, int vert_count);
}

#endif /* !PURPL_VULKAN_COMMAND_POOL_H */
