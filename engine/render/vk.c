/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    vk.c

Abstract:

    This module implements the Vulkan backend.

--*/

#include "vk.h"

//
// Allocated buffer
//

typedef struct VULKAN_BUFFER
{
    VkBuffer Buffer;
    VmaAllocation Allocation;
    VkDeviceSize Size;
} VULKAN_BUFFER, *PVULKAN_BUFFER;

//
// Model data
//

typedef struct VULKAN_MODEL_DATA
{
    VULKAN_BUFFER VertexBuffer;
    VULKAN_BUFFER IndexBuffer;
    VkDescriptorSet DescriptorSet;
    UINT8 LastFrameUsed;
} VULKAN_MODEL_DATA, *PVULKAN_MODEL_DATA;

//
// Texture data
//

typedef struct VULKAN_TEXTURE_DATA
{
    VkImage Image;
    VmaAllocation Allocation;
    VkImageView ImageView;
} VULKAN_TEXTURE_DATA, *PVULKAN_TEXTURE_DATA;

//
// Vulkan data
//

//
// Required instance extensions
//

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

//
// Required device extensions
//

PCSTR RequiredDeviceExtensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_EXT_ROBUSTNESS_2_EXTENSION_NAME
};

//
// Required validation layers (only in debug mode)
//

PCSTR RequiredValidationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};

//
// Instance
//

static VkInstance Instance;

//
// Surface
//

static VkSurfaceKHR Surface;

//
// Device stuff
//

//
// Device information
//

typedef struct VULKAN_GPU_INFO
{
    VkPhysicalDevice Device;

    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceMemoryProperties MemoryProperties;

    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VkSurfaceFormatKHR* SurfaceFormats;

    VkPresentModeKHR* PresentModes;

    VkQueueFamilyProperties* QueueFamilyProperties;
    UINT32 GraphicsFamilyIndex;
    UINT32 PresentFamilyIndex;

    VkExtensionProperties* ExtensionProperties;

    VkPhysicalDeviceFeatures SupportedFeatures;

    BOOLEAN Usable;
} VULKAN_GPU_INFO, *PVULKAN_GPU_INFO;

static PVULKAN_GPU_INFO Gpus;
static PVULKAN_GPU_INFO Gpu;
static UINT32 GpuIndex;
static VkDevice Device;

//
// VMA stuff
//

static VmaAllocator Allocator;

//
// Queues
//

static VkQueue GraphicsQueue;
static VkQueue PresentQueue;

//
// Command and synchronization stuff
//

static VkCommandPool CommandPool;
static VkCommandPool TransferCommandPool;
static VkCommandBuffer CommandBuffers[VULKAN_FRAME_COUNT];
static VkFence CommandBufferFences[VULKAN_FRAME_COUNT];
static VkSemaphore AcquireSemaphores[VULKAN_FRAME_COUNT];
static VkSemaphore RenderCompleteSemaphores[VULKAN_FRAME_COUNT];

//
// Swap chain stuff
//

static VkSwapchainKHR SwapChain;
static VkSurfaceFormatKHR SurfaceFormat;
static VkPresentModeKHR PresentMode;
static VkExtent2D SwapChainExtent;
static VkImage* SwapChainImages;
static VkImageView SwapChainImageViews[VULKAN_FRAME_COUNT];
static UINT32 SwapChainIndex;

//
// Render target stuff
//

static VkFormat DepthFormat;
static VkImage DepthImage;
static VmaAllocation DepthImageAllocation;
static VkImageView DepthView;

//
// Rendering stuff
//

static VkRenderPass RenderPass;
static VkFramebuffer Framebuffers[VULKAN_FRAME_COUNT];

//
// Descriptor things
//

static VkDescriptorPool DescriptorPool;
static VkDescriptorSetLayout DescriptorSetLayout;

//
// Shared pipeline information
//
static VkPipelineLayout PipelineLayout;
static VkPipelineCache PipelineCache;

//
// Frame index
//

static UINT8 FrameIndex;

//
// Whether Vulkan is initialized
//

static BOOLEAN VulkanInitialized;

//
// Whether the swapchain was recreated
//

static BOOLEAN Resized;

//
// Shader stuff
//

static VULKAN_BUFFER UniformBuffers[VULKAN_FRAME_COUNT];
static PVOID UniformBufferAddresses[VULKAN_FRAME_COUNT];

//
// Sampler
//

static VkSampler Sampler;

//
// Get the name of a result
//

static
PCSTR
GetResultString(
    VkResult Result
    )
