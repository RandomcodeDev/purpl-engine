#include "purpl/vulkan/inst.h"
using namespace purpl;

P_EXPORT purpl::vulkan_inst::vulkan_inst(window *wnd)
{
	uint i;
	char **required_exts;

	/* Ensure that we only go past this function when initialization succeeds */
	this->is_active = false;

#ifndef NDEBUG
	char **required_layers;

	/* Start our logger */
	this->debug_log = new logger(NULL, DEBUG, "vulkan.log");

	/* Check for validation layers if we're in debug mode */
	required_layers = get_required_validation_layers();

	VkDebugUtilsMessengerCreateInfoEXT debug_msngr_create_info = {};
	debug_msngr_create_info.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_msngr_create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_msngr_create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_msngr_create_info.pfnUserCallback = debug_msg;
	debug_msngr_create_info.pUserData = this->debug_log;
#endif

	VkApplicationInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Purpl Engine";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &info;
#ifndef NDEBUG
	create_info.pNext = &debug_msngr_create_info;
#else
	create_info.pNext = NULL;
#endif

	/* Get the available extensions */
	this->exts = get_vulkan_exts(&this->ext_count);

	required_exts = check_required_exts_avail();
	if (!required_exts)
		return;

	create_info.enabledExtensionCount = P_REQUIRED_VULKAN_EXT_COUNT;
	create_info.ppEnabledExtensionNames = required_exts;

	/* Enable validation layers if in debug mode */
#ifndef NDEBUG
	create_info.enabledLayerCount = P_REQUIRED_VULKAN_LAYER_COUNT;
	create_info.ppEnabledLayerNames = required_layers;
#endif

	/* Instantiate our instance :) */
	if (vkCreateInstance(&create_info, NULL, &this->inst) != VK_SUCCESS)
		return;

		/* This has to happen after the instance. Whoops :) */
#ifndef NDEBUG
	if (create_debug_utils_messenger_ext(
		    this->inst, &debug_msngr_create_info, NULL,
		    &this->debug_messenger) != VK_SUCCESS)
		return;
#endif

	/* Create a surface */
	this->surface = create_surface(this->inst, wnd);
	if (!this->surface)
		return;

	/* Locate a device to use for rendering */
	this->physical_device = locate_suitable_device(
		this->inst, this->surface, &this->queue_indices,
		&this->swapchain_features);
	if (!this->physical_device)
		return;

	/* Create a logical device */
	this->device = create_logical_device(this->physical_device,
					     this->queue_indices,
					     &this->graphics_queue,
					     &this->present_queue);
	if (!this->device)
		return;

	/* Create a swap chain */
	this->swapchain = create_a_freaking_swap_chain(
		this->physical_device, this->device, this->surface, wnd->width,
		wnd->height, this->queue_indices, &this->swapchain_format,
		&this->swapchain_extent, &this->swapchain_images,
		&this->swapchain_image_count);
	if (!this->swapchain)
		return;

	/* Create image views for the images */
	this->swapchain_image_views = create_image_views(
		this->swapchain_images, this->swapchain_image_count,
		this->swapchain_format, this->device);
	if (!this->swapchain_image_views)
		return;

	/* Create a render pass */
	this->render_pass =
		create_render_pass(this->device, this->swapchain_format);
	if (!this->render_pass)
		return;

	/* Now make a graphics pipeline */
	this->main_pipeline = create_graphics_pipeline(
		this->device, this->swapchain_extent, this->render_pass,
		vert_path, frag_path, &this->main_pipeline_layout);
	if (!this->main_pipeline || !this->main_pipeline_layout)
		return;

	/* Create framebuffers to render to */
	this->framebuffers =
		create_framebuffers(this->device, this->swapchain_image_views,
				    this->swapchain_image_count,
				    this->swapchain_extent, this->render_pass);
	if (!this->framebuffers)
		return;

	/* Create our command pool */
	this->command_pool =
		create_command_pool(this->device, this->queue_indices);
	if (!this->command_pool)
		return;

	/* Allocate our command buffers */
	this->command_buffers = allocate_command_buffers(
		this->device, this->command_pool, this->swapchain_image_count,
		this->render_pass, this->framebuffers, this->swapchain_extent,
		this->main_pipeline);
	if (!this->command_buffers)
		return;

	if (create_sync_objects(this->device, &this->image_avail_sem,
			      &this->render_finished_sem, &this->in_flight_fences, &this->images_in_flight, swapchain_image_count) != VK_SUCCESS)
		return;

	/* Avoid a memory leak */
	for (i = 0; i < P_REQUIRED_VULKAN_EXT_COUNT; i++)
		free(required_exts[i]);
	free(required_exts);

#ifndef NDEBUG
	for (i = 0; i < P_REQUIRED_VULKAN_LAYER_COUNT; i++)
		free(required_layers[i]);
	free(required_layers);
#endif

	/* Indicate that initialization succeeded, because execution won't reach here if it fails */
	this->is_active = true;
}

