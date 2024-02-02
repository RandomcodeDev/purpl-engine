#include "vk.h"

PCSTR RequiredExtensions[] = {
    VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef PURPL_WIN32
    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined PURPL_SWITCH
    VK_NN_VI_SURFACE_EXTENSION_NAME,
#elif defined PURPL_UNIX
    VK_KHR_XCB_SURFACE_EXTENSION_NAME,
    VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#endif
#ifdef PURPL_VULKAN_DEBUG
    VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
};

PCSTR RequiredLayers[] = {
#ifdef PURPL_SWITCH
    "VK_LAYER_NN_vi_swapchain",
#endif
#ifdef PURPL_VULKAN_DEBUG
    "VK_LAYER_KHRONOS_validation",
#endif
    NULL};

VOID VlkCreateInstance(VOID)
{
    VkResult Result;
    SIZE_T i;

    LogDebug("Creating Vulkan instance");

    VkInstanceCreateInfo CreateInformation = {0};
    CreateInformation.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkApplicationInfo ApplicationInformation = {0};
    ApplicationInformation.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ApplicationInformation.pApplicationName = GAME_NAME;
    ApplicationInformation.pEngineName = "Purpl Engine";
    ApplicationInformation.engineVersion = GAME_VERSION;
    ApplicationInformation.apiVersion = VK_API_VERSION_1_3;
    CreateInformation.pApplicationInfo = &ApplicationInformation;

    LogDebug("Required instance extensions:");
    CreateInformation.enabledExtensionCount =
        PURPL_ARRAYSIZE(RequiredExtensions);
    CreateInformation.ppEnabledExtensionNames = RequiredExtensions;
    for (i = 0; i < CreateInformation.enabledExtensionCount; i++)
    {
        LogDebug("\t%s", CreateInformation.ppEnabledExtensionNames[i]);
    }

    LogDebug("Required layers:");
    CreateInformation.enabledLayerCount = PURPL_ARRAYSIZE(RequiredLayers) - 1;
    CreateInformation.ppEnabledLayerNames = RequiredLayers;
    for (i = 0; i < CreateInformation.enabledLayerCount; i++)
    {
        LogDebug("\t%s", CreateInformation.ppEnabledLayerNames[i]);
    }

#ifdef PURPL_VULKAN_DEBUG
    VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInformation = {0};
    DebugMessengerCreateInformation.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugMessengerCreateInformation.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugMessengerCreateInformation.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    DebugMessengerCreateInformation.pfnUserCallback = VlkDebugCallback;
    CreateInformation.pNext = &DebugMessengerCreateInformation;
#endif

    LogTrace("Calling vkCreateInstance");
    Result = vkCreateInstance(&CreateInformation, VlkGetAllocationCallbacks(),
                              &VlkData.Instance);
    if (Result == VK_ERROR_LAYER_NOT_PRESENT)
    {
        LogDebug("Validation layers not found, retrying without them");
        CreateInformation.enabledLayerCount = 0;
        Result = vkCreateInstance(
            &CreateInformation, VlkGetAllocationCallbacks(), &VlkData.Instance);
    }
    if (Result != VK_SUCCESS)
    {
        CmnError("Failed to create Vulkan instance: VkResult %d", Result);
    }

    LogDebug("Loading Vulkan functions");
    volkLoadInstance(VlkData.Instance);
}