/*++

Routine Description:

    Gets a human-readable version of a VkResult.

Arguments:

    Result - The result to get the string for.

Return Value:

    A string describing the result.

--*/
{
    // Stolen from https://github.com/libsdl-org/SDL/blob/a4c6b38fef2ff046e38072200b096c03389bfa28/src/video/SDL_vulkan_utils.c#L29
    switch (Result)
    {
    case VK_SUCCESS:
        return "VK_SUCCESS";
    case VK_NOT_READY:
        return "VK_NOT_READY";
    case VK_TIMEOUT:
        return "VK_TIMEOUT";
    case VK_EVENT_SET:
        return "VK_EVENT_SET";
    case VK_EVENT_RESET:
        return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
        return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
        return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
        return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
        return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
        return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
        return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
        return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
        return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
        return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
        return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
        return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
        return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
        return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
        return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
        return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
        return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
        return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR:
        return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
        return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
        return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
        return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
        return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
        return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:
        return "VK_ERROR_INVALID_SHADER_NV";
#if VK_HEADER_VERSION >= 135 && VK_HEADER_VERSION < 162
    case VK_ERROR_INCOMPATIBLE_VERSION_KHR:
        return "VK_ERROR_INCOMPATIBLE_VERSION_KHR";
#endif
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
        return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT:
        return "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
        return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:
        return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
        return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
        return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
        return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
        return "VK_PIPELINE_COMPILE_REQUIRED_EXT";
    default:
        return "VK_UNKNOWN";
        break;
    }
}

//
// Check if something failed
//

#define VULKAN_CHECK(Call, ...) \
    do { \
        VkResult Result_ = (Call); \
        if ( Result_ != VK_SUCCESS __VA_ARGS__ ) \
        { \
            CmnError("Vulkan call " #Call " failed: %s (VkResult %d)", GetResultString(Result_), Result_); \
        } \
    } while(0)

static
VOID
ImGuiCheckVkResult(
    VkResult Result
    )
{
    if ( Result != VK_SUCCESS )
    {
        CmnError("ImGui Vulkan call failed: %s (VkResult %d)", GetResultString(Result), Result);
    }
}

static
VOID
SetObjectName(
    _In_ PVOID Object,
    _In_ VkObjectType ObjectType,
    _In_ _Printf_format_string_ PCSTR Name,
    ...
    )
{
    //UNREFERENCED_PARAMETER(Object);
    //UNREFERENCED_PARAMETER(ObjectType);
    //UNREFERENCED_PARAMETER(Name);
    // Seems to crash a lot
#ifdef PURPL_VULKAN_DEBUG
    if ( vkSetDebugUtilsObjectNameEXT && Device )
    {
        va_list Arguments;

        LogTrace("Setting object name of type %u object 0x%llX to %s", ObjectType, (UINT64)Object, Name);

        VkDebugUtilsObjectNameInfoEXT NameInformation = {0};
        NameInformation.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
        NameInformation.objectType = ObjectType;
        NameInformation.objectHandle = (UINT64)Object;

        va_start(
            Arguments,
            Name
            );
        NameInformation.pObjectName = CmnFormatTempStringVarArgs(
            Name,
            Arguments
            );
        va_end(Arguments);

        vkSetDebugUtilsObjectNameEXT(
            Device,
            &NameInformation
            );
    }
#endif
}

static
VkBool32
VKAPI_CALL
DebugCallback(
    _In_ VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    _In_ VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
    _In_ CONST VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    _In_opt_ PVOID UserData
    )
/*++

Routine Description:

    This routine logs Vulkan messages.

Arguments:

    MessageSeverity - The severity of the message.

    MessageTypes - The types of the message.

    CallbackData - Information about the callback.

    UserData - User data, unused.

Return Value:

    TRUE - Always returned.

--*/
{
    CHAR Location[128];
    LOG_LEVEL Level;

    UNREFERENCED_PARAMETER(UserData);

    switch ( MessageSeverity )
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        Level = LogLevelDebug;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        Level = LogLevelInfo;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        Level = LogLevelWarning;
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        Level = LogLevelError;
        break;
    default:
        Level = LogLevelInfo;
        break;
    }

    // Put the message types as the source file, using the address of the function as the line
    snprintf(
        Location,
        PURPL_ARRAYSIZE(Location),
        "VULKAN %s%s%sMESSAGE",
        (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) ? "GENERAL " : "",
        (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) ? "PERFORMANCE " : "",
        (MessageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) ? "VALIDATION " : ""
        );
    LogMessage(
        Level,
        Location,
        (UINT64)PlatGetReturnAddress(),
        true,
        "%s",
        CallbackData->pMessage
        );

    return TRUE;
}

static
VOID
CreateInstance(
    VOID
    )
/*++

Routine Description:

    Creates the Vulkan instance and debug messenger.

Arguments:

    None.

Return Value:

    None.

--*/
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
    CreateInformation.enabledExtensionCount = PURPL_ARRAYSIZE(RequiredExtensions);
    CreateInformation.ppEnabledExtensionNames = RequiredExtensions;
    for ( i = 0; i < CreateInformation.enabledExtensionCount; i++ )
    {
        LogDebug("\t%s", CreateInformation.ppEnabledExtensionNames[i]);
    }

    LogDebug("Required device extensions:");
    for ( i = 0; i < PURPL_ARRAYSIZE(RequiredDeviceExtensions); i++ )
    {
        LogDebug("\t%s", RequiredDeviceExtensions[i]);
    }

