#pragma once

#ifndef PURPL_VULKAN_DEBUG_H
#define PURPL_VULKAN_DEBUG_H

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
/* The validation layers to use for debugging */
extern char enabled_layers[256][VK_MAX_EXTENSION_NAME_SIZE];

/* Checks the support for the validation layers requested. */
char **get_required_validation_layers(void);

/* Logs to debug.log. */
VKAPI_ATTR VkBool32 VKAPI_CALL
debug_msg(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	  VkDebugUtilsMessageTypeFlagsEXT message_type,
	  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
	  void *user_data);

/* Loads in the vkCreateDebugUtilsMessengerEXT function and calls it. */
VkResult create_debug_utils_messenger_ext(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *create_info,
	const VkAllocationCallbacks *allocator,
	VkDebugUtilsMessengerEXT *debug_messenger);

/* Loads in the vkDestroyDebugUtilsMessengerEXT function and calls it */
void destroy_debug_utils_messenger_ext(VkInstance instance,
				       VkDebugUtilsMessengerEXT debug_messenger,
				       const VkAllocationCallbacks *allocator);
}

#endif /* !PURPL_VULKAN_DEBUG_H */
