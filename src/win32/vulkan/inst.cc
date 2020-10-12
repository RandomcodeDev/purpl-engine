#include "purpl/win32/vulkan/inst.h"
using namespace purpl;

P_EXPORT purpl::win32_vulkan_inst::win32_vulkan_inst(void)
{
	uint i;
	char **required_exts;
	
#ifndef NDEBUG
	char **required_layers;

	/* Start our logger */
	this->debug_log = new logger(DEBUG, "debug.log");
	
	/* Check for validation layers if we're in debug mode */
	required_layers = get_required_validation_layers();

	VkDebugUtilsMessengerCreateInfoEXT debug_msngr_create_info = {0};
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
	debug_msngr_create_info.pfnUserCallback = debug_msg;
	debug_msngr_create_info.pUserData = this->debug_log;
#endif

	VkApplicationInfo info = {0};
	info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	info.pApplicationName = "Purpl Engine";
	info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	info.pEngineName = "No Engine";
	info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	info.apiVersion = VK_API_VERSION_1_2;

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &info;
#ifndef NDEBUG
	create_info.pNext = &debug_msngr_create_info;
#else
	create_info.pNext = NULL;
#endif

	/* Get the available extensions */
	this->exts = get_vulkan_exts(&this->ext_count);

	required_exts = check_required_exts_avail();
	if (!required_exts)
		return;

	create_info.enabledExtensionCount = P_REQUIRED_VULKAN_EXT_COUNT;
	create_info.ppEnabledExtensionNames = required_exts;

	/* Enable validation layers if in debug mode */
#ifndef NDEBUG
	create_info.enabledLayerCount = P_BUFSIZE(required_layers, char *);
	create_info.ppEnabledLayerNames = required_layers;
#endif

	/* Instantiate our instance */
	if (vkCreateInstance(&create_info, NULL, &this->inst) != VK_SUCCESS)
		return;

	/* This has to happen after the instance. Whoops :) */
#ifndef NDEBUG
	if (create_debug_utils_messenger_ext(
		    this->inst, &debug_msngr_create_info, NULL,
		    &this->debug_messenger) != VK_SUCCESS)
		return;
#endif

	/* Initialize our device handle */
	this->device = NULL;

	/* Locate a device to use for rendering */
	this->physical_device = locate_suitable_device(this->inst);

	/* Avoid a memory leak */
	for (i = 0; i < P_BUFSIZE(required_exts, char *); i++)
		free(required_exts[i]);
	free(required_exts);

#ifndef NDEBUG
	for (i = 0; i < P_BUFSIZE(required_layers, char *); i++)
		free(required_layers[i]);
	free(required_layers);
#endif
}

P_EXPORT purpl::win32_vulkan_inst::~win32_vulkan_inst(void)
{
#ifndef NDEBUG
	destroy_debug_utils_messenger_ext(this->inst, this->debug_messenger, NULL);
	this->debug_log->~logger();
#endif

	vkDestroyInstance(this->inst, NULL);
}
