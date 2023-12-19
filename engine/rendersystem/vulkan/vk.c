/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    vk.c

Abstract:

    This module implements the Vulkan render system. It is based on a number of tutorials:

        https://vulkan-tutorial.com/Introduction
        https://vkguide.dev/
        https://www.fasterthan.life/blog/2017/7/11/i-am-graphics-and-so-can-you-part-1

--*/

#define RENDERSYSTEM_IMPLEMENTATION
#include "engine/engine.h"

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
// Font data
//

typedef struct VULKAN_FONT_DATA
{
    VULKAN_BUFFER VertexBuffer;
} VULKAN_FONT_DATA, *PVULKAN_FONT_DATA;

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

#ifdef PURPL_USE_MIMALLOC

//
// Use mimalloc
//

static
PVOID
AllocationFunction(
    PVOID Unused,
    SIZE_T Size,
    SIZE_T Alignment,
    VkSystemAllocationScope AllocationScope
    )
{
    return mi_aligned_alloc(
        Alignment,
        Size
        );
}

static
PVOID
ReallocationFunction(
    PVOID Unused,
    PVOID Original,
    SIZE_T Size,
    SIZE_T Alignment,
    VkSystemAllocationScope Scope
    )
{
    return mi_aligned_recalloc(
        Original,
        1,
        Size,
        Alignment
        );
}

static
VOID
FreeFunction(
    PVOID Unused,
    PVOID Chunk
    )
{
    PURPL_FREE(Chunk);
}

CONST VkAllocationCallbacks VulkanAllocationCallbacks = {
    .pfnAllocation = AllocationFunction,
    .pfnReallocation = ReallocationFunction,
    .pfnFree = FreeFunction
};
#endif

CONST VkAllocationCallbacks*
VulkanGetAllocationCallbacks(
    VOID
    )
{
#ifdef PURPL_USE_MIMALLOC
    return &VulkanAllocationCallbacks;
#else
    return NULL;
#endif
}

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
static VkDescriptorSet SharedDescriptorSets[VULKAN_FRAME_COUNT];

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
// Draw stuff
//

static PSHADER LastShader;
static PMODEL LastModel;
static PRENDER_TEXTURE LastTexture;

//
// Sampler
//

static VkSampler Sampler;

//
// All font glyphs use the exact same indices
//