#ifdef PURPL_VULKAN_DEBUG
    LogDebug("Required validation layers:");
    CreateInformation.enabledLayerCount = PURPL_ARRAYSIZE(RequiredValidationLayers);
    CreateInformation.ppEnabledLayerNames = RequiredValidationLayers;
    for ( i = 0; i < CreateInformation.enabledLayerCount; i++ )
    {
        LogDebug("\t%s", CreateInformation.ppEnabledLayerNames[i]);
    }

    VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInformation = {0};
    DebugMessengerCreateInformation.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    DebugMessengerCreateInformation.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    DebugMessengerCreateInformation.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    DebugMessengerCreateInformation.pfnUserCallback = DebugCallback;
    CreateInformation.pNext = &DebugMessengerCreateInformation;
#endif

    LogTrace("Calling vkCreateInstance");
    Result = vkCreateInstance(
        &CreateInformation,
        NULL,
        &Instance
        );
    if ( Result == VK_ERROR_LAYER_NOT_PRESENT )
    {
        LogDebug("Validation layers not found, retrying without them");
        CreateInformation.enabledLayerCount = 0;
        Result = vkCreateInstance(
            &CreateInformation,
            NULL,
            &Instance
            );
    }
    if ( Result != VK_SUCCESS )
    {
        CmnError("Failed to create Vulkan instance: VkResult %d", Result);
    }

#ifndef PURPL_SWITCH
    LogDebug("Loading Vulkan functions");
    volkLoadInstance(Instance);
#endif
}

static
VOID
CreateSurface(
    VOID
    )
