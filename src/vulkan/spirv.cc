#include "purpl/vulkan/spirv.h"
using namespace purpl;

VkShaderModule purpl::create_shader_module(VkDevice device, const char *spirv,
					   uint len)
{
	VkShaderModule shader;

	VkShaderModuleCreateInfo module_create_info = {};
	module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	module_create_info.codeSize = len;
	module_create_info.pCode = (u32 *)spirv;

	if (vkCreateShaderModule(device, &module_create_info, NULL, &shader) !=
	    VK_SUCCESS)
		return NULL;

	return shader;
}
