#include "purpl/vulkan/physical_device.h"
using namespace purpl;

uint purpl::score_device(VkPhysicalDevice device)
{
	VkPhysicalDeviceProperties device_properties;
	VkPhysicalDeviceFeatures device_features;
	uint score = 0;

	/* Retrieve some info about our device, so we can score it */
	vkGetPhysicalDeviceProperties(device, &device_properties);
	vkGetPhysicalDeviceFeatures(device, &device_features);

	/* Discrete GPUs can't possibly be worse than iGPUs, so make sure that's reflected */
	if (device_properties.deviceType ==
	    VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 1000;

	/* We just want the max texture resolution and some other stuff, we don't need the rest */
	score += device_properties.limits.maxImageDimension2D;
	score += device_properties.limits.maxViewportDimensions[0] +
		 device_properties.limits.maxViewportDimensions[1];
	score += device_properties.limits.maxMemoryAllocationCount;
	score += device_properties.limits.maxComputeSharedMemorySize;

#ifndef NDEBUG
	printf("Device \'%s\' has a score of %d\n",
	       device_properties.deviceName,
	       score); /* Print the scores of the devices in debug mode */
#endif

	return score;
}

VkPhysicalDevice
purpl::locate_suitable_device(VkInstance inst, VkSurfaceKHR surface,
			      struct queue_family_indices *indices,
			      struct swapchain_details *details)
{
	uint i;
	uint device_count;
	VkPhysicalDevice *devices;
	uint best_score;
	VkPhysicalDevice best;

	/* Determine how many devices are present */
	vkEnumeratePhysicalDevices(inst, &device_count, NULL);
	if (!device_count)
		return NULL;

	/* Allocate a buffer for our device handles */
	devices = (VkPhysicalDevice *)calloc(device_count,
					     sizeof(VkPhysicalDevice));
	if (!devices) {
		errno = ENOMEM;
		return NULL;
	}

	/* Now get a list of devices */
	vkEnumeratePhysicalDevices(inst, &device_count, devices);

	*indices = find_queue_families(devices[0], surface);
	*details = get_swapchain_details(devices[0], surface);

	/* Check our first device */
	best_score = score_device(devices[0]);
	best = devices[0];

	/* Now find the best device */
	for (i = 1; i < device_count; i++) {
		uint last_score;

		/* Prevent a device without the necessary queue families from being chosen */
		*indices = find_queue_families(devices[i], surface);
		if (!indices->has_graphics_family ||
		    !indices->has_present_family)
			continue;

		/* Also prevent a device without needed swap chain feature support from being chosen */
		*details = get_swapchain_details(devices[i], surface);
		if (!details->formats || !details->present_modes)
			continue;

		/* Score the current device */
		last_score = score_device(devices[i]);

		/* If this is better than the last device score, make it the best score and remember which device it was */
		if (last_score > best_score) {
			best_score = last_score;
			best = devices[i];
		}
	}

#ifndef NDEBUG
	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(best, &properties);

	printf("Using device \'%s\'\n", properties.deviceName);
#endif

	/* Make sure this has a score above zero */
	if (!best_score)
		return NULL;

	/* best will now be the highest scoring device, so return it */
	return best;
}