/*++

Routine Description:

    Creates a surface.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Surface = PlatCreateVulkanSurface(
        Instance,
        NULL
        );
}

static
VOID
EnumeratePhysicalDevices(
    VOID
    )
/*++

Routine Description:

    Populates the list of physical devices and gets information about them.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 DeviceCount;
    VkPhysicalDevice* Devices = NULL;
    UINT32 UsableCount;
    VkResult Result;
    UINT32 i;
    UINT32 j;

    LogDebug("Enumerating devices");
    VULKAN_CHECK(vkEnumeratePhysicalDevices(
        Instance,
        &DeviceCount,
        NULL
        ));

    LogDebug("Getting %d device handle(s)", DeviceCount);
    stbds_arrsetlen(Devices, DeviceCount);
    VULKAN_CHECK(vkEnumeratePhysicalDevices(
        Instance,
        &DeviceCount,
        Devices
        ));

    LogDebug("Initializing GPU info list");
    stbds_arrsetlen(
        Gpus,
        DeviceCount
        );

    memset(
        Gpus,
        0,
        stbds_arrlenu(Gpus) * sizeof(VULKAN_GPU_INFO)
        );

    UsableCount = 0;
    for ( i = 0; i < stbds_arrlenu(Devices); i++ )
    {
        PVULKAN_GPU_INFO CurrentGpu = &Gpus[i];
        CurrentGpu->Device = Devices[i];
        CurrentGpu->Usable = TRUE; // Assume this until it's disproven

        LogDebug("Getting information for device %u", i + 1);

        LogTrace("Getting queue family information");
        UINT32 QueueCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(
            CurrentGpu->Device,
            &QueueCount,
            NULL
            );
        if ( QueueCount < 1 )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no queue families", i + 1);
            continue;
        }

        stbds_arrsetlen(
            CurrentGpu->QueueFamilyProperties,
            QueueCount
            );
        vkGetPhysicalDeviceQueueFamilyProperties(
            CurrentGpu->Device,
            &QueueCount,
            CurrentGpu->QueueFamilyProperties
            );
        if ( QueueCount < 1 )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no queue families", i + 1);
            continue;
        }

        CurrentGpu->GraphicsFamilyIndex = UINT32_MAX;
        CurrentGpu->PresentFamilyIndex = UINT32_MAX;
        for ( j = 0; j < stbds_arrlenu(CurrentGpu->QueueFamilyProperties); j++ )
        {
            VkQueueFamilyProperties* Properties = &CurrentGpu->QueueFamilyProperties[j];

            if ( Properties->queueCount < 1 )
            {
                continue;
            }

            if ( Properties->queueFlags & VK_QUEUE_GRAPHICS_BIT )
            {
                CurrentGpu->GraphicsFamilyIndex = j;
            }

            LogTrace("Checking surface support for queue family %u", j);
            VkBool32 PresentSupported = FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(
                CurrentGpu->Device,
                j,
                Surface,
                &PresentSupported
                );
            if ( PresentSupported )
            {
                CurrentGpu->PresentFamilyIndex = j;
            }

            if ( CurrentGpu->GraphicsFamilyIndex < UINT32_MAX &&
                 CurrentGpu->PresentFamilyIndex < UINT32_MAX )
            {
                break;
            }
        }

        if ( CurrentGpu->GraphicsFamilyIndex == UINT32_MAX ||
             CurrentGpu->PresentFamilyIndex == UINT32_MAX )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get all required queue indices for device %u (graphics %u, present %u)", i + 1, CurrentGpu->GraphicsFamilyIndex, CurrentGpu->PresentFamilyIndex);
            continue;
        }

        LogTrace("Getting extensions");
        UINT32 ExtensionCount = 0;
        Result = vkEnumerateDeviceExtensionProperties(
            CurrentGpu->Device,
            NULL,
            &ExtensionCount,
            NULL
            );
        if ( Result != VK_SUCCESS )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get extensions for device %u: VkResult %d", i + 1, Result);
            continue;
        }
        if ( ExtensionCount < 1 )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no extensions when there are %zu required", i + 1, PURPL_ARRAYSIZE(RequiredDeviceExtensions));
            continue;
        }

        stbds_arrsetlen(
            CurrentGpu->ExtensionProperties,
            ExtensionCount
            );
        Result = vkEnumerateDeviceExtensionProperties(
            CurrentGpu->Device,
            NULL,
            &ExtensionCount,
            CurrentGpu->ExtensionProperties
            );
        if ( Result != VK_SUCCESS )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get extensions for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting surface capabilities");
        Result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            CurrentGpu->Device,
            Surface,
            &CurrentGpu->SurfaceCapabilities
            );
        if ( Result != VK_SUCCESS )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get surface capabilities for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting surface formats");
        UINT32 FormatCount = 0;
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            CurrentGpu->Device,
            Surface,
            &FormatCount,
            NULL
            );
        if ( Result != VK_SUCCESS )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get surface formats for device %u: VkResult %d", i + 1, Result);
            continue;
        }
        if ( FormatCount < 1 )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no surface formats", i + 1);
            continue;
        }

        stbds_arrsetlen(CurrentGpu->SurfaceFormats,
            FormatCount
            );
        Result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            CurrentGpu->Device,
            Surface,
            &FormatCount,
            CurrentGpu->SurfaceFormats
            );
        if ( Result != VK_SUCCESS )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get surface formats for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting present modes");
        UINT32 ModeCount = 0;
        Result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            CurrentGpu->Device,
            Surface,
            &ModeCount,
            NULL
            );
        if ( Result != VK_SUCCESS )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get present modes for device %u: VkResult %d", i + 1, Result);
            continue;
        }
        if ( FormatCount < 1 )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it has no present modes", i + 1);
            continue;
        }

        stbds_arrsetlen(CurrentGpu->PresentModes,
            ModeCount
            );
        Result = vkGetPhysicalDeviceSurfacePresentModesKHR(
            CurrentGpu->Device,
            Surface,
            &ModeCount,
            CurrentGpu->PresentModes
            );
        if ( Result != VK_SUCCESS )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Failed to get present modes for device %u: VkResult %d", i + 1, Result);
            continue;
        }

        LogTrace("Getting features");
        vkGetPhysicalDeviceFeatures(
            CurrentGpu->Device,
            &CurrentGpu->SupportedFeatures
            );
        if ( !CurrentGpu->SupportedFeatures.samplerAnisotropy )
        {
            CurrentGpu->Usable = FALSE;
            LogError("Ignoring device %u because it doesn't support sampler anisotropy", i + 1);
            continue;
        }

        LogTrace("Getting memory properties");
        vkGetPhysicalDeviceMemoryProperties(
            CurrentGpu->Device,
            &CurrentGpu->MemoryProperties
            );

        LogTrace("Getting properties");
        vkGetPhysicalDeviceProperties(
            CurrentGpu->Device,
            &CurrentGpu->Properties
            );

        UsableCount++;
        if ( CurrentGpu->Usable )
        {
            Gpu = CurrentGpu;
            GpuIndex = i;
            LogDebug("Selected device %zu %s [%04x:%04x]", GpuIndex, Gpu->Properties.deviceName, Gpu->Properties.vendorID, Gpu->Properties.deviceID);
            break;
        }
    }

    stbds_arrfree(Devices);

    LogDebug("Got information for %d device(s) (of which %d are usable)", stbds_arrlenu(Gpus), UsableCount);
    if ( UsableCount < 1 )
    {
        CmnError("Could not find any usable Vulkan devices");
    }
}

static
VOID
CreateLogicalDevice(
    VOID
    )
/*++

Routine Description:

    Creates the logical device and queues from the selected physical device.

Arguments:

    None.

Return Value:

    None.

--*/
{
    VkDeviceCreateInfo DeviceCreateInformation = {0};
    VkDeviceQueueCreateInfo QueueCreateInfos[2] = {0};
    FLOAT QueuePriority = 1.0f;

    LogDebug("Creating logical device");

    QueueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueCreateInfos[0].queueFamilyIndex = Gpu->GraphicsFamilyIndex;
    QueueCreateInfos[0].pQueuePriorities = &QueuePriority;
    QueueCreateInfos[0].queueCount = 1;
    DeviceCreateInformation.queueCreateInfoCount = 1;
    if ( Gpu->GraphicsFamilyIndex != Gpu->PresentFamilyIndex )
    {
        QueueCreateInfos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        QueueCreateInfos[1].queueFamilyIndex = Gpu->GraphicsFamilyIndex;
        QueueCreateInfos[1].pQueuePriorities = &QueuePriority;
        QueueCreateInfos[1].queueCount = 1;
        DeviceCreateInformation.queueCreateInfoCount = 2;
    }

    VkPhysicalDeviceFeatures DeviceFeatures = {0};
    DeviceFeatures.samplerAnisotropy = TRUE;

    VkPhysicalDeviceDescriptorIndexingFeatures DescriptorIndexingFeatures = {0};
    DescriptorIndexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
    DescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = TRUE;
    DescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = TRUE;

    VkPhysicalDeviceRobustness2FeaturesEXT DeviceRobustness2Features = {0};
    DeviceRobustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
    DeviceRobustness2Features.nullDescriptor = TRUE;
    DeviceRobustness2Features.pNext = &DescriptorIndexingFeatures;

    DeviceCreateInformation.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceCreateInformation.pQueueCreateInfos = QueueCreateInfos;
    DeviceCreateInformation.pEnabledFeatures = &DeviceFeatures;
    DeviceCreateInformation.ppEnabledExtensionNames = RequiredDeviceExtensions;
    DeviceCreateInformation.enabledExtensionCount = PURPL_ARRAYSIZE(RequiredDeviceExtensions);
    DeviceCreateInformation.pNext = &DeviceRobustness2Features;

    LogTrace("Calling vkCreateDevice");
    VULKAN_CHECK(vkCreateDevice(
        Gpu->Device,
        &DeviceCreateInformation,
        NULL,
        &Device
        ));
    SetObjectName(
        Device,
        VK_OBJECT_TYPE_DEVICE,
        "Logical device"
        );

    LogDebug("Retrieving queues");

    vkGetDeviceQueue(
        Device,
        Gpu->GraphicsFamilyIndex,
        0,
        &GraphicsQueue
        );
    SetObjectName(
        GraphicsQueue,
        VK_OBJECT_TYPE_QUEUE,
        "Graphics queue"
        );
    vkGetDeviceQueue(
        Device,
        Gpu->PresentFamilyIndex,
        0,
        &PresentQueue
        );
    SetObjectName(
        PresentQueue,
        VK_OBJECT_TYPE_QUEUE,
        "Presentation queue"
        );

    SetObjectName(
        Instance,
        VK_OBJECT_TYPE_INSTANCE,
        "Instance"
        );

    SetObjectName(
        Surface,
        VK_OBJECT_TYPE_SURFACE_KHR,
        "Surface"
        );
}

