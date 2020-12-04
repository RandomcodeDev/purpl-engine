#include "purpl/vulkan/buffer.h"
using namespace purpl;

VkVertexInputBindingDescription purpl::get_vert_input_binding_desc(struct vert_info vert)
{
	VkVertexInputBindingDescription binding_description = {};
	binding_description.binding = 0;
	binding_description.stride = sizeof(vert);
	binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return binding_description;
}

VkVertexInputAttributeDescription *purpl::get_vert_input_attrib_descs(struct vert_info vert)
{
	VkVertexInputAttributeDescription *attrib_descs = {};

	return attrib_descs;
}
