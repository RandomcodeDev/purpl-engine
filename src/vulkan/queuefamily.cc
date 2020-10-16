#include "purpl/vulkan/queuefamily.h"
using namespace purpl;

struct queue_family_indices purpl::find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	VkQueueFamilyProperties *queue_families;
	struct queue_family_indices indices{};
	uint queue_family_count = 0;
	uint i;

	/* Get the number of devices */
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);
	if (!queue_family_count) {
		indices.graphics_family = 0;
		indices.has_graphics_family = false;

		indices.present_family = 0;
		indices.has_present_family = false;
	}

	/* Allocate a buffer */
	queue_families =
		(VkQueueFamilyProperties *)calloc(queue_family_count, sizeof(VkQueueFamilyProperties));
	if (!queue_families) { /* Check our buffer */
		indices.graphics_family = 0;
		indices.has_graphics_family = false;

		indices.present_family = 0;
		indices.has_present_family = false;
	}

	/* Now get the queue families */
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count,
						 queue_families);

	/* Check for the ones we need */
	for (i = 0; i < queue_family_count && !indices.has_graphics_family && !indices.has_present_family; i++) {
		if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
			indices.has_graphics_family = true;
		}

		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &indices.has_present_family);
		if (indices.has_present_family) {
			indices.present_family = i;
			indices.has_present_family = true;
		}
	}

	return indices;
}
