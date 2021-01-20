#include "purpl/vulkan/buffer.h"
using namespace purpl;

VkVertexInputBindingDescription
purpl::get_vert_input_binding_desc(void)
{
	VkVertexInputBindingDescription binding_desc = {};
	binding_desc.binding = 0;
	binding_desc.stride = sizeof(vert);
	binding_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return binding_desc;
}

VkVertexInputAttributeDescription *
purpl::get_vert_input_attrib_descs(void)
{
	VkVertexInputAttributeDescription *attrib_descs = {};

	/* Allocate the attribute descriptions */
	attrib_descs = (VkVertexInputAttributeDescription *)calloc(
		2, sizeof(VkVertexInputAttributeDescription));
	if (!attrib_descs)
		return NULL;

	/* Vertices */
	attrib_descs[0].binding = 0;
	attrib_descs[0].location = 0;
	attrib_descs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attrib_descs[0].offset = offsetof(struct vert_info, pos);

	/* Colors */
	attrib_descs[1].binding = 0;
	attrib_descs[1].location = 0;
	attrib_descs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	attrib_descs[1].offset = offsetof(struct vert_info, colour);

	return attrib_descs;
}
