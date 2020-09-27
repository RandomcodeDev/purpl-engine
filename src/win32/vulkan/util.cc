#include "purpl/win32/vulkan/util.h"
using namespace purpl;

const char *purpl::get_vulkan_err_str(VkResult result)
{
	switch (result) {
	case VK_SUCCESS:
		return "Success";
	case VK_NOT_READY:
		return "A fence or query has not yet completed";
	case VK_TIMEOUT:
		return "A wait operation has not completed in the specified time";
	case VK_EVENT_SET:
		return "An event is signaled";
	case VK_EVENT_RESET:
		return "An event is unsignaled";
	case VK_INCOMPLETE:
		return "A return array was too small for the result";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "A host memory allocation has failed";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "A device memory allocation has failed";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "Initialization of an object could not be completed for implementation-specific reasons";
	case VK_ERROR_DEVICE_LOST:
		return "The logical or physical device has been lost";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "Mapping of a memory object has failed";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "A requested layer is not present or could not be loaded";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "A requested extension is not supported";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "A requested feature is not supported";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "The requested version of Vulkan is not supported by the driver or is otherwise incompatible";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "Too many objects of the type have already been created";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "A requested format is not supported on this device";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "A surface is no longer available";
	case VK_SUBOPTIMAL_KHR:
		return "A swapchain no longer matches the surface properties exactly, but can still be used";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "A surface has changed in such a way that it is no longer compatible with the swapchain";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "The display used by a swapchain does not use the same presentable image layout";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "The requested window is already connected to a VkSurfaceKHR, or to some other non-Vulkan API";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "A validation layer found an error";
	default:
		return "ERROR: UNKNOWN VULKAN ERROR";
	}
}

VkExtensionProperties *purpl::get_vulkan_exts(uint *count)
{
	VkExtensionProperties *exts;
	uint ext_count;
	int err;

	if (!count) { /* Caller passed an invalid pointer, set errno and return */
		errno = EINVAL;
		return NULL;
	}

	err = vkEnumerateInstanceExtensionProperties(NULL, &ext_count, NULL); /* Ask how many extensions there are */

	if (err) { /* An error occured, return NULL to indicate this */
		*count = err;
		return NULL;
	}

	exts = (VkExtensionProperties *)calloc(
		ext_count, sizeof(VkExtensionProperties)); /* We know how many extensions we have now, allocate a buffer */
	if (!exts) /* Usually this doesn't happen */
		return NULL;

	err = vkEnumerateInstanceExtensionProperties(NULL, &ext_count, exts); /* Now fill in the buffer */
	if (err) {
		*count = err;
		return NULL;
	}

	*count = ext_count; /* Set the pointer's memory to the number of extensions we found */
	return exts; /* Return the buffer */
}

char **purpl::check_required_exts_avail(void)
{
	int i;
	uint ext_count;
	VkExtensionProperties *exts;

	char **names;

	bool have_surface = NULL;
	bool have_win32_surface = NULL;

	exts = get_vulkan_exts(&ext_count);

	if (!exts)
		return NULL;

	for (i = 0; i < ext_count; i++) {
		if (strcmp(exts[i].extensionName, "VK_KHR_surface") == 0)
			have_surface = true; /* We have a match */
		if (strcmp(exts[i].extensionName, "VK_KHR_win32_surface") == 0)
			have_win32_surface = true; /* We have another match */
	}

	names = (char **)calloc(P_REQUIRED_VULKAN_EXT_COUNT,
			       sizeof(char *)); /* Allocate an array of pointers */
	if (!names)
		return NULL;

	/* Now allocate P_REQUIRED_VULKAN_EXT_COUNT names */
	for (i = 0; i < P_REQUIRED_VULKAN_EXT_COUNT; i++) {
		names[i] = (char *)calloc(VK_MAX_EXTENSION_NAME_SIZE, sizeof(char));
		if (!names[i])
			return NULL;
	}

	/* Specify the names of the extensions for the caller */
	strcpy(names[0], "VK_KHR_surface");
	strcpy(names[1], "VK_KHR_win32_surface");

	if (have_surface && have_win32_surface)
		return names;

	return NULL; /* One or both are missing in this case, fail */
}
