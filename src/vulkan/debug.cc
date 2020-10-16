#include "purpl/vulkan/debug.h"
#include "purpl/vulkan/inst.h"
using namespace purpl;

char **purpl::get_required_validation_layers(void)
{
	uint i, j;
	u32 layer_count;
	VkLayerProperties *layers;
	char **required_layer_names;

	/* Enumerate the available validation layers */
	vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	if (!layer_count)
		return NULL;

	/* Allocate a buffer for the layer query */
	layers = (VkLayerProperties *)calloc(
		layer_count, sizeof(VkLayerProperties));
	if (!layers)
		return NULL;

	vkEnumerateInstanceLayerProperties(&layer_count, layers);
	if (!layers)
		return NULL;

	/* Allocate another buffer for the names of the layers we need */
	required_layer_names =
		(char **)calloc(P_ARRAYSIZE(enabled_layers), sizeof(char *));
	if (!required_layer_names)
		return NULL;

	/* Now allocate memory for each of the layer names */
	for (i = 0; i < P_ARRAYSIZE(enabled_layers); i++) {
		required_layer_names[i] =
			(char *)calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
		if (!required_layer_names[i])
			return NULL;
	}

	/* Now we check if the layers we want are available */
	for (i = 0; i < P_ARRAYSIZE(enabled_layers); i++) {
		for (j = 0; j < layer_count; j++) {
			if (strcmp(enabled_layers[i], layers[j].layerName) ==
			    0) {
				strcpy(required_layer_names[i], layers[j].layerName);
				break; /* Avoid extra iterations */
			}
		}
	}

	return required_layer_names;
}

VKAPI_ATTR VkBool32 VKAPI_CALL
purpl::debug_msg(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		 VkDebugUtilsMessageTypeFlagsEXT message_type,
		 const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
		 void *user_data)
{
	((logger *)user_data)
		->write(((logger *)user_data)->logindex, DEBUG, P_FILENAME,
			__LINE__, "validation layer: %s: %s",
			callback_data->pMessageIdName, callback_data->pMessage);

	return false;
}

VkResult purpl::create_debug_utils_messenger_ext(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT *create_info,
	const VkAllocationCallbacks *allocator,
	VkDebugUtilsMessengerEXT *debug_messenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != NULL)
		return func(instance, create_info, allocator, debug_messenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void purpl::destroy_debug_utils_messenger_ext(
	VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger,
	const VkAllocationCallbacks *allocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func =
		(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
			instance, "vkDestroyDebugUtilsMessengerEXT");

	if (func != NULL)
		func(instance, debug_messenger, allocator);
}
