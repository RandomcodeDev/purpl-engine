#include "purpl/vulkan/command.h"
using namespace purpl;

VkCommandPool purpl::create_command_pool(VkDevice device,
					 struct queue_family_indices indices)
{
	VkCommandPool command_pool;

	if (!indices.has_graphics_family) {
		errno = EINVAL;
		return NULL;
	}

	VkCommandPoolCreateInfo command_pool_info = {};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.queueFamilyIndex = indices.graphics_family;
	command_pool_info.flags = 0;

	if (vkCreateCommandPool(device, &command_pool_info, NULL,
				&command_pool) != VK_SUCCESS)
		return NULL;

	return command_pool;
}

VkCommandBuffer *
purpl::allocate_command_buffers(VkDevice device, VkCommandPool command_pool,
				uint buffer_count, VkRenderPass render_pass,
				VkFramebuffer *framebuffers, VkExtent2D extent,
				VkPipeline pipeline)
{
	VkCommandBuffer *command_buffers;
	uint i;

	/* Check our parameters */
	if (!device || !command_pool || !buffer_count || !render_pass ||
	    !framebuffers || !pipeline) {
		errno = EINVAL;
		return NULL;
	}

	/* Allocate our handles for the command buffers */
	command_buffers = (VkCommandBuffer *)calloc(buffer_count,
						    sizeof(VkCommandBuffer));
	if (!command_buffers) {
		errno = ENOMEM;
		return NULL;
	}

	/* Fill out the information for allocating the command buffers */
	VkCommandBufferAllocateInfo buffer_alloc_info = {};
	buffer_alloc_info.sType =
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	buffer_alloc_info.commandPool = command_pool;
	buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	buffer_alloc_info.commandBufferCount = buffer_count;

	if (vkAllocateCommandBuffers(device, &buffer_alloc_info,
				     command_buffers) != VK_SUCCESS)
		return NULL;

	/* Information that won't change between draws */
	VkCommandBufferBeginInfo buffer_begin_info = {};
	buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	buffer_begin_info.flags = 0;
	buffer_begin_info.pInheritanceInfo = NULL;

	VkRenderPassBeginInfo render_begin_info = {};
	render_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_begin_info.renderPass = render_pass;

	render_begin_info.renderArea.offset = { 0, 0 };
	render_begin_info.renderArea.extent = extent;

	VkClearValue clear_value = { 0.0f, 0.0f, 0.0f, 1.0f };
	render_begin_info.clearValueCount = 1;
	render_begin_info.pClearValues = &clear_value;

	/* Render stuff */
	for (i = 0; i < buffer_count; i++) {
		/* Begin the current command buffer */
		if (vkBeginCommandBuffer(command_buffers[i],
					 &buffer_begin_info) != VK_SUCCESS)
			return NULL;

		/* Begin the render pass */
		render_begin_info.framebuffer = framebuffers[i];
		vkCmdBeginRenderPass(command_buffers[i], &render_begin_info,
				     VK_SUBPASS_CONTENTS_INLINE);

		/* Bind the graphics pipeline */
		vkCmdBindPipeline(command_buffers[i],
				  VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		/* Draw our stuff */
		vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

		/* Finish our render pass */
		vkCmdEndRenderPass(command_buffers[i]);

		if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS)
			return NULL;
	}

	return command_buffers;
}

uint purpl::create_sync_objects(VkDevice device, VkSemaphore **image_available,
				VkSemaphore **render_finished,
				VkFence **in_flight, VkFence **images_in_flight,
				uint image_count)
{
	VkResult err;
	uint i;

	/* Check our arguments */
	if (!device || !image_available || !render_finished || !in_flight)
		return EINVAL;

	/* Allocate memory for our semaphores */
	*image_available = (VkSemaphore *)calloc(P_VULKAN_MAX_FRAMES_IN_FLIGHT,
						 sizeof(VkSemaphore));
	*render_finished = (VkSemaphore *)calloc(P_VULKAN_MAX_FRAMES_IN_FLIGHT,
						 sizeof(VkSemaphore));
	*in_flight = (VkFence *)calloc(P_VULKAN_MAX_FRAMES_IN_FLIGHT,
				       sizeof(VkFence));
	*images_in_flight = (VkFence *)calloc(image_count, sizeof(VkFence));

	VkSemaphoreCreateInfo semaphore_info = {};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fence_info = {};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (i = 0; i < P_VULKAN_MAX_FRAMES_IN_FLIGHT; i++) {
		err = vkCreateSemaphore(device, &semaphore_info, NULL,
					*image_available);
		if (err != VK_SUCCESS)
			return err;

		err = vkCreateSemaphore(device, &semaphore_info, NULL,
					*render_finished);
		if (err != VK_SUCCESS)
			return err;

		err = vkCreateFence(device, &fence_info, NULL, *in_flight);
		if (err != VK_SUCCESS)
			return err;

		err = vkCreateFence(device, &fence_info, NULL,
				    *images_in_flight);
		if (err != VK_SUCCESS)
			return err;
	}

	return VK_SUCCESS;
}
