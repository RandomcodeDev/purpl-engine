#pragma once

#ifndef PURPL_VULKAN_INST_H
#define PURPL_VULKAN_INST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifndef NDEBUG
#include "debug.h" /* Include our definitions and stuff for using validation layers when we're in debug mode */
#endif

#include "logical_device.h"
#include "macro.h"
#include "physical_device.h"
#include "surface.h"
#include "swapchain.h"
#include "util.h"

#include <vulkan/vulkan.h>

#include "purpl/log.h"
#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/window.h"

namespace purpl {
class P_EXPORT vulkan_inst {
public:
	/* Used to indicate whether initialization failed */
	bool init_success;

	/*
	 * Initializes *everything* for the instance.
	 * Defined in inst.cc
	 */
	vulkan_inst(window *wnd);

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
	struct queue_family_indices queue_indices; /* Where each queue family resides */

	/* Our surface */
	VkSurfaceKHR surface;

	/* Swap chain stuff */
	struct swapchain_details swapchain_features; /* The features of the swap chain */
	VkSwapchainKHR swapchain; /* The actual swap chain */

#ifndef NDEBUG
	/* Our debug logger */
	logger *debug_log;
#endif
};
}

#endif