static
VOID
CreateSemaphores(
    VOID
    )
/*++

Routine Description:

    Creates semaphores for frame rendering and presentation.

Arguments:

    None.

Return Value:

    None.

--*/
{
    VkSemaphoreCreateInfo SemaphoreCreateInformation = {0};
    SemaphoreCreateInformation.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    UINT32 i;

    LogDebug("Creating %d semaphores", VULKAN_FRAME_COUNT * 2);

    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        VULKAN_CHECK(vkCreateSemaphore(
            Device,
            &SemaphoreCreateInformation,
            NULL,
            &AcquireSemaphores[i]
            ));
        SetObjectName(
            AcquireSemaphores[i],
            VK_OBJECT_TYPE_SEMAPHORE,
            "Acquisition semaphore %u",
            i
            );
        VULKAN_CHECK(vkCreateSemaphore(
            Device,
            &SemaphoreCreateInformation,
            NULL,
            &RenderCompleteSemaphores[i]
            ));
        SetObjectName(
            RenderCompleteSemaphores[i],
            VK_OBJECT_TYPE_SEMAPHORE,
            "Render completion semaphore %u",
            i
            );
    }
}

static
VOID
CreateCommandPools(
    VOID
    )
/*++

Routine Description:

    Creates the command pools.

Arguments:

    None.

Return Value:

    None.

--*/
{
    VkCommandPoolCreateInfo CommandPoolCreateInformation = {0};

    LogDebug("Creating command pools");

    CommandPoolCreateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CommandPoolCreateInformation.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CommandPoolCreateInformation.queueFamilyIndex = Gpu->GraphicsFamilyIndex;

    LogTrace("Creating primary command pool");
    VULKAN_CHECK(vkCreateCommandPool(
        Device,
        &CommandPoolCreateInformation,
        NULL,
        &CommandPool
        ));
    SetObjectName(
        CommandPool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "Command pool"
        );

    CommandPoolCreateInformation.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    LogTrace("Creating transfer command pool");
    VULKAN_CHECK(vkCreateCommandPool(
        Device,
        &CommandPoolCreateInformation,
        NULL,
        &TransferCommandPool
        ));
    SetObjectName(
        TransferCommandPool,
        VK_OBJECT_TYPE_COMMAND_POOL,
        "Transfer command pool"
        );
}