void P_EXPORT purpl::vulkan_inst::update(window *wnd)
{
	uint image_index;
	uint current_frame = 0;

	VkPipelineStageFlags wait_stages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	if (!wnd->should_close) {
		/* Wait for our fences to complete */
		vkWaitForFences(this->device, 1, &this->in_flight_fences[current_frame], true, UINT64_MAX);

		/* Acquire the next image */
		vkAcquireNextImageKHR(this->device, this->swapchain, UINT64_MAX,
				      this->image_avail_sem[current_frame], NULL,
				      &image_index);

		/* See whether this image is in use */
		if (this->images_in_flight[image_index])
			vkWaitForFences(this->device, 1, &images_in_flight[image_index], true, UINT64_MAX);

		/* Now mark the current image as in use */
		this->images_in_flight[image_index] =
			this->in_flight_fences[current_frame];

		/* Now prepare to submit the command buffers */
		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &this->image_avail_sem[current_frame];
		submit_info.pWaitDstStageMask = wait_stages;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers =
			&this->command_buffers[image_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &this->render_finished_sem[current_frame];

		/* Reset our fences */
		vkResetFences(this->device, 1,
			      &this->in_flight_fences[current_frame]);

		/* Submit the command buffer */
		vkQueueSubmit(this->graphics_queue, 1, &submit_info, this->in_flight_fences[current_frame]);

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &this->render_finished_sem[current_frame];
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &this->swapchain;
		present_info.pImageIndices = &image_index;
		present_info.pResults = NULL;

		/* Present the image */
		vkQueuePresentKHR(this->present_queue, &present_info);

		/* Increment the frame */
		current_frame = (current_frame + 1) % P_VULKAN_MAX_FRAMES_IN_FLIGHT;
	}

	vkDeviceWaitIdle(this->device);
}

P_EXPORT purpl::vulkan_inst::~vulkan_inst(void)
{
	uint i;

	for (i = 0; i < P_VULKAN_MAX_FRAMES_IN_FLIGHT; i++) {
		if (this->render_finished_sem[i])
			vkDestroySemaphore(this->device,
					   this->render_finished_sem[i], NULL);
		if (this->image_avail_sem[i])
			vkDestroySemaphore(this->device,
					   this->image_avail_sem[i], NULL);
		if (this->in_flight_fences[i])
			vkDestroyFence(this->device, this->in_flight_fences[i], NULL);
	}

	if (this->command_pool)
		vkDestroyCommandPool(this->device, this->command_pool, NULL);

	for (i = 0; i < this->swapchain_image_count; i++) {
		if (this->framebuffers[i])
			vkDestroyFramebuffer(this->device,
					     this->framebuffers[i], NULL);
	}

	if (this->main_pipeline)
		vkDestroyPipeline(this->device, this->main_pipeline, NULL);
	if (this->main_pipeline_layout)
		vkDestroyPipelineLayout(this->device,
					this->main_pipeline_layout, NULL);
	if (this->render_pass)
		vkDestroyRenderPass(this->device, this->render_pass, NULL);

	for (i = 0; i < this->swapchain_image_count; i++) {
		if (this->swapchain_image_views[i])
			vkDestroyImageView(this->device,
					   this->swapchain_image_views[i],
					   NULL);
	}

	if (this->swapchain)
		vkDestroySwapchainKHR(this->device, this->swapchain, NULL);
	if (this->device)
		vkDestroyDevice(this->device, NULL);
	if (this->surface)
		vkDestroySurfaceKHR(this->inst, this->surface, NULL);

#ifndef NDEBUG
	if (this->debug_messenger)
		destroy_debug_utils_messenger_ext(this->inst,
						  this->debug_messenger, NULL);
#endif
	if (this->inst)
		vkDestroyInstance(this->inst, NULL);

#ifndef NDEBUG
	delete this->debug_log;
#endif
}
