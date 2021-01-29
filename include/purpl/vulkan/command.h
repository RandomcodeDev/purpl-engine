#pragma once

#ifndef PURPL_VULKAN_COMMAND_POOL_H
#define PURPL_VULKAN_COMMAND_POOL_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <cglm/call.h>

#include "purpl/macro.h"

#include <vulkan/vulkan.h>

#include "purpl/types.h"
#include "purpl/util.h"

#include "macro.h"
#include "queuefamily.h"

namespace purpl
{
/* Creates a command pool for rendering. */
VkCommandPool create_command_pool(VkDevice device,
				  struct queue_family_indices indices);

/* Allocates command buffers for rendering. */
VkCommandBuffer *
allocate_command_buffers(VkDevice device, VkCommandPool command_pool,
			 uint buffer_count, VkRenderPass render_pass,
			 VkFramebuffer *framebuffers, VkExtent2D extent,
			 VkPipeline pipeline);

/* Creates the semaphores for synchronizing presentation steps. */
uint create_sync_objects(VkDevice device, VkSemaphore **image_available,
			 VkSemaphore **render_finished, VkFence **in_flight,
			 VkFence **images_in_flight, uint image_count);
}

#endif /* !PURPL_VULKAN_COMMAND_POOL_H */
