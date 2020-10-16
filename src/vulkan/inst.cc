#include "purpl/vulkan/inst.h"
using namespace purpl;

P_EXPORT purpl::vulkan_inst::vulkan_inst(window *wnd)
{
	uint i;
	char **required_exts;

	/* Ensure that we only go past this function when initialization succeeds */
	this->init_success = false;

#ifndef NDEBUG
	char **required_layers;

	/* Start our logger */
	this->debug_log = new logger(DEBUG, "debug.log");

	/* Check for validation layers if we're in debug mode */
	required_layers = get_required_validation_layers();

	VkDebugUtilsMessengerCreateInfoEXT debug_msngr_create_info = { 0 };
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
	create_info.enabledLayerCount = P_REQUIRED_VULKAN_LAYER_COUNT;
	create_info.ppEnabledLayerNames = required_layers;
#endif

	/* Instantiate our instance :) */
	if (vkCreateInstance(&create_info, NULL, &this->inst) != VK_SUCCESS)
		return;

		/* This has to happen after the instance. Whoops :) */
#ifndef NDEBUG
	if (create_debug_utils_messenger_ext(
		    this->inst, &debug_msngr_create_info, NULL,
		    &this->debug_messenger) != VK_SUCCESS)
		return;
#endif

	/* Create a surface */
	this->surface = create_surface(this->inst, wnd);
	if (!this->surface)
		return;

	/* Locate a device to use for rendering */
	this->physical_device = locate_suitable_device(
		this->inst, this->surface, &this->queue_indices,
		&this->swapchain_features);
	if (!this->physical_device)
		return;

	/* Create a logical device */
	this->device = create_logical_device(this->physical_device,
					     this->queue_indices,
					     &this->graphics_queue,
					     &this->present_queue);
	if (!this->device)
		return;

	/* Create a swap chain */
	this->swapchain = create_a_freaking_swap_chain(this->physical_device, this->device,
							this->surface,
							wnd->width, wnd->height,
							this->queue_indices);
	if (!this->swapchain)
		return;

	/* Avoid a memory leak */
	for (i = 0; i < P_REQUIRED_VULKAN_EXT_COUNT; i++)
		free(required_exts[i]);
	free(required_exts);

#ifndef NDEBUG
	for (i = 0; i < P_REQUIRED_VULKAN_LAYER_COUNT; i++)
		free(required_layers[i]);
	free(required_layers);
#endif

	/* Indicate that initialization succeeded, because execution won't reach here if that fails */
	this->init_success = true;
}

P_EXPORT purpl::vulkan_inst::~vulkan_inst(void)
{
	vkDestroySwapchainKHR(this->device, this->swapchain, NULL);
	vkDestroyDevice(this->device, NULL);
	vkDestroySurfaceKHR(this->inst, this->surface, NULL);

#ifndef NDEBUG
	destroy_debug_utils_messenger_ext(this->inst, this->debug_messenger,
					  NULL);
	this->debug_log->~logger();
#endif
	vkDestroyInstance(this->inst, NULL);
}
