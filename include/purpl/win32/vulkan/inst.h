#pragma once

#ifndef PURPL_WIN32_VULKAN_INST_H
#define PURPL_WIN32_VULKAN_INST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#ifndef NDEBUG
#include "validation.h" /* Include our definitions and stuff for using validation layers when we're in debug mode */
#endif

#include "macro.h"
#include "util.h"

#include <vulkan/vulkan.h>

#include "purpl/macro.h"

namespace purpl {
class P_EXPORT win32_vulkan_inst {
public:
	/*
	 * Sets up the instance.
	 * Defined in inst.cc
	 */
	win32_vulkan_inst(void);

	/*
	 * Frees the various resources used by the instance;
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

	/* The device we use for rendering */
	VkPhysicalDevice device;

	/* The number of devices we have in the system */
	uint device_count;
};
}

#endif
