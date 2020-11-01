#include "purpl/vulkan/logical_device.h"
using namespace purpl;

VkDevice purpl::create_logical_device(VkPhysicalDevice physical_device,
				  struct queue_family_indices indices, VkQueue *graphics_queue_handle, VkQueue *present_queue_handle)
{
	float queue_priority;
	char **exts;
	VkDevice device;

	/* First of all, check whether we've been passed a struct with no queue families enabled */
	if (!indices.has_graphics_family || !indices.has_present_family) {
		errno = EINVAL;
		return NULL;
	}

	exts = check_required_device_exts_avail(physical_device);

	/* The priority for our queues */
	queue_priority = 1.0f;

	/* Request a graphics queue */
	VkDeviceQueueCreateInfo graphics_queue_create_info = {};
	graphics_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	graphics_queue_create_info.queueFamilyIndex = indices.graphics_family;
	graphics_queue_create_info.queueCount = 1;
	graphics_queue_create_info.pQueuePriorities = &queue_priority;

	/* And a present queue */
	VkDeviceQueueCreateInfo present_queue_create_info = {};
	present_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	present_queue_create_info.queueFamilyIndex = indices.present_family;
	present_queue_create_info.queueCount = 1;
	present_queue_create_info.pQueuePriorities = &queue_priority;

	VkPhysicalDeviceFeatures device_features{};

	/* Fill the create info struct */
	VkDeviceCreateInfo device_create_info = {};
	device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_create_info.pQueueCreateInfos = &graphics_queue_create_info;
	device_create_info.queueCreateInfoCount = 1;

	device_create_info.pEnabledFeatures = &device_features;

	device_create_info.ppEnabledExtensionNames = exts;
	device_create_info.enabledExtensionCount = 1;

	device_create_info.enabledLayerCount = 0;

	if (vkCreateDevice(physical_device, &device_create_info, NULL, &device) != VK_SUCCESS)
		return NULL;

	/* Now we need to clean up and return */
	vkGetDeviceQueue(device, indices.graphics_family, 0, graphics_queue_handle);
	vkGetDeviceQueue(device, indices.present_family, 0, present_queue_handle);

	for (uint i = 0; i < P_REQUIRED_VULKAN_DEVICE_EXT_COUNT; i++) {
		if (exts[i])
			free(exts[i]);
	}

	free(exts);

	return device;
}
