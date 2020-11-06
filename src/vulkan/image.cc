#include "purpl/vulkan/image.h"
using namespace purpl;

VkImageView *purpl::create_image_views(VkDevice device, VkImage *images,
				       uint image_count, VkFormat image_format)
{
	VkImageView *image_views;
	uint i;

	if (!images) {
		errno = EINVAL;
		return NULL;
	}

	/* Allocate a buffer */
	image_views = (VkImageView *)calloc(image_count, sizeof(VkImageView));
	if (!image_views) {
		errno = ENOMEM;
		return NULL;
	}

	/* Define the main info for our image views */
	VkImageViewCreateInfo image_view_create_info = {};
	image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	image_view_create_info.format = image_format;

	image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	image_view_create_info.subresourceRange.aspectMask =
		VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_create_info.subresourceRange.baseArrayLayer = 0;
	image_view_create_info.subresourceRange.baseMipLevel = 0;
	image_view_create_info.subresourceRange.layerCount = 1;
	image_view_create_info.subresourceRange.levelCount = 1;

	/* Iterate through all the image views and create them */
	for (i = 0; i < image_count; i++) {
		image_view_create_info.image = images[i];

		if (vkCreateImageView(device, &image_view_create_info, NULL,
				      &image_views[i]) != VK_SUCCESS)
			return NULL;
	}

	/* Return the buffer */
	return image_views;
}
