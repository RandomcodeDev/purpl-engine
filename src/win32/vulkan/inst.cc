#include <purpl/win32/vulkan/inst.h>
using namespace purpl;

P_EXPORT purpl::win32_vulkan_inst::win32_vulkan_inst(void)
{
	VkApplicationInfo info{};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Purpl Engine";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &info;

	char **ext_names = NULL;

	ext_names = check_required_exts_avail();

	create_info.enabledExtensionCount = P_REQUIRED_VULKAN_EXT_COUNT;
	create_info.ppEnabledExtensionNames = ext_names;
	create_info.enabledLayerCount = 0;

	if (vkCreateInstance(&create_info, NULL, &this->inst))
		return;
}

P_EXPORT purpl::win32_vulkan_inst::~win32_vulkan_inst(void)
{
	vkDestroyInstance(this->inst, NULL);
}
