#include "purpl/vulkan/framebuffer.h"
using namespace purpl;

VkFramebuffer *purpl::create_framebuffers(VkDevice device,
					  VkImageView *image_views,
					  uint image_view_count,
					  VkExtent2D extent,
					  VkRenderPass render_pass)
{
	VkFramebuffer *buffers;
	uint i;

	/* Check the parameters we recieved */
	if (!image_views || !render_pass) {
		errno = EINVAL;
		return NULL;
	}

	/* Allocate a buffer */
	buffers = (VkFramebuffer *)calloc(image_view_count,
					  sizeof(VkFramebuffer));
	if (!buffers)
		return NULL;

	/* Fill out our create info */
	VkFramebufferCreateInfo framebuffer_info = {};
	framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_info.renderPass = render_pass;
	framebuffer_info.attachmentCount = 1;
	framebuffer_info.width = extent.width;
	framebuffer_info.height = extent.height;
	framebuffer_info.layers = 1;

	/* Create each framebuffer */
	for (i = 0; i < image_view_count; i++) {
		framebuffer_info.pAttachments = &image_views[i];

		if (vkCreateFramebuffer(device, &framebuffer_info, NULL, &buffers[i]) != VK_SUCCESS)
			return NULL;
	}

	/* Return our buffers */
	return buffers;
}