static
VOID
AllocateCommandBuffers(
    VOID
    )
/*++

Routine Description:

    Allocates the command buffers.

Arguments:

    None.

Return Value:

    None.

--*/
{
    VkCommandBufferAllocateInfo CommandBufferAllocateInformation = {0};
    UINT32 i;

    LogDebug("Allocating command buffers");

    CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInformation.commandPool = CommandPool;
    CommandBufferAllocateInformation.commandBufferCount = VULKAN_FRAME_COUNT;

    VULKAN_CHECK(vkAllocateCommandBuffers(
        Device,
        &CommandBufferAllocateInformation,
        CommandBuffers
        ));
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        SetObjectName(
            CommandBuffers[i],
            VK_OBJECT_TYPE_COMMAND_BUFFER,
            "Command buffer %u",
            i
            );
    }

    LogDebug("Creating command buffer fences");

    VkFenceCreateInfo FenceCreateInformation = {0};
    FenceCreateInformation.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceCreateInformation.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        VULKAN_CHECK(vkCreateFence(
            Device,
            &FenceCreateInformation,
            NULL,
            &CommandBufferFences[i]
            ));
        SetObjectName(
            CommandBufferFences[i],
            VK_OBJECT_TYPE_FENCE,
            "Command buffer fence %u",
            i
            );
    }
}

static
VOID
CreateAllocator(
    VOID
)
/*++

Routine Description:

Creates a VMA allocator.

Arguments:

None.

Return Value:

None.

--*/
{
    VmaAllocatorCreateInfo AllocatorCreateInformation = {0};
    VmaVulkanFunctions VulkanFunctions = {0};

    LogDebug("Creating Vulkan allocator");

    AllocatorCreateInformation.instance = Instance;
    AllocatorCreateInformation.physicalDevice = Gpu->Device;
    AllocatorCreateInformation.device = Device;
    AllocatorCreateInformation.vulkanApiVersion = VK_API_VERSION_1_3;

    VulkanFunctions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
    VulkanFunctions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;
    VulkanFunctions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
    VulkanFunctions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
    VulkanFunctions.vkAllocateMemory = vkAllocateMemory;
    VulkanFunctions.vkFreeMemory = vkFreeMemory;
    VulkanFunctions.vkMapMemory = vkMapMemory;
    VulkanFunctions.vkUnmapMemory = vkUnmapMemory;
    VulkanFunctions.vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges;
    VulkanFunctions.vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges;
    VulkanFunctions.vkBindBufferMemory = vkBindBufferMemory;
    VulkanFunctions.vkBindImageMemory = vkBindImageMemory;
    VulkanFunctions.vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements;
    VulkanFunctions.vkGetImageMemoryRequirements = vkGetImageMemoryRequirements;
    VulkanFunctions.vkCreateBuffer = vkCreateBuffer;
    VulkanFunctions.vkDestroyBuffer = vkDestroyBuffer;
    VulkanFunctions.vkCreateImage = vkCreateImage;
    VulkanFunctions.vkDestroyImage = vkDestroyImage;
    VulkanFunctions.vkCmdCopyBuffer = vkCmdCopyBuffer;
    VulkanFunctions.vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2;
    VulkanFunctions.vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2;
    VulkanFunctions.vkBindBufferMemory2KHR = vkBindBufferMemory2;
    VulkanFunctions.vkBindImageMemory2KHR = vkBindImageMemory2;
    VulkanFunctions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
    VulkanFunctions.vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements;
    VulkanFunctions.vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements;
    AllocatorCreateInformation.pVulkanFunctions = &VulkanFunctions;

    VULKAN_CHECK(vmaCreateAllocator(
        &AllocatorCreateInformation,
        &Allocator
        ));
}

static
VkSurfaceFormatKHR
ChooseSurfaceFormat(
    VOID
    )
/*++

Routine Description:

    Tries to find a specific format, forces it if only one undefined
    format, and returns the first format if those fail.

Arguments:

    None.

Return Value:

    The surface format to use.

--*/
{
    LogDebug("Choosing surface format");

    if ( stbds_arrlenu(Gpu->SurfaceFormats) == 1 &&
         Gpu->SurfaceFormats[0].format == VK_FORMAT_UNDEFINED )
    {
        VkSurfaceFormatKHR Format = {0};
        Format.format = VK_FORMAT_B8G8R8_UNORM;
        Format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return Format;
    }
    else
    {
        UINT32 i;

        for (i = 0; i < stbds_arrlenu(Gpu->SurfaceFormats); i++)
        {
            VkSurfaceFormatKHR* Format = &Gpu->SurfaceFormats[i];
            if (Format->format == VK_FORMAT_B8G8R8A8_UNORM &&
                Format->colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                return *Format;
            }
        }
    }

    return Gpu->SurfaceFormats[0];
}

