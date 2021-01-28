#pragma once

#ifndef PURPL_VULKAN_SWAPCHAIN_H
#define PURPL_VULKAN_SWAPCHAIN_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <math.h>

#include "purpl/macro.h"

#include <vulkan/vulkan.h>

#include "purpl/types.h"
#include "purpl/util.h"

#include "macro.h"
#include "queuefamily.h"
#include "surface.h"

namespace purpl
{
/* A structure to hold information about swap chain features */
struct swapchain_details {
	VkSurfaceCapabilitiesKHR capabilities;
	VkSurfaceFormatKHR *formats;
	uint format_count;
	VkPresentModeKHR *present_modes;
	uint present_mode_count;
};

/*
 * Gets information about supported swap chain features.
 * Defined in swapchain.cc
 */
struct swapchain_details get_swapchain_details(VkPhysicalDevice device,
					       VkSurfaceKHR surface);

/*
 * Chooses the best format from the buffer passed for available_formats.
 * Defined in swapchain.cc
 */
VkSurfaceFormatKHR *
choose_best_surface_format(VkSurfaceFormatKHR *available_formats,
			   uint format_count);

/*
 * Chooses the best presentation mode from the buffer passed for available_modes.
 * Defined in swapchain.cc
 */
VkPresentModeKHR choose_best_present_mode(VkPresentModeKHR *available_modes,
					  uint mode_count);

/*
 * Chooses the extent for the swap chain.
 * Defined in swapchain.cc
 */
VkExtent2D choose_extent(VkSurfaceCapabilitiesKHR capabilities, uint width,
			 uint height);

/*
 * Creates a swap chain.
 * Defined in swapchain.cc
 */
VkSwapchainKHR create_a_freaking_swap_chain(
	VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface,
	uint current_width, uint current_height,
	struct queue_family_indices indices, VkFormat *swapchain_format,
	VkExtent2D *swapchain_extent, VkImage **swapchain_images,
	uint *image_count);
}
#endif /* !PURPL_VULKAN_SWAPCHAIN_H */