static VULKAN_BUFFER FontGlyphIndexBuffer;

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
            CommonError("Vulkan call " #Call " failed: %s (VkResult %d)", GetResultString(Result_), Result_); \
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
        CommonError("ImGui Vulkan call failed: %s (VkResult %d)", GetResultString(Result), Result);
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
#if !defined(PURPL_LINUX) && !defined(PURPL_SWITCH)
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
        NameInformation.pObjectName = CommonFormatTempStringVarArgs(
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
        (UINT64)PlatformGetReturnAddress(),
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
        VulkanGetAllocationCallbacks(),
        &Instance
        );
    if ( Result == VK_ERROR_LAYER_NOT_PRESENT )
    {
        LogDebug("Validation layers not found, retrying without them");
        CreateInformation.enabledLayerCount = 0;
        Result = vkCreateInstance(
            &CreateInformation,
            VulkanGetAllocationCallbacks(),
            &Instance
            );
    }
    if ( Result != VK_SUCCESS )
    {
        CommonError("Failed to create Vulkan instance: VkResult %d", Result);
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
    Surface = PlatformCreateVulkanSurface(
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
            RenderInterfaces[RenderApi].DeviceName = Gpu->Properties.deviceName;
            RenderInterfaces[RenderApi].DeviceIndex = GpuIndex;
            RenderInterfaces[RenderApi].DeviceVendorId = (UINT16)Gpu->Properties.vendorID;
            RenderInterfaces[RenderApi].DeviceId = (UINT16)Gpu->Properties.deviceID;
            break;
        }
    }

    stbds_arrfree(Devices);

    LogDebug("Got information for %d device(s) (of which %d are usable)", stbds_arrlenu(Gpus), UsableCount);
    if ( UsableCount < 1 )
    {
        CommonError("Could not find any usable Vulkan devices");
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

    VkPhysicalDeviceRobustness2FeaturesEXT DeviceRobustness2Features = {0};
    DeviceRobustness2Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT;
    DeviceRobustness2Features.nullDescriptor = TRUE;

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
        VulkanGetAllocationCallbacks(),
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
            VulkanGetAllocationCallbacks(),
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
            VulkanGetAllocationCallbacks(),
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
        VulkanGetAllocationCallbacks(),
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
        VulkanGetAllocationCallbacks(),
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
            VulkanGetAllocationCallbacks(),
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
        PlatformGetVideoSize(
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
        VulkanGetAllocationCallbacks(),
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
    PlatformGetVideoSize(
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
        VulkanGetAllocationCallbacks(),
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
        CommonError("Swap chain has no images");
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
                VulkanGetAllocationCallbacks()
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
            VulkanGetAllocationCallbacks()
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

    CommonError("Failed to find supported format");

    return VK_FORMAT_UNDEFINED;
}

static
VOID
CreateShaderLayouts(
    VOID
    )
/*++

Routine Description:

    Creates the pipeline and descriptor set layouts.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogDebug("Creating shader layout stuff");

    VkDescriptorSetLayoutBinding UboLayoutBinding = {0};
    UboLayoutBinding.binding = 0;
    UboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    UboLayoutBinding.descriptorCount = 1;
    UboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    //VkDescriptorSetLayoutBinding ScreenSamplerLayoutBinding = {0};
    //ScreenSamplerLayoutBinding.binding = 1;
    //ScreenSamplerLayoutBinding.descriptorCount = 1;
    //ScreenSamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //ScreenSamplerLayoutBinding.pImmutableSamplers = NULL;
    //ScreenSamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding SamplerLayoutBinding = {0};
    SamplerLayoutBinding.binding = 2;
    SamplerLayoutBinding.descriptorCount = 1;
    SamplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    SamplerLayoutBinding.pImmutableSamplers = NULL;
    SamplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding DescriptorSetLayoutBindings[] = {
        UboLayoutBinding,
        //ScreenSamplerLayoutBinding,
        SamplerLayoutBinding
    };

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInformation = {0};
    DescriptorSetLayoutCreateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DescriptorSetLayoutCreateInformation.pBindings = DescriptorSetLayoutBindings;
    DescriptorSetLayoutCreateInformation.bindingCount = PURPL_ARRAYSIZE(DescriptorSetLayoutBindings);

    LogTrace("Calling vkCreateDescriptorSetLayout");
    VULKAN_CHECK(vkCreateDescriptorSetLayout(
        Device,
        &DescriptorSetLayoutCreateInformation,
        VulkanGetAllocationCallbacks(),
        &DescriptorSetLayout
        ));
    SetObjectName(
        DescriptorSetLayout,
        VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT,
        "Main descriptor set layout"
        );

    // TODO: maybe think about per-object uniform buffers instead of this, if it becomes an issue
    VkPushConstantRange PushConstantRange = {0};
    PushConstantRange.offset = 0;
    PushConstantRange.size = sizeof(RENDER_MODEL_UNIFORM_DATA);
    PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkPipelineLayoutCreateInfo PipelineLayoutCreateInformation = {0};
    PipelineLayoutCreateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutCreateInformation.pSetLayouts = &DescriptorSetLayout;
    PipelineLayoutCreateInformation.setLayoutCount = 1;
    PipelineLayoutCreateInformation.pPushConstantRanges = &PushConstantRange;
    PipelineLayoutCreateInformation.pushConstantRangeCount = 1;

    LogTrace("Calling vkCreatePipelineLayout");
    VULKAN_CHECK(vkCreatePipelineLayout(
        Device,
        &PipelineLayoutCreateInformation,
        VulkanGetAllocationCallbacks(),
        &PipelineLayout
        ));
    SetObjectName(
        PipelineLayout,
        VK_OBJECT_TYPE_PIPELINE_LAYOUT,
        "Main pipeline layout"
        );
}

static
VOID
CreateRenderTargets(
    VOID
    )
/*++

Routine Description:

    Creates render target images.

Arguments:

    None.

Return Value:

    None.

--*/
{
    VkFormat Formats[] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT
    };

    DepthFormat = ChooseFormat(
        Formats,
        PURPL_ARRAYSIZE(Formats),
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );

    LogDebug("Creating depth image");

    VkImageCreateInfo ImageCreateInformation = {0};
    ImageCreateInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    PlatformGetVideoSize(
        &ImageCreateInformation.extent.width,
        &ImageCreateInformation.extent.height
        );
    ImageCreateInformation.extent.depth = 1;
    ImageCreateInformation.format = DepthFormat;
    ImageCreateInformation.mipLevels = 1;
    ImageCreateInformation.arrayLayers = 1;
    ImageCreateInformation.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInformation.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    ImageCreateInformation.imageType = VK_IMAGE_TYPE_2D;
    VmaAllocationCreateInfo AllocationCreateInformation = {0};
    AllocationCreateInformation.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
    VULKAN_CHECK(vmaCreateImage(
        Allocator,
        &ImageCreateInformation,
        &AllocationCreateInformation,
        &DepthImage,
        &DepthImageAllocation,
        NULL
        ));
    SetObjectName(
        DepthImage,
        VK_OBJECT_TYPE_IMAGE,
        "Depth image"
        );

    LogDebug("Creating depth image view");

    CreateImageView(
        &DepthView,
        DepthImage,
        DepthFormat,
        VK_IMAGE_ASPECT_DEPTH_BIT
        );
}

static
VOID
DestroyRenderTargets(
    VOID
    )
/*++

Routine Description:

    Destroys the render target images.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if ( DepthView )
    {
        LogDebug("Destroying depth image view 0x%llX", (UINT64)DepthView);
        vkDestroyImageView(
            Device,
            DepthView,
            VulkanGetAllocationCallbacks()
            );
        DepthView = NULL;
    }

    if ( DepthImage )
    {
        LogDebug("Destroying depth image 0x%llX", (UINT64)DepthImage);
        vmaDestroyImage(
            Allocator,
            DepthImage,
            DepthImageAllocation
            );
        DepthImage = NULL;
    }
}

static
VOID
CreateRenderPass(
    VOID
    )
/*++

Routine Description:

    Creates the render pass.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogDebug("Creating render pass");

    VkAttachmentDescription Attachments[2] = {0};
    Attachments[0].format = SurfaceFormat.format;
    Attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    Attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    Attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    Attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    Attachments[1].format = DepthFormat;
    Attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    Attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    Attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    Attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    Attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    Attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    Attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ColourReference = {0};
    ColourReference.attachment = 0;
    ColourReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthReference = {0};
    DepthReference.attachment = 1;
    DepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription SubpassDescription = {0};
    SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    SubpassDescription.colorAttachmentCount = 1;
    SubpassDescription.pColorAttachments = &ColourReference;
    SubpassDescription.pDepthStencilAttachment = &DepthReference;

    VkSubpassDependency SubpassDependency = {0};
    SubpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    SubpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    SubpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    SubpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo RenderPassCreateInformation = {0};
    RenderPassCreateInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInformation.attachmentCount = PURPL_ARRAYSIZE(Attachments);
    RenderPassCreateInformation.pAttachments = Attachments;
    RenderPassCreateInformation.subpassCount = 1;
    RenderPassCreateInformation.pSubpasses = &SubpassDescription;
    RenderPassCreateInformation.dependencyCount = 1;
    RenderPassCreateInformation.pDependencies = &SubpassDependency;

    VULKAN_CHECK(vkCreateRenderPass(
        Device,
        &RenderPassCreateInformation,
        VulkanGetAllocationCallbacks(),
        &RenderPass
        ));
}

static
VOID
CreateFramebuffers(
    VOID
    )
/*++

Routine Description:

    Creates the framebuffers.

Arguments:

    None.

Reteurn Value:

    None.

--*/
{
    VkImageView Attachments[2] = {0};
    UINT32 i;

    Attachments[1] = DepthView;

    LogDebug("Creating %d framebuffers", VULKAN_FRAME_COUNT);

    VkFramebufferCreateInfo FramebufferCreateInformation = {0};
    FramebufferCreateInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferCreateInformation.renderPass = RenderPass;
    FramebufferCreateInformation.attachmentCount = PURPL_ARRAYSIZE(Attachments);
    FramebufferCreateInformation.pAttachments = Attachments;
    PlatformGetVideoSize(
        &FramebufferCreateInformation.width,
        &FramebufferCreateInformation.height
        );
    FramebufferCreateInformation.layers = 1;

    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        Attachments[0] = SwapChainImageViews[i];
        VULKAN_CHECK(vkCreateFramebuffer(
            Device,
            &FramebufferCreateInformation,
            VulkanGetAllocationCallbacks(),
            &Framebuffers[i]
            ));
    }
}

static
VOID
DestroyFramebuffers(
    VOID
    )
/*++

Routine Description:

    Destroys the framebuffers.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 i;

    LogDebug("Destroying framebuffers");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        if ( Framebuffers[i] )
        {
            vkDestroyFramebuffer(
                Device,
                Framebuffers[i],
                VulkanGetAllocationCallbacks()
                );
        }
    }
}

static
VOID
HandleResize(
    VOID
)
{
    LogDebug("Handling resize");

    vkDeviceWaitIdle(Device);

    DestroyFramebuffers();
    vkDestroyRenderPass(
        Device,
        RenderPass,
        NULL
        );
    DestroyRenderTargets();
    DestroySwapChain();
    CreateSwapChain();
    CreateRenderTargets();
    CreateRenderPass();
    CreateFramebuffers();

    // https://stackoverflow.com/questions/70762372/how-to-recreate-swapchain-after-vkacquirenextimagekhr-is-vk-suboptimal-khr
    CONST VkPipelineStageFlags WaitDestinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInformation.waitSemaphoreCount = 1;
    SubmitInformation.pWaitSemaphores = &AcquireSemaphores[FrameIndex];
    SubmitInformation.pWaitDstStageMask = &WaitDestinationStage;
    vkQueueSubmit(
        PresentQueue,
        1,
        &SubmitInformation,
        NULL
        );
}

static
VOID
AllocateBuffer(
    _In_ VkDeviceSize Size,
    _In_ VkBufferUsageFlags Usage,
    _In_ VkMemoryPropertyFlags Flags,
    _Out_ PVULKAN_BUFFER Buffer
    )
/*++

Routine Description:

    Allocates a VkBuffer.

Arguments:

    Size - The size of the buffer.

    Usage - The usage of the buffer.

    Flags - The flags to create the buffer with.

    Buffer - Filled in by this function.

Return Value:

    None.

--*/
{
    memset(
        Buffer,
        0,
        sizeof(VULKAN_BUFFER)
        );
    Buffer->Size = Size;

    VkBufferCreateInfo BufferCreateInformation = {0};
    BufferCreateInformation.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInformation.size = Buffer->Size;
    BufferCreateInformation.usage = Usage;
    BufferCreateInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo AllocationCreateInformation = {0};
    AllocationCreateInformation.requiredFlags = Flags;

    //LogTrace("Allocating %zu-byte buffer 0x%llX", Size, (UINT64)Buffer);
    VULKAN_CHECK(vmaCreateBuffer(
        Allocator,
        &BufferCreateInformation,
        &AllocationCreateInformation,
        &Buffer->Buffer,
        &Buffer->Allocation,
        NULL
        ));
}

static
VOID
FreeBuffer(
    _In_ PVULKAN_BUFFER Buffer
    )
{
    LogTrace("Freeing %zu-byte buffer 0x%llX", Buffer->Size, (UINT64)Buffer);
    vmaDestroyBuffer(
        Allocator,
        Buffer->Buffer,
        Buffer->Allocation
        );
    memset(
        Buffer,
        0,
        sizeof(VULKAN_BUFFER)
        );
}

static
VkCommandBuffer
BeginTransfer(VOID)
{
    VkCommandBufferAllocateInfo CommandBufferAllocateInformation = {0};

    CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInformation.commandPool = TransferCommandPool;
    CommandBufferAllocateInformation.commandBufferCount = 1;

    //LogTrace("Creating transfer command buffer");
    VkCommandBuffer TransferBuffer;
    VULKAN_CHECK(vkAllocateCommandBuffers(
        Device,
        &CommandBufferAllocateInformation,
        &TransferBuffer
        ));

    VkCommandBufferBeginInfo BeginInformation = {0};
    BeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    //LogTrace("Beginning transfer command buffer");
    vkBeginCommandBuffer(
        TransferBuffer,
        &BeginInformation
        );

    return TransferBuffer;
}

static
VOID
EndTransfer(
    _In_ VkCommandBuffer TransferBuffer
    )
{
    //LogTrace("Ending transfer command buffer");
    vkEndCommandBuffer(TransferBuffer);

    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInformation.commandBufferCount = 1;
    SubmitInformation.pCommandBuffers = &TransferBuffer;

    //LogTrace("Submitting transfer command buffer");
    vkQueueSubmit(
        GraphicsQueue,
        1,
        &SubmitInformation,
        NULL
        );
    vkQueueWaitIdle(GraphicsQueue);

    //LogTrace("Destroying transfer command buffer");
    vkFreeCommandBuffers(
        Device,
        TransferCommandPool,
        1,
        &TransferBuffer
        );
}

static
VOID
CopyBuffer(
    _In_ PVULKAN_BUFFER Source,
    _In_ PVULKAN_BUFFER Destination,
    _In_ VkDeviceSize Size
    )
{
    VkCommandBuffer TransferBuffer;

    //LogTrace("Copying Vulkan buffer 0x%llX to 0x%llX", (UINT64)Source, (UINT64)Destination);

    TransferBuffer = BeginTransfer();

    VkBufferCopy CopyRegion = {0};
    CopyRegion.size = Size;
    vkCmdCopyBuffer(
        TransferBuffer,
        Source->Buffer,
        Destination->Buffer,
        1,
        &CopyRegion
        );

    EndTransfer(TransferBuffer);
}

static
VOID
AllocateBufferWithData(
    _In_ PVOID Data,
    _In_ VkDeviceSize Size,
    _In_ VkBufferUsageFlags Usage,
    _In_ VkMemoryPropertyFlags Flags,
    _Out_ PVULKAN_BUFFER Buffer
    )
{
    VULKAN_BUFFER StagingBuffer;
    PVOID BufferAddress;

    AllocateBuffer(
        Size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &StagingBuffer
        );

    BufferAddress = NULL;
    vmaMapMemory(
        Allocator,
        StagingBuffer.Allocation,
        &BufferAddress
        );
    memcpy(
        BufferAddress,
        Data,
        Size
        );
    vmaUnmapMemory(
        Allocator,
        StagingBuffer.Allocation
        );

    AllocateBuffer(
        Size,
        Usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        Flags,
        Buffer
        );
    CopyBuffer(
        &StagingBuffer,
        Buffer,
        Size
        );
    FreeBuffer(&StagingBuffer);
}

static
VOID
TransitionImageLayout(
    _Inout_ VkImage Image,
    _In_ VkImageLayout OldLayout,
    _In_ VkImageLayout NewLayout
    )
{
    VkCommandBuffer TransferBuffer;
    VkPipelineStageFlags SourceStage;
    VkPipelineStageFlags DestinationStage;

    //LogTrace("Transitioning image 0x%llX from %u to %u", (UINT64)Image, OldLayout, NewLayout);

    TransferBuffer = BeginTransfer();

    VkImageMemoryBarrier Barrier = {0};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    Barrier.oldLayout = OldLayout;
    Barrier.newLayout = NewLayout;

    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    Barrier.image = Image;

    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = 1;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;

    if ( OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
         NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if ( OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
              NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
    {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        LogWarning("Unknown layout transition");

        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = 0;

        SourceStage = 0;
        DestinationStage = 0;
    }

    vkCmdPipelineBarrier(
        TransferBuffer,
        SourceStage,
        DestinationStage,
        0,
        0,
        NULL,
        0,
        NULL,
        1,
        &Barrier
        );

    EndTransfer(TransferBuffer);
}

static
VOID
CopyBufferToImage(
    _In_ VkBuffer Buffer,
    _Out_ VkImage Image,
    _In_ UINT32 Width,
    _In_ UINT32 Height
    )
{
    VkCommandBuffer TransferBuffer;

    //LogTrace("Copying buffer 0x%llX to image 0x%llX", (UINT64)Buffer, (UINT64)Image);

    TransferBuffer = BeginTransfer();

    VkBufferImageCopy Region = {0};
    Region.bufferOffset = 0;
    Region.bufferRowLength = 0;
    Region.bufferImageHeight = 0;

    Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.imageSubresource.mipLevel = 0;
    Region.imageSubresource.baseArrayLayer = 0;
    Region.imageSubresource.layerCount = 1;

    Region.imageOffset.x = 0;
    Region.imageOffset.y = 0;
    Region.imageOffset.z = 0;

    Region.imageExtent.width = Width;
    Region.imageExtent.height = Height;
    Region.imageExtent.depth = 1;

    vkCmdCopyBufferToImage(
        TransferBuffer,
        Buffer,
        Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &Region
        );

    EndTransfer(TransferBuffer);
}

static
VOID
CreateImageWithData(
    _In_ PVOID Data,
    _In_ VkDeviceSize Size,
    _In_ UINT32 Width,
    _In_ UINT32 Height,
    _In_ VkImageCreateInfo* ImageCreateInformation,
    _In_ VmaAllocationCreateInfo* AllocationCreateInformation,
    _In_ VkImageLayout TargetLayout,
    _Out_ VkImage* Image,
    _Out_ VmaAllocation* Allocation
    )
{
    VULKAN_BUFFER StagingBuffer;
    PVOID ImageBuffer;

    AllocateBuffer(
        Size,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &StagingBuffer
        );

    ImageBuffer = NULL;
    vmaMapMemory(
        Allocator,
        StagingBuffer.Allocation,
        &ImageBuffer
        );
    memcpy(
        ImageBuffer,
        Data,
        Size
        );
    vmaUnmapMemory(
        Allocator,
        StagingBuffer.Allocation
        );

    VULKAN_CHECK(vmaCreateImage(
        Allocator,
        ImageCreateInformation,
        AllocationCreateInformation,
        Image,
        Allocation,
        NULL
        ));

    TransitionImageLayout(
        *Image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        );
    CopyBufferToImage(
        StagingBuffer.Buffer,
        *Image,
        Width,
        Height
        );
    TransitionImageLayout(
        *Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        );

    FreeBuffer(&StagingBuffer);
}

static
VOID
CreateDescriptorPool(
    VOID
    )
{
    VkDescriptorPoolCreateInfo CreateInformation = {0};

    LogDebug("Creating descriptor pool");

    // ImGui tutorials all do this
    CONST VkDescriptorPoolSize PoolSizes[] = {
        { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
        { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
    };

    CreateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    CreateInformation.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    CreateInformation.poolSizeCount = (UINT32)PURPL_ARRAYSIZE(PoolSizes);
    CreateInformation.maxSets = 1000 * CreateInformation.poolSizeCount;
    CreateInformation.pPoolSizes = PoolSizes;
    VULKAN_CHECK(vkCreateDescriptorPool(
        Device,
        &CreateInformation,
        VulkanGetAllocationCallbacks(),
        &DescriptorPool
        ));
    SetObjectName(
        DescriptorPool,
        VK_OBJECT_TYPE_DESCRIPTOR_POOL,
        "Descriptor pool"
        );
}

static
VOID
CreateUniformBuffers(
    VOID
    )
{
    UINT8 i;

    LogDebug("Allocating %d uniform buffers", VULKAN_FRAME_COUNT);

    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        AllocateBuffer(
            sizeof(RENDER_GLOBAL_UNIFORM_DATA),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &UniformBuffers[i]
            );
        vmaMapMemory(
            Allocator,
            UniformBuffers[i].Allocation,
            &UniformBufferAddresses[i]
            );
        SetObjectName(
            UniformBuffers[i].Buffer,
            VK_OBJECT_TYPE_BUFFER,
            "Uniform buffer %u",
            i
            );
    }
}

static
VOID
CreateSampler(
    VOID
    )
{
    LogDebug("Creating sampler");

    VkSamplerCreateInfo SamplerCreateInformation = {0};
    SamplerCreateInformation.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    SamplerCreateInformation.magFilter = VK_FILTER_LINEAR;
    SamplerCreateInformation.minFilter = VK_FILTER_LINEAR;

    SamplerCreateInformation.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCreateInformation.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    SamplerCreateInformation.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    SamplerCreateInformation.anisotropyEnable = TRUE;
    SamplerCreateInformation.maxAnisotropy = Gpu->Properties.limits.maxSamplerAnisotropy;
    SamplerCreateInformation.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;

    SamplerCreateInformation.unnormalizedCoordinates = FALSE;

    SamplerCreateInformation.compareEnable = FALSE;
    SamplerCreateInformation.compareOp = VK_COMPARE_OP_ALWAYS;

    SamplerCreateInformation.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    SamplerCreateInformation.mipLodBias = 0.0f;
    SamplerCreateInformation.minLod = 0.0f;
    SamplerCreateInformation.maxLod = 0.0f;

    VULKAN_CHECK(vkCreateSampler(
        Device,
        &SamplerCreateInformation,
        VulkanGetAllocationCallbacks(),
        &Sampler
        ));
}

static
VOID
CreateDescriptorSets(
    VOID
    )
{
    VkDescriptorSetLayout Layouts[VULKAN_FRAME_COUNT] = {0};
    UINT i;

    LogDebug("Allocating %d descriptor sets", VULKAN_FRAME_COUNT);

    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        Layouts[i] = DescriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo DescriptorSetAllocateInformation = {0};
    DescriptorSetAllocateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    DescriptorSetAllocateInformation.descriptorPool = DescriptorPool;
    DescriptorSetAllocateInformation.descriptorSetCount = VULKAN_FRAME_COUNT;
    DescriptorSetAllocateInformation.pSetLayouts = Layouts;

    VULKAN_CHECK(vkAllocateDescriptorSets(
        Device,
        &DescriptorSetAllocateInformation,
        SharedDescriptorSets
        ));

    VkDescriptorBufferInfo DescriptorBufferInformation = {0};
    DescriptorBufferInformation.offset = 0;
    DescriptorBufferInformation.range = sizeof(RENDER_GLOBAL_UNIFORM_DATA);

    VkWriteDescriptorSet DescriptorSetWrites[1] = {0};
    DescriptorSetWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorSetWrites[0].dstBinding = 0;
    DescriptorSetWrites[0].dstArrayElement = 0;
    DescriptorSetWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    DescriptorSetWrites[0].descriptorCount = 1;
    DescriptorSetWrites[0].pBufferInfo = &DescriptorBufferInformation;

    LogTrace("Updating descriptor sets");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        DescriptorBufferInformation.buffer = UniformBuffers[i].Buffer;
        DescriptorSetWrites[0].dstSet = SharedDescriptorSets[i];
        SetObjectName(
            SharedDescriptorSets[i],
            VK_OBJECT_TYPE_DESCRIPTOR_SET,
            "Descriptor set %u",
            i
            );

        vkUpdateDescriptorSets(
            Device,
            PURPL_ARRAYSIZE(DescriptorSetWrites),
            DescriptorSetWrites,
            0,
            NULL
            );
    }
}

static
VOID
CreateSharedFontObjects(
    VOID
    )
{
    LogDebug("Creating shared font glyph index buffer");

    CONST UINT32 FontGlyphIndices[] = {
        0, 1, 2,
        1, 2, 3
    };

    AllocateBufferWithData(
        (PVOID)FontGlyphIndices,
        sizeof(FontGlyphIndices),
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &FontGlyphIndexBuffer
        );
}

VOID
VulkanInitialize(
    VOID
    )
/*++

Routine Description:

    Initializes the Vulkan render system.

Arguments:

    None.

Return Value:

    None.

--*/
{
    LogDebug("Initializing Vulkan");

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
    CreateShaderLayouts();
    CreateRenderTargets();
    CreateDescriptorPool();
    CreateUniformBuffers();
    CreateSampler();
    CreateDescriptorSets();
    CreateRenderPass();
    CreateFramebuffers();
    CreateSharedFontObjects();

    FrameIndex = 0;
    VulkanInitialized = TRUE;

    LogDebug("Vulkan initialization succeeded");
}

static
VOID
IncrementFrameIndex(
    VOID
    )
{
    FrameIndex = (FrameIndex + 1) % VULKAN_FRAME_COUNT;
}

VOID
VulkanBeginCommands(
    _In_ PRENDER_GLOBAL_UNIFORM_DATA UniformData,
    _In_ BOOLEAN WindowResized
    )
/*++

Routine Description:

    Prepares the command buffer.

Arguments:

    UniformData - Global uniform data.
    WindowResized - The return value of PlatformVideoResized.

Return Value:

    None.

--*/
{
    VkResult Result;

    VULKAN_CHECK(vkWaitForFences(
        Device,
        1,
        &CommandBufferFences[FrameIndex],
        TRUE,
        UINT64_MAX
        ));

    SwapChainIndex = 0;
    Result = vkAcquireNextImageKHR(
        Device,
        SwapChain,
        UINT64_MAX,
        AcquireSemaphores[FrameIndex],
        NULL,
        &SwapChainIndex
        );
    if ( Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || WindowResized )
    {
        if ( Result != VK_SUCCESS )
        {
            LogDebug("Got VkResult %d when acquiring next swap chain image", Result);
        }
        else
        {
            LogDebug("Window resized");
        }
        HandleResize();
        VULKAN_CHECK(vkAcquireNextImageKHR(
            Device,
            SwapChain,
            UINT64_MAX,
            AcquireSemaphores[FrameIndex],
            NULL,
            &SwapChainIndex
            ));
        Resized = TRUE;
        //return;
    }
    else if ( Result != VK_SUCCESS )
    {
        CommonError("Failed to acquire next image: VkResult %d", Result);
    }

    VULKAN_CHECK(vkResetFences(
        Device,
        1,
        &CommandBufferFences[FrameIndex]
        ));

    VULKAN_CHECK(vkResetCommandBuffer(
        CommandBuffers[FrameIndex],
        0
        ));

    VkCommandBufferBeginInfo CommandBeginInformation = {0};
    CommandBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CommandBeginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VULKAN_CHECK(vkBeginCommandBuffer(
        CommandBuffers[FrameIndex],
        &CommandBeginInformation
        ));

    if ( Resized )
    {
        CONST VkImageMemoryBarrier LayoutBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .image = SwapChainImages[SwapChainIndex],
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        vkCmdPipelineBarrier(
            CommandBuffers[FrameIndex],
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            0,
            NULL,
            0,
            NULL,
            1,
            &LayoutBarrier
            );
    }

    VkClearValue ClearValues[2] = {0};
    ClearValues[0].color.float32[0] = 0.0f;
    ClearValues[0].color.float32[1] = 0.0f;
    ClearValues[0].color.float32[2] = 0.0f;
    ClearValues[0].color.float32[3] = 1.0f;
    ClearValues[1].depthStencil.depth = 1.0f;
    ClearValues[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo RenderPassInformation = {0};
    RenderPassInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassInformation.renderPass = RenderPass;
    PlatformGetVideoSize(
        &RenderPassInformation.renderArea.extent.width,
        &RenderPassInformation.renderArea.extent.height
        );
    RenderPassInformation.framebuffer = Framebuffers[FrameIndex];
    RenderPassInformation.clearValueCount = PURPL_ARRAYSIZE(ClearValues);
    RenderPassInformation.pClearValues = ClearValues;

    vkCmdBeginRenderPass(
        CommandBuffers[FrameIndex],
        &RenderPassInformation,
        VK_SUBPASS_CONTENTS_INLINE
        );

    VkRect2D Scissor = {0};
    PlatformGetVideoSize(
        &Scissor.extent.width,
        &Scissor.extent.height
        );

    VkViewport Viewport = {0};
    Viewport.width = (FLOAT)Scissor.extent.width;
    Viewport.height = (FLOAT)Scissor.extent.height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    vkCmdSetViewport(
        CommandBuffers[FrameIndex],
        0,
        1,
        &Viewport
        );
    vkCmdSetScissor(
        CommandBuffers[FrameIndex],
        0,
        1,
        &Scissor
        );
    vkCmdSetPrimitiveTopology(
        CommandBuffers[FrameIndex],
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
        );

    memcpy(
        UniformBufferAddresses[FrameIndex],
        UniformData,
        sizeof(RENDER_GLOBAL_UNIFORM_DATA)
        );

    LastShader = NULL;
    LastModel = NULL;
}

static
VOID
BindShader(
    _In_ PSHADER Shader
    )
{
    if ( LastShader != Shader )
    {
        vkCmdBindPipeline(
            CommandBuffers[FrameIndex],
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            Shader->Handle
            );
        LastShader = Shader;
    }

    vkCmdBindDescriptorSets(
        CommandBuffers[FrameIndex],
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        PipelineLayout,
        0,
        1,
        &SharedDescriptorSets[FrameIndex],
        0,
        NULL
        );
}

static
VOID
BindTexture(
    _In_ PRENDER_TEXTURE Texture
    )
{
    PVULKAN_TEXTURE_DATA TextureData;

    TextureData = Texture->Handle;
    VkDescriptorImageInfo DescriptorImageInformation = { 0 };
    DescriptorImageInformation.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    DescriptorImageInformation.imageView = TextureData->ImageView;
    DescriptorImageInformation.sampler = Sampler;

    VkWriteDescriptorSet DescriptorSetWrite = { 0 };
    DescriptorSetWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    DescriptorSetWrite.dstBinding = 2;
    DescriptorSetWrite.dstArrayElement = 0;
    DescriptorSetWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    DescriptorSetWrite.descriptorCount = 1;
    DescriptorSetWrite.pImageInfo = &DescriptorImageInformation;
    DescriptorSetWrite.dstSet = SharedDescriptorSets[FrameIndex];

    vkUpdateDescriptorSets(
        Device,
        1,
        &DescriptorSetWrite,
        0,
        NULL
        );
}

VOID
VulkanDrawModel(
    _In_ PMODEL Model,
    _In_ PRENDER_MODEL_UNIFORM_DATA UniformData
    )
{
    if ( Resized )
    {
        return;
    }

    vkCmdPushConstants(
        CommandBuffers[FrameIndex],
        PipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        sizeof(RENDER_MODEL_UNIFORM_DATA),
        UniformData
        );

    if ( LastModel != Model )
    {
        PVULKAN_MODEL_DATA ModelData = Model->Handle;
        VkDeviceSize Offset = 0;

        vkCmdBindVertexBuffers(
            CommandBuffers[FrameIndex],
            0,
            1,
            &ModelData->VertexBuffer.Buffer,
            &Offset
            );
        if ( Model->Mesh->IndexCount )
        {
            vkCmdBindIndexBuffer(
                CommandBuffers[FrameIndex],
                ModelData->IndexBuffer.Buffer,
                Offset,
                VK_INDEX_TYPE_UINT32
                );
        }
        LastModel = Model;
    }

    BindTexture(Model->Material->Texture);
    BindShader(Model->Material->Shader);

    vkCmdDrawIndexed(
        CommandBuffers[FrameIndex],
        (UINT32)(Model->Mesh->IndexCount * 3),
        1,
        0,
        0,
        0
        );
}

VOID
VulkanDrawGlyph(
    _In_ PRENDER_FONT Font,
    _In_ FLOAT Scale,
    _In_ vec4 Colour,
    _In_ vec2 Position,
    _In_ PGLYPH Glyph,
    _In_ SIZE_T Offset
    )
{
    PVULKAN_FONT_DATA FontData;
    VkDeviceSize VertexBufferOffset;

    if ( Resized )
    {
        return;
    }

    FontData = Font->Handle;

    VertexBufferOffset = Offset;
    vkCmdBindVertexBuffers(
        CommandBuffers[FrameIndex],
        0,
        1,
        &FontData->VertexBuffer.Buffer,
        &VertexBufferOffset
        );
    vkCmdBindIndexBuffer(
        CommandBuffers[FrameIndex],
        FontGlyphIndexBuffer.Buffer,
        0,
        VK_INDEX_TYPE_UINT32
        );

    BindTexture(Font->Atlas);
    BindShader(RenderGetShader("font"));

    vkCmdDrawIndexed(
        CommandBuffers[FrameIndex],
        6,
        1,
        0,
        (INT32)VertexBufferOffset,
        0
        );
}

VOID
VulkanPresentFrame(
    VOID
    )
/*++

Routine Description:

    Presents the most recent frame.

Arguments:

    None.

Return Value:

    None.

--*/
{
    VkResult Result;

    if ( Resized )
    {
        Resized = FALSE;
    }

    vkCmdEndRenderPass(CommandBuffers[FrameIndex]);
    VULKAN_CHECK(vkEndCommandBuffer(CommandBuffers[FrameIndex]));

    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubmitInformation.pWaitDstStageMask = &WaitStage;

    SubmitInformation.waitSemaphoreCount = 1;
    SubmitInformation.pWaitSemaphores = &AcquireSemaphores[FrameIndex];
    SubmitInformation.signalSemaphoreCount = 1;
    SubmitInformation.pSignalSemaphores = &RenderCompleteSemaphores[FrameIndex];
    SubmitInformation.commandBufferCount = 1;
    SubmitInformation.pCommandBuffers = &CommandBuffers[FrameIndex];

    VULKAN_CHECK(vkQueueSubmit(
        PresentQueue,
        1,
        &SubmitInformation,
        CommandBufferFences[FrameIndex]
        ));

    VkPresentInfoKHR PresentInformation = {0};
    PresentInformation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInformation.pSwapchains = &SwapChain;
    PresentInformation.swapchainCount = 1;
    PresentInformation.pWaitSemaphores = &RenderCompleteSemaphores[FrameIndex];
    PresentInformation.waitSemaphoreCount = 1;
    PresentInformation.pImageIndices = &SwapChainIndex;

    Result = vkQueuePresentKHR(
        PresentQueue,
        &PresentInformation
        );
    if ( Result != VK_ERROR_OUT_OF_DATE_KHR && Result != VK_SUCCESS )
    {
        CommonError("Failed to present frame: VkResult %d", Result);
    }

    IncrementFrameIndex();
}

VOID
VulkanShutdown(
    VOID
    )
/*++

Routine Description:

    Destroys all resources.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 i;

    LogDebug("Shutting down Vulkan");
    VulkanInitialized = FALSE;
    vkDeviceWaitIdle(Device);

    DestroyFramebuffers();

    FreeBuffer(&FontGlyphIndexBuffer);

    if ( RenderPass )
    {
        LogDebug("Destroying render pass 0x%llX", RenderPass);
        vkDestroyRenderPass(
            Device,
            RenderPass,
            VulkanGetAllocationCallbacks()
            );
    }

    if ( Sampler )
    {
        LogDebug("Destroying sampler 0x%llX", (UINT64)Sampler);
        vkDestroySampler(
            Device,
            Sampler,
            VulkanGetAllocationCallbacks()
            );
        Sampler = NULL;
    }

    LogDebug("Freeing uniform buffers");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        vmaUnmapMemory(
            Allocator,
            UniformBuffers[i].Allocation
            );
        FreeBuffer(&UniformBuffers[i]);
    }

    if ( DescriptorPool )
    {
        LogDebug("Destroying descriptor pool 0x%llX", (UINT64)DescriptorPool);
        vkDestroyDescriptorPool(
            Device,
            DescriptorPool,
            VulkanGetAllocationCallbacks()
            );
        DescriptorPool = NULL;
    }

    if ( DescriptorSetLayout )
    {
        LogDebug("Destroying descriptor set layout 0x%llX", (UINT64)DescriptorSetLayout);
        vkDestroyDescriptorSetLayout(
            Device,
            DescriptorSetLayout,
            VulkanGetAllocationCallbacks()
            );
        DescriptorSetLayout = NULL;
    }

    if ( PipelineLayout )
    {
        LogDebug("Destroying pipeline layout 0x%llX", (UINT64)PipelineLayout);
        vkDestroyPipelineLayout(
            Device,
            PipelineLayout,
            VulkanGetAllocationCallbacks()
            );
        PipelineLayout = NULL;
    }

    DestroyRenderTargets();

    DestroySwapChain();

    LogDebug("Destroying command fences");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        if ( CommandBufferFences[i] )
        {
            vkDestroyFence(
                Device,
                CommandBufferFences[i],
                VulkanGetAllocationCallbacks()
                );
            CommandBufferFences[i] = NULL;
        }
    }

    if ( TransferCommandPool )
    {
        LogDebug("Destroying transfer command pool 0x%llX", (UINT64)TransferCommandPool);
        vkDestroyCommandPool(
            Device,
            TransferCommandPool,
            VulkanGetAllocationCallbacks()
            );
        TransferCommandPool = NULL;
    }

    if ( CommandPool )
    {
        LogDebug("Destroying command pool 0x%llX", (UINT64)CommandPool);
        vkDestroyCommandPool(
            Device,
            CommandPool,
            VulkanGetAllocationCallbacks()
            );
        CommandPool = NULL;
    }

    LogDebug("Destroying semaphores");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        if ( AcquireSemaphores[i] )
        {
            vkDestroySemaphore(
                Device,
                AcquireSemaphores[i],
                VulkanGetAllocationCallbacks()
                );
            AcquireSemaphores[i] = NULL;
        }
        if ( RenderCompleteSemaphores[i] )
        {
            vkDestroySemaphore(
                Device,
                RenderCompleteSemaphores[i],
                VulkanGetAllocationCallbacks()
                );
            RenderCompleteSemaphores[i] = NULL;
        }
    }

    if ( Surface )
    {
        LogDebug("Destroying VkSurfaceKHR 0x%llX", (UINT64)Surface);
        vkDestroySurfaceKHR(
            Instance,
            Surface,
            VulkanGetAllocationCallbacks()
            );
        Surface = NULL;
    }

    if ( Allocator )
    {
        LogDebug("Destroying VmaAllocator 0x%llX", (UINT64)Device);
        vmaDestroyAllocator(Allocator);
        Allocator = NULL;
    }

    if ( Device )
    {
        LogDebug("Destroying VkDevice 0x%llX", (UINT64)Device);
        vkDestroyDevice(
            Device,
            VulkanGetAllocationCallbacks()
            );
        Device = NULL;
    }

    if ( Gpus )
    {
        LogDebug("Freeing GPU list");
        for ( i = 0; i < stbds_arrlenu(Gpus); i++ )
        {
            if ( Gpus[i].QueueFamilyProperties )
            {
                stbds_arrfree(Gpus[i].QueueFamilyProperties);
            }
            if ( Gpus[i].ExtensionProperties )
            {
                stbds_arrfree(Gpus[i].ExtensionProperties);
            }
            if ( Gpus[i].SurfaceFormats )
            {
                stbds_arrfree(Gpus[i].SurfaceFormats);
            }
            if ( Gpus[i].PresentModes )
            {
                stbds_arrfree(Gpus[i].PresentModes);
            }
            memset(
                &Gpus[i],
                0,
                sizeof(VULKAN_GPU_INFO)
                );
        }
        Gpu = NULL;
        stbds_arrfree(Gpus);
        Gpus = NULL;
    }

    if ( Instance )
    {
        LogDebug("Destroying VkInstance 0x%llX", (UINT64)Instance);
        vkDestroyInstance(
            Instance,
            VulkanGetAllocationCallbacks()
            );
        Instance = NULL;
    }

    LogDebug("Vulkan shutdown succeeded");
}

static CONST VkVertexInputAttributeDescription MeshVertexAttributeDescriptions[4] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(VERTEX, Position),
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32A32_SFLOAT,
        .offset = offsetof(VERTEX, Colour),
    },
    {
        .binding = 0,
        .location = 2,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(VERTEX, TextureCoordinate),
    },
    {
        .binding = 0,
        .location = 3,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(VERTEX, Normal),
    }
};

static CONST VkVertexInputAttributeDescription FontVertexAttributeDescriptions[2] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(GLYPH_VERTEX, Position),
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(GLYPH_VERTEX, TextureCoordinate),
    },
};

VOID
VulkanCreateShader(
    _In_ PSHADER SourceShader
    )
/*++

Routine Description:

    Creates a pipeline and shader objects.

Arguments:

    SourceShader - The shader object to complete.

Return Value:

    None.

--*/
{
    VkResult Result;
    VkShaderModule VertexModule;
    VkShaderModule FragmentModule;
    CONST VkVertexInputAttributeDescription* VertexAttributeDescriptions;
    SIZE_T VertexAttributeCount;

    LogDebug("Loading type %d Vulkan shader %s", SourceShader->Type, SourceShader->Name);

    VkShaderModuleCreateInfo VertexCreateInformation = {0};
    VertexCreateInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    VertexCreateInformation.codeSize = SourceShader->VertexLength;
    VertexCreateInformation.pCode = SourceShader->VertexData;

    VkShaderModuleCreateInfo FragmentCreateInformation = {0};
    FragmentCreateInformation.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    FragmentCreateInformation.codeSize = SourceShader->FragmentLength;
    FragmentCreateInformation.pCode = SourceShader->FragmentData;

    LogTrace("Creating shader modules");
    VULKAN_CHECK(vkCreateShaderModule(
        Device,
        &VertexCreateInformation,
        NULL,
        &VertexModule
        ));
    VULKAN_CHECK(vkCreateShaderModule(
        Device,
        &FragmentCreateInformation,
        NULL,
        &FragmentModule
        ));

    CONST VkDynamicState DynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_LINE_WIDTH,
        VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY
    };

    VkPipelineDynamicStateCreateInfo PipelineDynamicState = {0};
    PipelineDynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    PipelineDynamicState.dynamicStateCount = PURPL_ARRAYSIZE(DynamicStates);
    PipelineDynamicState.pDynamicStates = DynamicStates;

    VkPipelineShaderStageCreateInfo VertexStageCreateInformation = {0};
    VertexStageCreateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertexStageCreateInformation.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertexStageCreateInformation.module = VertexModule;
    VertexStageCreateInformation.pName = "main";

    VkPipelineShaderStageCreateInfo FragmentStageCreateInformation = {0};
    FragmentStageCreateInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragmentStageCreateInformation.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragmentStageCreateInformation.module = FragmentModule;
    FragmentStageCreateInformation.pName = "main";

    VkPipelineShaderStageCreateInfo ShaderStages[] = {
        VertexStageCreateInformation,
        FragmentStageCreateInformation
    };

    VkVertexInputBindingDescription VertexBindingDescription = {0};
    VertexBindingDescription.binding = 0;
    VertexBindingDescription.stride = sizeof(VERTEX);
    VertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    switch ( SourceShader->Type )
    {
    case ShaderTypeMesh:
        VertexAttributeDescriptions = MeshVertexAttributeDescriptions;
        VertexAttributeCount = PURPL_ARRAYSIZE(MeshVertexAttributeDescriptions);
        break;
    case ShaderTypeFont:
        VertexAttributeDescriptions = FontVertexAttributeDescriptions;
        VertexAttributeCount = PURPL_ARRAYSIZE(FontVertexAttributeDescriptions);
        break;
    //case ShaderTypeUi:
    //    VertexAttributeDescriptions = UiVertexAttributeDescriptions;
    //    VertexAttributeCount = PURPL_ARRAYSIZE(UiVertexAttributeDescriptions);
    //    break;
    }

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = {0};
    InputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyState.primitiveRestartEnable = FALSE;

    VkRect2D Scissor = {0};
    PlatformGetVideoSize(
        &Scissor.extent.width,
        &Scissor.extent.height
        );

    VkViewport Viewport = {0};
    Viewport.width = (FLOAT)Scissor.extent.width;
    Viewport.height = (FLOAT)Scissor.extent.height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    VkPipelineViewportStateCreateInfo ViewportState = {0};
    ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportState.viewportCount = 1;
    ViewportState.pViewports = &Viewport;
    ViewportState.scissorCount = 1;
    ViewportState.pScissors = &Scissor;

    VkPipelineVertexInputStateCreateInfo VertexInputInformation = {0};
    VertexInputInformation.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    VertexInputInformation.vertexBindingDescriptionCount = 1;
    VertexInputInformation.pVertexBindingDescriptions = &VertexBindingDescription;
    VertexInputInformation.vertexAttributeDescriptionCount = VertexAttributeCount;
    VertexInputInformation.pVertexAttributeDescriptions = VertexAttributeDescriptions;

    VkPipelineRasterizationStateCreateInfo RasterizationState = {0};
    RasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizationState.lineWidth = 1.0f;
    RasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
    RasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo MultisampleState = {0};
    MultisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisampleState.minSampleShading = 1.0f;

    VkPipelineColorBlendAttachmentState ColourBlendAttachment = {0};
    ColourBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColourBlendAttachment.blendEnable = TRUE;
    ColourBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColourBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;

    VkPipelineColorBlendStateCreateInfo ColourBlendState = {0};
    ColourBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColourBlendState.attachmentCount = 1;
    ColourBlendState.pAttachments = &ColourBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo DepthStencilState = {0};
    DepthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    //DepthStencilState.flags = VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT;
    DepthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
    DepthStencilState.depthWriteEnable = TRUE;
    DepthStencilState.depthTestEnable = TRUE;
    DepthStencilState.depthBoundsTestEnable = FALSE;
    DepthStencilState.stencilTestEnable = FALSE;

    VkGraphicsPipelineCreateInfo PipelineCreateInformation = {0};
    PipelineCreateInformation.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineCreateInformation.stageCount = PURPL_ARRAYSIZE(ShaderStages);
    PipelineCreateInformation.pStages = ShaderStages;
    PipelineCreateInformation.pVertexInputState = &VertexInputInformation;
    PipelineCreateInformation.pInputAssemblyState = &InputAssemblyState;
    PipelineCreateInformation.pViewportState = &ViewportState;
    PipelineCreateInformation.pRasterizationState = &RasterizationState;
    PipelineCreateInformation.pMultisampleState = &MultisampleState;
    PipelineCreateInformation.pColorBlendState = &ColourBlendState;
    PipelineCreateInformation.pDynamicState = &PipelineDynamicState;
    PipelineCreateInformation.pDepthStencilState = &DepthStencilState;
    PipelineCreateInformation.layout = PipelineLayout;
    PipelineCreateInformation.renderPass = RenderPass;

    LogTrace("Creating pipeline");
    Result = vkCreateGraphicsPipelines(
        Device,
        NULL,
        1,
        &PipelineCreateInformation,
        NULL,
        (VkPipeline*)&SourceShader->Handle
        );
    if ( Result != VK_SUCCESS )
    {
        LogError("Failed to load shader %s: VkResult %d", SourceShader->Name, Result);
        return;
    }
    SetObjectName(
        SourceShader->Handle,
        VK_OBJECT_TYPE_PIPELINE,
        "%s pipeline",
        SourceShader->Name
        );

    LogTrace("Destroying shader modules");
    vkDestroyShaderModule(
        Device,
        VertexModule,
        NULL
        );
    vkDestroyShaderModule(
        Device,
        FragmentModule,
        NULL
        );
}

VOID
VulkanDestroyShader(
    _In_ PSHADER Shader
    )
/*++

Routine Description:

    Destroys a shader object

Arguments:

    Shader - The shader to destroy.

Return Value:

    None.

--*/
{
    if ( !VulkanInitialized || !Shader )
    {
        return;
    }

    vkDeviceWaitIdle(Device);

    LogDebug("Destroying shader %s", Shader->Name);

    if ( Shader->Handle )
    {
        LogTrace("Destroying pipeline");
        vkDestroyPipeline(
            Device,
            Shader->Handle,
            NULL
            );
        Shader->Handle = NULL;
    }
}

VOID
VulkanUseMesh(
    _In_ PMODEL SourceModel
    )
{
    PVULKAN_MODEL_DATA ModelData;
    VkDeviceSize VertexBufferSize;
    VkDeviceSize IndexBufferSize;

    ModelData = PURPL_ALLOC(
        1,
        sizeof(VULKAN_MODEL_DATA)
        );
    if ( !ModelData )
    {
        CommonError("Failed to allocate memory for Vulkan data for model %s", SourceModel->Name);
    }
    SourceModel->Handle = ModelData;

    // TODO: Single massive vertex buffer with as many meshes as possible for better efficiency?

    LogTrace("Copying vertex buffer");
    VertexBufferSize = sizeof(VERTEX) * SourceModel->Mesh->VertexCount;
    AllocateBufferWithData(
        SourceModel->Mesh->Vertices,
        VertexBufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &ModelData->VertexBuffer
        );
    SetObjectName(
        ModelData->VertexBuffer.Buffer,
        VK_OBJECT_TYPE_BUFFER,
        "%s vertex buffer",
        SourceModel->Name
        );

    if ( SourceModel->Mesh->IndexCount )
    {
        LogTrace("Copying index buffer");
        IndexBufferSize = sizeof(ivec3) * SourceModel->Mesh->IndexCount;
        AllocateBufferWithData(
            SourceModel->Mesh->Indices,
            IndexBufferSize,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            &ModelData->IndexBuffer
            );
        SetObjectName(
            ModelData->IndexBuffer.Buffer,
            VK_OBJECT_TYPE_BUFFER,
            "%s index buffer",
            SourceModel->Name
            );
    }
}

VOID
VulkanDestroyModel(
    _In_ PMODEL Model
    )
{
    PVULKAN_MODEL_DATA ModelData;

    if ( !Model->Handle )
    {
        return;
    }

    vkDeviceWaitIdle(Device);

    ModelData = Model->Handle;
    if ( ModelData )
    {
        if ( Model->Mesh->IndexCount )
        {
            FreeBuffer(&ModelData->IndexBuffer);
        }
        FreeBuffer(&ModelData->VertexBuffer);
        PURPL_FREE(ModelData);
        Model->Handle = NULL;
    }
}

VOID
VulkanUseTexture(
    _In_ PRENDER_TEXTURE Texture
    )
{
    VkDeviceSize ImageSize;
    VULKAN_BUFFER StagingBuffer;
    PVOID ImageBuffer;
    PVULKAN_TEXTURE_DATA TextureData;

    ImageSize = GetTextureSize(*Texture->Texture);

    VkImageCreateInfo ImageCreateInformation = {0};
    ImageCreateInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInformation.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInformation.extent.width = (UINT32)Texture->Texture->Width;
    ImageCreateInformation.extent.height = (UINT32)Texture->Texture->Height;
    ImageCreateInformation.extent.depth = 1;
    ImageCreateInformation.mipLevels = 1;
    ImageCreateInformation.arrayLayers = 1;

    switch ( Texture->Texture->Format )
    {
    case TextureFormatRgb8:
        ImageCreateInformation.format = VK_FORMAT_R8G8B8_SRGB;
        break;
    case TextureFormatDepth:
        ImageCreateInformation.format = VK_FORMAT_D32_SFLOAT;
    case TextureFormatRgba8:
    default:
        ImageCreateInformation.format = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    }

    ImageCreateInformation.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInformation.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInformation.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ImageCreateInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInformation.samples = VK_SAMPLE_COUNT_1_BIT;
    ImageCreateInformation.flags = 0;

    VmaAllocationCreateInfo AllocationCreateInformation = {0};
    AllocationCreateInformation.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    TextureData = PURPL_ALLOC(
        1,
        sizeof(VULKAN_TEXTURE_DATA)
        );
    if ( !TextureData )
    {
        CommonError("Failed to allocate memory for Vulkan data for texture %s: %s", Texture->Name, strerror(errno));
    }
    Texture->Handle = TextureData;

    CreateImageWithData(
        Texture->Texture->Pixels,
        ImageSize,
        Texture->Texture->Width,
        Texture->Texture->Height,
        &ImageCreateInformation,
        &AllocationCreateInformation,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        &TextureData->Image,
        &TextureData->Allocation
        );

    CreateImageView(
        &TextureData->ImageView,
        TextureData->Image,
        ImageCreateInformation.format,
        VK_IMAGE_ASPECT_COLOR_BIT
        );
}

VOID
VulkanDestroyTexture(
    _In_ PRENDER_TEXTURE Texture
    )
{
    PVULKAN_TEXTURE_DATA TextureData;

    if ( !Texture->Handle )
    {
        return;
    }

    LogDebug("Destroying texture %s", Texture->Name);

    LogTrace("Destroying image");
    TextureData = Texture->Handle;
    vkDestroyImageView(
        Device,
        TextureData->ImageView,
        VulkanGetAllocationCallbacks()
        );
    vmaDestroyImage(
        Allocator,
        TextureData->Image,
        TextureData->Allocation
        );
    memset(
        TextureData,
        0,
        sizeof(VULKAN_TEXTURE_DATA)
        );
    PURPL_FREE(TextureData);
    Texture->Handle = NULL;
}

VOID
VulkanUseFont(
    _In_ PRENDER_FONT SourceFont
    )
{
    PVULKAN_FONT_DATA FontData;
    VULKAN_BUFFER StagingBuffer;
    PVOID StagingBufferAddress;
    VkDeviceSize GlyphsSize;
    SIZE_T i;

    FontData = PURPL_ALLOC(
        1,
        sizeof(VULKAN_FONT_DATA)
        );
    if ( !FontData )
    {
        CommonError("Failed to allocate memory for Vulkan font data for font %s: %s", SourceFont->Name, strerror(errno));
    }
    SourceFont->Handle = FontData;

    GlyphsSize = stbds_hmlenu(SourceFont->Font->Glyphs) * sizeof(GLYPH);

    AllocateBuffer(
        GlyphsSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &StagingBuffer
        );
    AllocateBuffer(
        GlyphsSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        &FontData->VertexBuffer
        );

    StagingBufferAddress = NULL;
    vmaMapMemory(
        Allocator,
        StagingBuffer.Allocation,
        &StagingBufferAddress
        );
    for ( i = 0; i < stbds_hmlenu(SourceFont->Font->Glyphs); i++ )
    {
        memcpy(
            StagingBufferAddress,
            &SourceFont->Font->Glyphs[i].value,
            sizeof(GLYPH)
            );
    }
    vmaUnmapMemory(
        Allocator,
        StagingBuffer.Allocation
        );

    VkCommandBuffer TransferBuffer = BeginTransfer();
    CopyBuffer(
        &StagingBuffer,
        &FontData->VertexBuffer,
        GlyphsSize
        );
    EndTransfer(TransferBuffer);

    FreeBuffer(&StagingBuffer);
}

VOID
VulkanDestroyFont(
    _In_ PRENDER_FONT Font
    )
{
    PVULKAN_FONT_DATA FontData;

    FontData = Font->Handle;
    if ( !FontData )
    {
        return;
    }

    LogDebug("Destroying font %s", Font->Name);

    FreeBuffer(&FontData->VertexBuffer);
    memset(
        FontData,
        0,
        sizeof(VULKAN_FONT_DATA)
        );
    PURPL_FREE(FontData);
    Font->Handle = NULL;
}
