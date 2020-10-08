#include "purpl/win32/vulkan/inst.h"
using namespace purpl;

P_EXPORT purpl::win32_vulkan_inst::win32_vulkan_inst(void)
{
	uint i;

	/* Check for validation layers if we're in debug mode */
#ifndef NDEBUG
	if (!check_validation_layer_support())
		return;
#endif

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
	create_info.pNext = NULL;

	/* Get the available extensions */
	this->exts = get_vulkan_exts(&this->ext_count);

	for (i = 0; i < this->ext_count; i++)
		printf("%s\n", this->exts[i].extensionName);

	create_info.enabledExtensionCount = P_REQUIRED_VULKAN_EXT_COUNT;
	create_info.ppEnabledExtensionNames = check_required_exts_avail();
	
	/* Enable validation layers if in debug mode */
#ifndef NDEBUG
	create_info.enabledLayerCount = P_ARRAYSIZE(enabled_layers);
	create_info.ppEnabledLayerNames = (char **)enabled_layers;

	VkDebugUtilsMessengerCreateInfoEXT debug_msngr_create_info{};
	debug_msngr_create_info.sType =
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_msngr_create_info.messageSeverity =
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debug_msngr_create_info.messageType =
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debug_msngr_create_info.pfnUserCallback = debug_log;
	debug_msngr_create_info.pUserData = NULL;

	if (create_debug_utils_messenger_ext(this->inst, &debug_msngr_create_info, NULL, &this->debug_messenger) != VK_SUCCESS)
		return;

	create_info.pNext = &debug_msngr_create_info;
#endif

	if (vkCreateInstance(&create_info, NULL, &this->inst) != VK_SUCCESS)
		return;
}

P_EXPORT purpl::win32_vulkan_inst::~win32_vulkan_inst(void)
{
	destroy_debug_utils_messenger_ext(this->inst, this->debug_messenger, NULL);

	vkDestroyInstance(this->inst, NULL);
}
