#pragma once

#ifndef PURPL_WIN32_VULKAN_INST_H
#define PURPL_WIN32_VULKAN_INST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifndef NDEBUG
#include "debug.h" /* Include our definitions and stuff for using validation layers when we're in debug mode */
#endif

#include "physical_device.h"
#include "macro.h"
#include "util.h"

#include <vulkan/vulkan.h>

#include "purpl/log.h"
#include "purpl/macro.h"
#include "purpl/types.h"

namespace purpl {
class P_EXPORT win32_vulkan_inst {
public:
	/*
	 * Sets up the instance.
	 * Defined in inst.cc
	 */
	win32_vulkan_inst(void);

	/*
	 * Cleans up the instance;
	 * Defined in inst.cc
	 */
	~win32_vulkan_inst(void);

private:
	/* Our Vulkan instance */
	VkInstance inst;

	/* Debug logger */
	VkDebugUtilsMessengerEXT debug_messenger;

	/* Extensions */
	VkExtensionProperties *exts;
	uint ext_count;
	
	/* Validation layers */
	char **validation_layers;

	/* A handle to our physical device */
	VkPhysicalDevice physical_device;

	/* A handle to our logical device */
	VkDevice device;
	
#ifndef NDEBUG
	/* Our debug logger */
	logger *debug_log;
#endif
};
}

#endif
