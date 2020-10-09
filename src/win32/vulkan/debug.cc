#include "purpl/win32/vulkan/debug.h"
using namespace purpl;

bool purpl::check_validation_layer_support(void)
{
	uint i, j;
	u32 layer_count;

	/* Enumerate the available validation layers */
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);

	/* Allocate a buffer for the layer query */
	VkLayerProperties *layers = (VkLayerProperties *)calloc(
		layer_count, sizeof(VkLayerProperties));
	if (!layers)
		return false;

	vkEnumerateInstanceLayerProperties(&layer_count, layers);
	if (!layers)
		return false;


	/* Now we check if the layers we want are available */
	for (i = 0; i < P_ARRAYSIZE(enabled_layers); i++) {
		bool layer_found = false;

		for (j = 0; j < layer_count; j++) {
			if (strcmp(enabled_layers[j], layers[j].layerName) == 0) {
				layer_found = true; /* We found a match */
				break;
			}
		}

		return layer_found;
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL
purpl::debug_log(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		 VkDebugUtilsMessageTypeFlagsEXT message_type,
		 const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
		 void *user_data)
{
	if (message_type == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT ||
	    message_type == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		printf("validation message: %s: %s\n",
		       callback_data->pMessageIdName, callback_data->pMessage);

	return false;
}

VkResult purpl::create_debug_utils_messenger_ext(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *create_info,
	const VkAllocationCallbacks *allocator,
	VkDebugUtilsMessengerEXT *debug_messenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func =
		(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkCreateDebugUtilsMessengerEXT");

	if (func == NULL)
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	else
		return func(instance, create_info, allocator, debug_messenger);
}

void purpl::destroy_debug_utils_messenger_ext(

	VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
	const VkAllocationCallbacks *allocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func == NULL)
		return;
	else
		func(instance, debug_messenger, allocator);
}