static
VkPresentModeKHR
ChoosePresentMode(
    VOID
    )
/*++

Routine Description:

    Looks for the mailbox present mode and returns FIFO if that
    can't be found.

Arguments:

    None.

Return Value:

    A suitable presentation mode.

--*/
{
    UINT32 i;

    LogDebug("Choosing presentation mode");

    for ( i = 0; i < stbds_arrlenu(Gpu->PresentModes); i++ )
    {
        if ( Gpu->PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR )
        {
            return Gpu->PresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

static
VkExtent2D
GetSurfaceExtent(
    VOID
    )
/*++

Routine Description:

    Determines the bounds of the surface for the swapchain.

Arguments:

    None.

Return Value:

    The size of the window or surface.

--*/
{
    VkExtent2D Extent = {0};

    LogDebug("Determining surface extent");

    if ( Gpu->SurfaceCapabilities.currentExtent.width == UINT32_MAX )
    {
        VidGetSize(
            &Extent.width,
            &Extent.height
            );
    }
    else
    {
        Extent = Gpu->SurfaceCapabilities.currentExtent;
    }

    return Extent;
}

static
VOID
CreateImageView(
    _Out_ VkImageView* ImageView,
    _In_ VkImage Image,
    _In_ VkFormat Format,
    _In_ VkImageAspectFlags Aspect
    )
{
    VkImageViewCreateInfo ImageViewCreateInformation = {0};
    ImageViewCreateInformation.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    ImageViewCreateInformation.image = Image;

    ImageViewCreateInformation.viewType = VK_IMAGE_VIEW_TYPE_2D;

    ImageViewCreateInformation.format = Format;

    ImageViewCreateInformation.subresourceRange.aspectMask = Aspect;
    ImageViewCreateInformation.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInformation.subresourceRange.levelCount = 1;
    ImageViewCreateInformation.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInformation.subresourceRange.layerCount = 1;
    ImageViewCreateInformation.flags = 0;

    LogTrace("Creating image view");
    VULKAN_CHECK(vkCreateImageView(
        Device,
        &ImageViewCreateInformation,
        NULL,
        ImageView
        ));
}

static
VOID
CreateSwapChain(
    VOID
    )
/*++

Routine Description:

    Creates the swap chain.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 i;

    SurfaceFormat = ChooseSurfaceFormat();
    PresentMode = ChoosePresentMode();
    VidGetSize(
        &SwapChainExtent.width,
        &SwapChainExtent.height
        );

    VkSurfaceCapabilitiesKHR SurfaceCapabilities = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        Gpu->Device,
        Surface,
        &SurfaceCapabilities
        );
    // if ( SwapChainExtent.width != SurfaceCapabilities.currentExtent.width ||
    //      SwapChainExtent.height != SurfaceCapabilities.currentExtent.height )
    // {
    //     SwapChainExtent.width = SurfaceCapabilities.currentExtent.width;
    //     SwapChainExtent.height = SurfaceCapabilities.currentExtent.height;
    // }

    LogDebug("Creating swap chain");

    VkSwapchainCreateInfoKHR SwapChainCreateInformation = {0};
    SwapChainCreateInformation.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapChainCreateInformation.surface = Surface;

    SwapChainCreateInformation.minImageCount = VULKAN_FRAME_COUNT;

    SwapChainCreateInformation.imageFormat = SurfaceFormat.format;
    SwapChainCreateInformation.imageColorSpace = SurfaceFormat.colorSpace;
    SwapChainCreateInformation.imageExtent = SwapChainExtent;
    SwapChainCreateInformation.imageArrayLayers = 1;

    SwapChainCreateInformation.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if ( Gpu->GraphicsFamilyIndex != Gpu->PresentFamilyIndex )
    {
        UINT32 Indices[] = {
            Gpu->GraphicsFamilyIndex,
            Gpu->PresentFamilyIndex
        };

        SwapChainCreateInformation.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInformation.queueFamilyIndexCount = 2;
        SwapChainCreateInformation.pQueueFamilyIndices = Indices;
    }
    else
    {
        SwapChainCreateInformation.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    SwapChainCreateInformation.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    SwapChainCreateInformation.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapChainCreateInformation.presentMode = PresentMode;

    SwapChainCreateInformation.clipped = TRUE;

    VULKAN_CHECK(vkCreateSwapchainKHR(
        Device,
        &SwapChainCreateInformation,
        NULL,
        &SwapChain
        ));
    SetObjectName(
        SwapChain,
        VK_OBJECT_TYPE_SWAPCHAIN_KHR,
        "Swap chain"
        );

    LogDebug("Creating swap chain image views");

    UINT32 ImageCount = 0;

    VULKAN_CHECK(vkGetSwapchainImagesKHR(
        Device,
        SwapChain,
        &ImageCount,
        NULL
        ));
    if ( ImageCount < 1 )
    {
        CmnError("Swap chain has no images");
    }

    stbds_arrsetlen(
        SwapChainImages,
        ImageCount
        );
    VULKAN_CHECK(vkGetSwapchainImagesKHR(
        Device,
        SwapChain,
        &ImageCount,
        SwapChainImages
        ));

    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        CreateImageView(
            &SwapChainImageViews[i],
            SwapChainImages[i],
            SurfaceFormat.format,
            VK_IMAGE_ASPECT_COLOR_BIT
            );
        SetObjectName(
            SwapChainImages[i],
            VK_OBJECT_TYPE_IMAGE,
            "Swap chain image %u",
            i
            );
        SetObjectName(
            SwapChainImageViews[i],
            VK_OBJECT_TYPE_IMAGE_VIEW,
            "Swap chain image view %u",
            i
            );
    }
}

static
VOID
DestroySwapChain(
    VOID
    )
/*++

Routine Description:

    Destroys the swap chain and its image views.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 i;

    LogDebug("Destroying swap chain image views");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        if ( SwapChainImageViews[i] )
        {
            vkDestroyImageView(
                Device,
                SwapChainImageViews[i],
                NULL
                );
            SwapChainImageViews[i] = NULL;
        }
    }

    if ( SwapChainImages )
    {
        LogDebug("Freeing swap chain image list");
        stbds_arrfree(SwapChainImages);
        SwapChainImages = NULL;
    }

    if ( SwapChain )
    {
        LogDebug("Destroying swap chain 0x%llX", (UINT64)SwapChain);
        vkDestroySwapchainKHR(
            Device,
            SwapChain,
            NULL
            );
        SwapChain = NULL;
    }
}

static
VkFormat
ChooseFormat(
    VkFormat* Formats,
    UINT32 FormatCount,
    VkImageTiling ImageTiling,
    VkFormatFeatureFlags FormatFeatures
    )
/*++

Routine Description:

    Chooses a format from the list given.

Arguments:

    Formats - The formats to choose from.

    FormatCount - The number of formats.

    ImageTiling - The desired tiling mode.

    FormatFeatures - The desired features.

Return Value:

    A suitable format or VK_FORMAT_UNDEFINED if none could be found.

--*/
{
    UINT32 i;

    LogDebug("Choosing a format");

    for ( i = 0; i < FormatCount; i++ )
    {
        VkFormatProperties Properties;
        vkGetPhysicalDeviceFormatProperties(
            Gpu->Device,
            Formats[i],
            &Properties
            );

        if ( (ImageTiling == VK_IMAGE_TILING_LINEAR &&
             (Properties.linearTilingFeatures & FormatFeatures) == FormatFeatures) ||
             (ImageTiling == VK_IMAGE_TILING_OPTIMAL &&
             (Properties.optimalTilingFeatures & FormatFeatures) == FormatFeatures) )
        {
            return Formats[i];
        }
    }

    CmnError("Failed to find supported format");

    return VK_FORMAT_UNDEFINED;
}

static
VOID
Initialize(
    VOID
    )
{
    LogDebug("Initializing Vulkan backend");

#ifndef PURPL_SWITCH
    LogDebug("Initializing volk");
    VULKAN_CHECK(volkInitialize());
#endif

    CreateInstance();
    CreateSurface();
    EnumeratePhysicalDevices();
    CreateLogicalDevice();
    CreateSemaphores();
    CreateCommandPools();
    AllocateCommandBuffers();
    CreateAllocator();
    CreateSwapChain();

    FrameIndex = 0;
    VulkanInitialized = TRUE;

    LogDebug("Successfully initialized Vulkan backend");
}

static
VOID
IncrementFrameIndex(
    VOID
    )
{
    FrameIndex = (FrameIndex + 1) % VULKAN_FRAME_COUNT;
}

static
VOID
BeginFrame(
    VOID
    )
{
    if ( !VulkanInitialized )
    {
        return;
    }
}

static
VOID
EndFrame(
    VOID
    )
{
    if ( !VulkanInitialized )
    {
        return;
    }
}

static
VOID
Shutdown(
    VOID
    )
{
    if ( !VulkanInitialized )
    {
        return;
    }

    LogDebug("Shutting down Vulkan backend");

    LogDebug("Successfully shut down Vulkan backend");
}

VOID
VkInitializeBackend(
    _Out_ PRENDER_BACKEND Backend
    )
{
    LogDebug("Filling out render backend for Vulkan");

    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->Shutdown = Shutdown;
}
