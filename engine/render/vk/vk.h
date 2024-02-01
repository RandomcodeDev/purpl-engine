/*++

Copyright (c) 2024 Randomcode Developers

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
#define VMA_SYSTEM_ALIGNED_MALLOC CmnAlignedAlloc
#define VMA_SYSTEM_ALIGNED_FREE CmnAlignedFree
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
        VkResult Result = (Call); \
        if ( Result != VK_SUCCESS __VA_ARGS__ ) \
        { \
            CmnError("Vulkan call " #Call " at %s:%d failed: %s (VkResult %d)", __FILE__, __LINE__, VlkGetResultString(Result), Result); \
        } \
    } while( 0 )

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
// Render attachment
//

typedef struct VULKAN_IMAGE
{
    VkImage Handle;
    VmaAllocation Allocation;
    VkImageView View;
    VkFormat Format;
} VULKAN_IMAGE, *PVULKAN_IMAGE;

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
    // Allocator
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
    VkCommandBuffer DeferredCommandBuffer;
    VkCommandBuffer CommandBuffers[VULKAN_FRAME_COUNT];
    VkFence CommandBufferFences[VULKAN_FRAME_COUNT];
    VkSemaphore AcquireSemaphores[VULKAN_FRAME_COUNT];
    VkSemaphore RenderCompleteSemaphores[VULKAN_FRAME_COUNT];
    VkSemaphore DeferredSemaphore;

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
    // Rendering stuff
    //

    VkRenderPass MainRenderPass; // Outputs to the screen
    VkFramebuffer ScreenFramebuffers[VULKAN_FRAME_COUNT];

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
// Get allocation callbacks
//

extern
VkAllocationCallbacks*
VlkGetAllocationCallbacks(
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

extern
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

extern
VOID
VlkEnumeratePhysicalDevices(
    VOID
    );

//
// Create the logical device
//

extern
VOID
VlkCreateLogicalDevice(
    VOID
    );

//
// Create presentation semaphores
//

extern
VOID
VlkCreateSemaphores(
    VOID
    );

//
// Create the main and transfer command pools
//

extern
VOID
VlkCreateCommandPools(
    VOID
    );

//
// Allocate the main command buffers
//

extern
VOID
VlkAllocateCommandBuffers(
    VOID
    );

//
// Allocate a buffer
//

extern
VOID
VlkAllocateBuffer(
    _In_ VkDeviceSize Size,
    _In_ VkBufferUsageFlags Usage,
    _In_ VkMemoryPropertyFlags Flags,
    _Out_ PVULKAN_BUFFER Buffer
    );

//
// Allocate a buffer and copy data to it
//

extern
VOID
VlkAllocateBufferWithData(
    _In_ PVOID Data,
    _In_ VkDeviceSize Size,
    _In_ VkBufferUsageFlags Usage,
    _In_ VkMemoryPropertyFlags Flags,
    _Out_ PVULKAN_BUFFER Buffer
    );

//
// Free a buffer
//

extern
VOID
VlkFreeBuffer(
    _Inout_ PVULKAN_BUFFER Buffer
    );

//
// Create a temporary command buffer in the transfer command pool
//

extern
VkCommandBuffer
VlkBeginTransfer(
    VOID
    );

//
// Submits and frees a command buffer from VlkBeginTransfer
//

extern
VOID
VlkEndTransfer(
    _In_ VkCommandBuffer TransferBuffer
    );
    
//
// Copy a buffer to another buffer
//

extern
VOID
VlkCopyBuffer(
    _In_ PVULKAN_BUFFER Source,
    _In_ PVULKAN_BUFFER Destination,
    _In_ VkDeviceSize Size
    );

//
// Create an image view
//

extern
VOID
VlkCreateImageView(
    _Out_ VkImageView* ImageView,
    _In_ VkImage Image,
    _In_ VkFormat Format,
    _In_ VkImageAspectFlags Aspect
    );

//
// Choose a format from a set
//

extern
VkFormat
VlkChooseFormat(
    VkFormat* Formats,
    UINT32 FormatCount,
    VkImageTiling ImageTiling,
    VkFormatFeatureFlags FormatFeatures
    );

//
// Change an image's layout
//

extern
VOID
VlkTransitionImageLayout(
    _Inout_ VkImage Image,
    _In_ VkImageLayout OldLayout,
    _In_ VkImageLayout NewLayout
    );

//
// Copy a buffer into an image
//

extern
VOID
VlkCopyBufferToImage(
    _In_ VkBuffer Buffer,
    _Out_ VkImage Image,
    _In_ UINT32 Width,
    _In_ UINT32 Height
    );

//
// Create an image
//

extern
VOID
VlkCreateImage(
    _In_ UINT32 Width,
    _In_ UINT32 Height,
    _In_ VkFormat Format,
    _In_ VkImageLayout Layout,
    _In_ VkImageUsageFlags Usage,
    _In_ VmaMemoryUsage MemoryUsage,
    _In_ VkImageAspectFlags Aspect,
    _Out_ PVULKAN_IMAGE Image
    );

//
// Create an initialized image
//

extern
VOID
VlkCreateImageWithData(
    _In_ PVOID Data,
    _In_ VkDeviceSize Size,
    _In_ UINT32 Width,
    _In_ UINT32 Height,
    _In_ VkFormat Format,
    _In_ VkImageLayout Layout,
    _In_ VkImageUsageFlags Usage,
    _In_ VmaMemoryUsage MemoryUsage,
    _In_ VkImageAspectFlags Aspect,
    _Out_ PVULKAN_IMAGE Image
    );

//
// Destroy an image
//

extern
VOID
VlkDestroyImage(
    _Inout_ PVULKAN_IMAGE Image
    );

//
// Choose a surface format
//

extern
VkSurfaceFormatKHR
VlkChooseSurfaceFormat(
    VOID
    );

//
// Choose a present mode
//

extern
VkPresentModeKHR
VlkChoosePresentMode(
    VOID
    );

//
// Get the extent of the surface
//

extern
VkExtent2D
VlkGetSurfaceExtent(
    VOID
    );

//
// Create the swap chain
//

extern
VOID
VlkCreateSwapChain(
    VOID
    );

//
// Destroy the swap chain
//

extern
VOID
VlkDestroySwapChain(
    VOID
    );

//
// Create a render pass
//

extern
VkRenderPass
VlkCreateRenderPass(
    _In_ VkAttachmentDescription* Attachments,
    _In_ SIZE_T AttachmentCount,
    _In_ VkSubpassDescription* Subpasses,
    _In_ SIZE_T SubpassCount,
    _In_ VkSubpassDependency* SubpassDependencies,
    _In_ SIZE_T SubpassDependencyCount
    );

//
// Create the main render pass
//

extern
VOID
VlkCreateMainRenderPass(
    VOID
    );
