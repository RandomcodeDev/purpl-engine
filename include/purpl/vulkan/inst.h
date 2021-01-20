#pragma once

#ifndef PURPL_VULKAN_INST_H
#define PURPL_VULKAN_INST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/log.h"
#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/window.h"

#include "command.h"

#ifndef NDEBUG
#include "debug.h" /* Include our definitions and stuff for using validation layers when we're in debug mode */
#endif

#include "framebuffer.h"
#include "image.h"
#include "logical_device.h"
#include "macro.h"
#include "pipeline.h"
#include "physical_device.h"
#include "surface.h"
#include "swapchain.h"
#include "util.h"

namespace purpl
{
extern char vert_path[260]; /* The vertex shader to use */
extern char frag_path[260]; /* The fragment shader to use */

/*
 * Recreates the swapchain (used when the window resizes and the like).
 * Defined in inst.cc
 */
VkSwapchainKHR recreate_swap_chain(
	VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface,
	uint new_width, uint new_height, queue_family_indices indices,
	VkFormat *swapchain_format, VkExtent2D *swapchain_extent,
	VkImage **swapchain_images, uint *image_count,
	VkSwapchainKHR *swapchain, VkImageView **image_views,
	VkRenderPass *render_pass, const char *vert_shader_path,
	const char *frag_shader_path, struct vert_info *verts,
	size_t vert_count, VkPipeline *pipeline,
	VkPipelineLayout *pipeline_layout, VkFramebuffer **framebuffers,
	VkCommandPool command_pool, VkCommandBuffer **command_buffers);

class P_EXPORT vulkan_inst {
    public:
	/* Used to indicate whether instance is alive */
	bool is_active;

	/*
	 * Initializes *everything* for the instance.
	 * Defined in inst.cc
	 */
	vulkan_inst(window *wnd, struct vert_info *verts, size_t vert_count);

	/*
	 * Does some stuff to present rendered images.
	 * Defined in inst.cc
	 */
	void update(window *wnd, struct vert_info *verts, size_t vert_count);

	/*
	 * Cleans up the instance;
	 * Defined in inst.cc
	 */
	~vulkan_inst(void);

    private:
	/* Our Vulkan instance */
	VkInstance inst;

	/* Debug logger */
	VkDebugUtilsMessengerEXT debug_messenger;

	/* Extensions */
	VkExtensionProperties *exts;
	uint ext_count; /* Number of extensions */

	/* Validation layers */
	char **validation_layers;

	/* A handle to our physical device */
	VkPhysicalDevice physical_device;

	/* A handle to our logical device */
	VkDevice device;

	/* Queue family related stuff */
	VkQueue graphics_queue; /* The graphics queue */
	VkQueue present_queue; /* The presentation queue */
	struct queue_family_indices
		queue_indices; /* Where each queue family resides */

	/* Our surface */
	VkSurfaceKHR surface;

	/* Swap chain stuff */
	struct swapchain_details
		swapchain_features; /* The features of the swap chain */
	VkSwapchainKHR swapchain; /* The actual swap chain */
	VkFormat swapchain_format; /* The format of the swap chain */
	VkExtent2D
		swapchain_extent; /* The resolution of the images in the swap chain */
	VkImage *swapchain_images; /* The images in the swap chain */
	uint swapchain_image_count; /* The number of images in the swap chain */
	VkImageView *
		swapchain_image_views; /* Image views of the images in the swap chain */

	/* Graphics pipelines and related stuff */
	VkPipeline main_pipeline;
	VkPipelineLayout main_pipeline_layout;
	VkRenderPass render_pass;

	/* The framebuffers for our images */
	VkFramebuffer *framebuffers;

	/* The command pool */
	VkCommandPool command_pool;

	/* Command buffers */
	VkCommandBuffer *command_buffers;

	/* Objects used for synchronization during rendering */
	VkSemaphore *image_avail_sem;
	VkSemaphore *render_finished_sem;
	VkFence *in_flight_fences;
	VkFence *images_in_flight;

#ifndef NDEBUG
	/* Our debug logger */
	logger *debug_log;
#endif
};
}

#endif
