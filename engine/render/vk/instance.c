#include "vk.h"

PCSTR RequiredExtensions[] = {
//    VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
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

#ifndef PURPL_SWITCH
    CONST char *LayerName = "VK_LAYER_KHRONOS_validation";

    CONST VkBool32 SettingValidateCore = VK_TRUE;
    CONST VkBool32 SettingValidateSync = VK_TRUE;
    CONST VkBool32 SettingThreadSafety = VK_TRUE;
    CONST char *SettingDebugAction[] = {"VK_DBG_LAYER_ACTION_BREAK"};
    CONST char *SettingReportFlags[] = {"info", "warn", "perf", "error", "debug"};
    CONST VkBool32 SettingEnableMessageLimit = VK_TRUE;
    CONST int32_t SettingDuplicateMessageLimit = 3;
    CONST char *SettingEnables[] = {"VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT",
                                    "VALIDATION_CHECK_ENABLE_VENDOR_SPECIFIC_ALL"};
    CONST VkBool32 SettingPrintfToStdout = VK_FALSE;

    CONST VkLayerSettingEXT LayerSettings[] = {
        {LayerName, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &SettingValidateCore},
        // TODO: fix the WRITE_AFTER_WRITE hazard that shows up when this is enabled
        //{LayerName, "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &SettingValidateSync},
        {LayerName, "thread_safety", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &SettingThreadSafety},
        {LayerName, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, SettingDebugAction},
        {LayerName, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT, PURPL_ARRAYSIZE(SettingReportFlags),
         SettingReportFlags},
        {LayerName, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &SettingEnableMessageLimit},
        {LayerName, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &SettingDuplicateMessageLimit},
        //{LayerName, "enables", VK_LAYER_SETTING_TYPE_STRING_EXT, PURPL_ARRAYSIZE(SettingEnables), SettingEnables},
        {LayerName, "printf_to_stdout", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &SettingPrintfToStdout}};
#endif

    VkInstanceCreateInfo CreateInformation = {0};
    CreateInformation.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

    VkApplicationInfo ApplicationInformation = {0};
    ApplicationInformation.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ApplicationInformation.pApplicationName = PURPL_NAME;
    ApplicationInformation.pEngineName = "Purpl Engine";
    ApplicationInformation.engineVersion = PURPL_VERSION;
    ApplicationInformation.apiVersion = VK_API_VERSION_1_3;
    CreateInformation.pApplicationInfo = &ApplicationInformation;

    LogDebug("Required instance extensions:");
    CreateInformation.enabledExtensionCount = PURPL_ARRAYSIZE(RequiredExtensions);
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
    DebugMessengerCreateInformation.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugMessengerCreateInformation.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugMessengerCreateInformation.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                                  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    DebugMessengerCreateInformation.pfnUserCallback = VlkDebugCallback;

#ifndef PURPL_SWITCH
    VkLayerSettingsCreateInfoEXT LayerSettingsCreateInformation = {0};
    LayerSettingsCreateInformation.sType = VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT;
    LayerSettingsCreateInformation.pSettings = LayerSettings;
    LayerSettingsCreateInformation.settingCount = PURPL_ARRAYSIZE(LayerSettings);
    LayerSettingsCreateInformation.pNext = &DebugMessengerCreateInformation;

    CreateInformation.pNext = &LayerSettingsCreateInformation;
#endif
#endif

    LogTrace("Calling vkCreateInstance");
    Result = vkCreateInstance(&CreateInformation, VlkGetAllocationCallbacks(), &VlkData.Instance);
    if (Result == VK_ERROR_LAYER_NOT_PRESENT)
    {
        LogDebug("Validation layers not found, retrying without them");
        CreateInformation.enabledLayerCount = 0;
        Result = vkCreateInstance(&CreateInformation, VlkGetAllocationCallbacks(), &VlkData.Instance);
    }
    if (Result != VK_SUCCESS)
    {
        CmnError("Failed to create Vulkan instance: %s", VlkGetResultString(Result));
    }

    LogDebug("Loading Vulkan functions");
    volkLoadInstance(VlkData.Instance);

#if defined PURPL_VULKAN_DEBUG && !defined PURPL_SWITCH
    LogDebug("Creating real debug messenger");
    vkCreateDebugUtilsMessengerEXT(VlkData.Instance, &DebugMessengerCreateInformation, VlkGetAllocationCallbacks(),
                                   &VlkData.DebugMessenger);
#endif
}
