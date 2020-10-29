#include "purpl/vulkan/swapchain.h"
using namespace purpl;

struct swapchain_details purpl::get_swapchain_details(VkPhysicalDevice device,
						      VkSurfaceKHR surface)
{
	struct swapchain_details details;

	/* Query the capabilities of the swap chain */
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
						  &details.capabilities);

	/* Now get information about supported formats */
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
					     &details.format_count, NULL);
	if (!details.format_count)
		return { details.capabilities, NULL, 0, NULL, 0 };

	details.formats = (VkSurfaceFormatKHR *)calloc(
		details.format_count, sizeof(VkSurfaceFormatKHR));
	if (!details.formats)
		return { details.capabilities, NULL, 0, NULL, 0 };

	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device, surface, &details.format_count, details.formats);

	/* Now get information about how many present modes there are */
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device, surface, &details.present_mode_count, NULL);
	if (!details.present_mode_count)
		return { details.capabilities, NULL, 0, NULL, 0 };

	details.present_modes = (VkPresentModeKHR *)calloc(
		details.present_mode_count, sizeof(VkSurfaceFormatKHR));
	if (!details.present_modes)
		return { details.capabilities, NULL, 0, NULL, 0 };

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
						  &details.present_mode_count,
						  details.present_modes);

	return details;
}

VkSurfaceFormatKHR *
purpl::choose_best_surface_format(VkSurfaceFormatKHR *available_formats,
				  uint format_count)
{
	uint i;

	if (!available_formats || !format_count) {
		errno = EINVAL;
		return NULL;
	}

	for (i = 0; i < format_count; i++) {
		if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
		    available_formats[i].colorSpace ==
			    VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return &available_formats[i];
	}

	return available_formats; /* Go with the first format if we don't find the 32-bit SRGB one, because it'll be good enough */
}

VkPresentModeKHR
purpl::choose_best_present_mode(VkPresentModeKHR *available_modes,
				uint mode_count)
{
	uint i;

	if (!available_modes || !mode_count)
		return VK_PRESENT_MODE_FIFO_KHR;

	for (i = 0; i < mode_count; i++) {
		if (available_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
			return available_modes[i];
	}

	/* Guaranteed by the spec to be available, so if we can't get triple buffer mode, we know we can return this */
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D purpl::choose_extent(VkSurfaceCapabilitiesKHR capabilities,
				uint width, uint height)
{
	VkExtent2D extent;

	/* Check if we have to do anything */
	if (capabilities.currentExtent.width != UINT32_MAX)
		return capabilities.currentExtent;

	/* Here we actually have to figure out what the resolution should be */
	extent = { width, height };

	/* Get the best size within the capabilities of the surface */
	extent.width =
		fmax(capabilities.minImageExtent.width,
		     fmin(capabilities.maxImageExtent.width, extent.width));
	extent.height =
		fmax(capabilities.minImageExtent.height,
		     fmin(capabilities.maxImageExtent.height, extent.height));

	return extent;
}

VkSwapchainKHR purpl::create_a_freaking_swap_chain(
	VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface,
	uint current_width, uint current_height,
	struct queue_family_indices indices, VkFormat *swapchain_format,
	VkExtent2D *swapchain_extent, VkImage **swapchain_images,
	uint *swapchain_image_count)
{
	struct swapchain_details details;
	VkSwapchainKHR swapchain;
	VkSurfaceFormatKHR *surface_format;
	VkPresentModeKHR present_mode;
	VkExtent2D extent;
	uint image_count;
	uint i;

	uint indices_arr[] = { indices.graphics_family,
			       indices.present_family };

	/* Avoid using invalid indices, and also invalid value-returning pointers */
	if (!indices.has_graphics_family || !indices.has_present_family ||
	    !swapchain_format || !swapchain_extent || !swapchain_images ||
	    !swapchain_image_count) {
		errno = EINVAL;
		return NULL;
	}

	/* Get information about the features available for swap chains */
	details = get_swapchain_details(physical_device, surface);
	if (!details.formats || !details.format_count)
		return NULL;

	/* No need to check the return values for these, the conditions are already checked or don't matter that much */
	surface_format = choose_best_surface_format(details.formats,
						    details.format_count);
	present_mode = choose_best_present_mode(details.present_modes,
						details.present_mode_count);

	/* Give the image format to the caller */
	*swapchain_format = surface_format->format;

	/* Determine the resolution our images will have */
	extent = choose_extent(details.capabilities, current_width,
			       current_height);

	/* Give the image extent to the caller */
	*swapchain_extent = extent;

	/* Now determine how many images our chain can hold */
	image_count = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0 &&
	    image_count > details.capabilities.maxImageCount)
		image_count = details.capabilities.maxImageCount;

	/* Same thing for the image capacity */
	*swapchain_image_count = image_count;

	/* Fill out our swap chain creation info structure */
	VkSwapchainCreateInfoKHR swapchain_create_info{};
	swapchain_create_info.sType =
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_create_info.surface = surface;
	swapchain_create_info.minImageCount = image_count;
	swapchain_create_info.imageFormat = surface_format->format;
	swapchain_create_info.imageColorSpace = surface_format->colorSpace;
	swapchain_create_info.imageExtent = extent;
	swapchain_create_info.imageArrayLayers = 1;
	swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	/* Check if the graphics and present families are the same, and fill in that part of the creation info accordingly */
	if (indices.graphics_family != indices.present_family) {
		swapchain_create_info.imageSharingMode =
			VK_SHARING_MODE_CONCURRENT;
		swapchain_create_info.queueFamilyIndexCount = 2;
		swapchain_create_info.pQueueFamilyIndices = indices_arr;
	} else {
		swapchain_create_info.imageSharingMode =
			VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount = 0;
		swapchain_create_info.pQueueFamilyIndices = NULL;
	}

	swapchain_create_info.preTransform =
		details.capabilities.currentTransform;
	swapchain_create_info.compositeAlpha =
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_create_info.presentMode = present_mode;
	swapchain_create_info.clipped =
		true; /* True is actually equivalent to 1 when assigned to an integer */
	swapchain_create_info.oldSwapchain = NULL;

	if (vkCreateSwapchainKHR(device, &swapchain_create_info, NULL,
				 &swapchain) != VK_SUCCESS)
		return NULL;

	/* Now we need to create a buffer for our swap chain's images and also retrieve said images */
	vkGetSwapchainImagesKHR(device, swapchain, &image_count, NULL);

	*swapchain_images = (VkImage *)calloc(image_count, sizeof(VkImage));
	if (!*swapchain_images)
		return NULL;

	vkGetSwapchainImagesKHR(device, swapchain, &image_count,
				*swapchain_images);

	return swapchain;
}
