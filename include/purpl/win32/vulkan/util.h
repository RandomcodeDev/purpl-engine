#pragma once

#ifndef PURPL_WIN32_VULKAN_UTIL_H
#define PURPL_WIN32_VULKAN_UTIL_H

#include <stdio.h> /* For debugging */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"

#include "macro.h"

namespace purpl
{
/*
 * Copied from GLFW, specifically vulkan.c, line 165. GLFW authors, if you don't like
 * this use of your code I'll take it out and write my own version, just let me know.
 * Defined in util.cc
 */
const char *get_vulkan_err_str(VkResult result);

/*
 * Gets all the available Vulkan extensions, and how many there are.
 * In the event of an error, count or errno will hold the error code and NULL will be returned.
 * Defined in util.cc
 */
VkExtensionProperties *get_vulkan_exts(uint *count);

/*
 * Checks if the required extensions are present (VK_KHR_surface and VK_KHR_win32_surface)
 * If they aren't, returns false.
 * Defined in util.cc
 */
char **check_required_exts_avail(void);

/*
 * Logs messages from the validation layers. For debugging, so we don't care about having a logger instance.
 * Defined in util.cc
 */
VKAPI_ATTR VkBool32 VKAPI_CALL
debug_log(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
	  VkDebugUtilsMessageTypeFlagsEXT message_type,
	  const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
	  void *user_data);

/*
 * Loads in the vkCreateDebugUtilsMessengerEXT function and calls it.
 * Defined in util.cc
 */
VkResult create_debug_utils_messenger_ext(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *create_info,
	const VkAllocationCallbacks *allocator,
	VkDebugUtilsMessengerEXT *debug_messenger);

/*
 * Loads in the vkDestroyDebugUtilsMessengerEXT function and calls it;
 * Defined in util.cc
 */
void destroy_debug_utils_messenger_ext(
	VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
	const VkAllocationCallbacks *allocator);
}

#endif
