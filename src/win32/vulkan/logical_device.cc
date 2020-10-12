#include "purpl/win32/vulkan/logical_device.h"
using namespace purpl;

VkDevice purpl::create_logical_device(VkPhysicalDevice physical_device,
				  struct queue_family_indices indices)
{
	float queue_priority;

	/* First of all, check whether we've been passed a struct with no queue families enabled */
	if (!indices.has_graphics_family) {
		errno = EINVAL;
		return NULL;
	}

	/* Request a graphics queue */
	VkDeviceQueueCreateInfo queue_create_info = {0};
	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.queueFamilyIndex = indices.graphics_family;
	queue_create_info.queueCount = 1;
	
	queue_priority = 1.0f;
	queue_create_info.pQueuePriorities = &queue_priority;
}
