#include "purpl/vulkan/surface.h"
using namespace purpl;

VkSurfaceKHR purpl::create_surface(VkInstance instance, window *wnd)
{
	VkSurfaceKHR surface;

#ifdef _WIN32
	PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;

	vkCreateWin32SurfaceKHR =
		(PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
	if (!vkCreateWin32SurfaceKHR)
		return NULL;

	VkWin32SurfaceCreateInfoKHR surface_create_info{};
	surface_create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_create_info.hwnd = wnd->handle;
	surface_create_info.hinstance = GetModuleHandleA(NULL);

	if (vkCreateWin32SurfaceKHR(instance, &surface_create_info, NULL, &surface) != VK_SUCCESS)
		return NULL;
#elif __linux__
	PFN_vkCreateXlibSurfaceKHR vkCreateXlibSurfaceKHR;

	vkCreateXlibSurfaceKHR =
		(PFN_vkCreateXlibSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXlibSurfaceKHR");
	if (!vkCreateXlibSurfaceKHR)
		return NULL;
	
	VkXlibSurfaceCreateInfoKHR surface_create_info{};
	surface_create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surface_create_info.window = wnd->handle;
	surface_create_info.dpy = wnd->display;

	if (vkCreateXlibSurfaceKHR(instance, &surface_create_info, NULL, &surface) != VK_SUCCESS)
		return NULL;
#endif

	return surface;
}
