/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    vk.h

Abstract:

    This module contains definitions used by the Vulkan backend.

--*/

#pragma once

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#include "common/common.h"
#include "common/log.h"

#include "engine/render/render.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"
END_EXTERN_C

#if defined PURPL_DEBUG || defined PURPL_RELWITHDEBINFO
#define PURPL_VULKAN_DEBUG 1
#endif

#define VMA_VULKAN_VERSION 1003000
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
    //#define VMA_SYSTEM_ALIGNED_MALLOC PURPL_ALIGNED_ALLOC
    //#define VMA_SYSTEM_ALIGNED_FREE PURPL_ALIGNED_FREE
#define VMA_DEBUG_LOG_FORMAT(format, ...) LogDebug((format), __VA_ARGS__)
#define VMA_HEAVY_ASSERT(expr) if ( !(expr) ) \
{ \
    CmnError("VMA assertion failed: " #expr); \
}
#ifdef PURPL_VULKAN_DEBUG
#define VMA_STATS_STRING_ENABLED 1
#endif
#include "vk_mem_alloc.h"

#define VULKAN_FRAME_COUNT 3

//
// Check if something failed
//

#define VULKAN_CHECK(Call, ...) \
    do { \
        VkResult Result_ = (Call); \
        if ( Result_ != VK_SUCCESS __VA_ARGS__ ) \
        { \
            CmnError("Vulkan call " #Call " failed: %s (VkResult %d)", VlkGetResultString(Result_), Result_); \
        } \
    } while(0)

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

//
// Vulkan data
//

typedef struct VULKAN_DATA
{
    //
    // Instance
    //

    VkInstance Instance;

    //
    // Surface
    //

    VkSurfaceKHR Surface;

    //
    // Device stuff
    //

    PVULKAN_GPU_INFO Gpus;
    PVULKAN_GPU_INFO Gpu;
    UINT32 GpuIndex;
    VkDevice Device;

    //
    // VMA stuff
    //

    VmaAllocator Allocator;

    //
    // Queues
    //

    VkQueue GraphicsQueue;
    VkQueue PresentQueue;

    //
    // Command and synchronization stuff
    //

    VkCommandPool CommandPool;
    VkCommandPool TransferCommandPool;
    VkCommandBuffer CommandBuffers[VULKAN_FRAME_COUNT];
    VkFence CommandBufferFences[VULKAN_FRAME_COUNT];
    VkSemaphore AcquireSemaphores[VULKAN_FRAME_COUNT];
    VkSemaphore RenderCompleteSemaphores[VULKAN_FRAME_COUNT];

    //
    // Swap chain stuff
    //

    VkSwapchainKHR SwapChain;
    VkSurfaceFormatKHR SurfaceFormat;
    VkPresentModeKHR PresentMode;
    VkExtent2D SwapChainExtent;
    VkImage* SwapChainImages;
    VkImageView SwapChainImageViews[VULKAN_FRAME_COUNT];
    UINT32 SwapChainIndex;

    //
    // Render target stuff
    //

    VkFormat DepthFormat;
    VkImage DepthImage;
    VmaAllocation DepthImageAllocation;
    VkImageView DepthView;

    //
    // Rendering stuff
    //

    VkRenderPass RenderPass;
    VkFramebuffer Framebuffers[VULKAN_FRAME_COUNT];

    //
    // Descriptor things
    //

    VkDescriptorPool DescriptorPool;
    VkDescriptorSetLayout DescriptorSetLayout;

    //
    // Shared pipeline information
    //

    VkPipelineLayout PipelineLayout;
    VkPipelineCache PipelineCache;

    //
    // Frame index
    //

    UINT8 FrameIndex;

    //
    // Whether Vulkan is initialized
    //

    BOOLEAN Initialized;

    //
    // Whether the swapchain was recreated
    //

    BOOLEAN Resized;

    //
    // Shader stuff
    //

    VULKAN_BUFFER UniformBuffers[VULKAN_FRAME_COUNT];
    PVOID UniformBufferAddresses[VULKAN_FRAME_COUNT];

    //
    // Sampler
    //

    VkSampler Sampler;
} VULKAN_DATA, *PVULKAN_DATA;

extern VULKAN_DATA VlkData;

//
// Create the instance
//

extern
VOID
VlkCreateInstance(
    VOID
    );

//
// Get the name of a result
//

extern
PCSTR
VlkGetResultString(
    VkResult Result
    );

//
// Set an object's name
//

extern
VOID
VlkSetObjectName(
    _In_ PVOID Object,
    _In_ VkObjectType ObjectType,
    _In_ _Printf_format_string_ PCSTR Name,
    ...
    );

//
// Debug callback to log things
//

VkBool32
VKAPI_CALL
VlkDebugCallback(
    _In_ VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
    _In_ VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
    _In_ CONST VkDebugUtilsMessengerCallbackDataEXT* CallbackData,
    _In_opt_ PVOID UserData
    );

//
// Get devices
//

VOID
VlkEnumeratePhysicalDevices(
    VOID
    );
