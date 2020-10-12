#pragma once

#ifndef PURPL_WIN32_VULKAN_DEBUG_H
#define PURPL_WIN32_VULKAN_DEBUG_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
/* The validation layers to use for debugging */
const char enabled_layers[][VK_MAX_EXTENSION_NAME_SIZE] = { "VK_LAYER_KHRONOS_validation" };

/*
 * Checks the support for the validation layers requested.
 * Defined in debug.cc
 */
char **get_required_validation_layers(void);

/*
 * Logs to debug.log.
 * Defined in debug.cc
 */
VKAPI_ATTR VkBool32 VKAPI_CALL
debug_msg(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	  VkDebugUtilsMessageTypeFlagsEXT message_type,
	  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
	  void *user_data);

/*
 * Loads in the vkCreateDebugUtilsMessengerEXT function and calls it.
 * Defined in debug.cc
 */
VkResult create_debug_utils_messenger_ext(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *create_info,
	const VkAllocationCallbacks *allocator,
	VkDebugUtilsMessengerEXT *debug_messenger);

/*
 * Loads in the vkDestroyDebugUtilsMessengerEXT function and calls it;
 * Defined in debug.cc
 */
void destroy_debug_utils_messenger_ext(VkInstance instance,
				       VkDebugUtilsMessengerEXT debug_messenger,
				       const VkAllocationCallbacks *allocator);
}

#endif /* !PURPL_WIN32_VULKAN_DEBUG_H */
