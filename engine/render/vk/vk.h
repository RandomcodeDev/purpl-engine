/// @file vk.h
///
/// @brief This file contains definitions used by the Vulkan backend.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#include "common/alloc.h"
#include "common/common.h"
#include "common/log.h"

#include "engine/engine.h"

#include "engine/render/render.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"
END_EXTERN_C

#if defined PURPL_DEBUG
#define PURPL_VULKAN_DEBUG 1
#endif

#define VMA_VULKAN_VERSION 1003000
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#define VMA_SYSTEM_ALIGNED_MALLOC CmnAlignedAlloc
#define VMA_SYSTEM_ALIGNED_FREE CmnAlignedFree
#define VMA_DEBUG_LOG_FORMAT(format, ...) LogDebug((format), __VA_ARGS__)
#define VMA_ASSERT(expr)                                                                                               \
    if (!(expr))                                                                                                       \
    {                                                                                                                  \
        CmnError("VMA assertion failed: " #expr);                                                                      \
    }
#ifdef PURPL_VULKAN_DEBUG
#define VMA_STATS_STRING_ENABLED 1
#endif
#include "vk_mem_alloc.h"

#define VULKAN_FRAME_COUNT 3
#define VULKAN_MAX_DESCRIPTOR_SETS 1000

/// @brief Hard error if a VkResult isn't VK_SUCCESS
///
/// @param[in] Call The call/expression to check
/// @param[in] ... Anything extra to put in the if statement
#define VULKAN_CHECK(Call, ...)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        VkResult Result_ = (Call);                                                                                     \
        if (Result_ != VK_SUCCESS __VA_ARGS__)                                                                         \
        {                                                                                                              \
            CmnError("Vulkan call " #Call " : %s (VkResult %d)",                                                       \
                     VlkGetResultString(Result_), Result_);                                                            \
        }                                                                                                              \
    } while (0)

/// @brief A buffer allocated with VlkAllocateBuffer
PURPL_MAKE_TAG(struct, VULKAN_BUFFER, {
    VkBuffer Buffer;
    VmaAllocation Allocation;
    VkDeviceSize Size;
})

/// @brief Data for a model
PURPL_MAKE_TAG(struct, VULKAN_MODEL_DATA, {
    VULKAN_BUFFER VertexBuffer;
    VULKAN_BUFFER IndexBuffer;
})

#define VULKAN_UNIFORM_ALIGNMENT 64

PURPL_MAKE_TAG(struct, VULKAN_SCENE_UNIFORM, {
    RENDER_SCENE_UNIFORM Data;
    BYTE Padding[PURPL_ALIGN(VULKAN_UNIFORM_ALIGNMENT, sizeof(RENDER_SCENE_UNIFORM)) - sizeof(RENDER_SCENE_UNIFORM)];
})

PURPL_MAKE_TAG(struct, VULKAN_OBJECT_UNIFORM, {
    RENDER_OBJECT_UNIFORM Data;
    BYTE Padding[PURPL_ALIGN(VULKAN_UNIFORM_ALIGNMENT, sizeof(RENDER_OBJECT_UNIFORM)) - sizeof(RENDER_OBJECT_UNIFORM)];
})

#define VULKAN_SET_UNIFORM(UniformBufferAddress, Value)                                                                \
    memcpy(&(&(UniformBufferAddress)[VlkData.FrameIndex])->Data, (Value), sizeof(*Value))

/// @brief Data for an object
PURPL_MAKE_TAG(struct, VULKAN_OBJECT_DATA, {
    VkDescriptorSet DescriptorSet;
    VULKAN_BUFFER UniformBuffer;
    PVULKAN_OBJECT_UNIFORM UniformBufferAddress;
})

/// @brief Information about a GPU
PURPL_MAKE_TAG(struct, VULKAN_GPU_INFO, {
    VkPhysicalDevice Device;

    VkPhysicalDeviceProperties Properties;
    VkPhysicalDeviceMemoryProperties MemoryProperties;

    VkSurfaceCapabilitiesKHR SurfaceCapabilities;
    VkSurfaceFormatKHR *SurfaceFormats;

    VkPresentModeKHR *PresentModes;

    VkQueueFamilyProperties *QueueFamilyProperties;
    UINT32 GraphicsFamilyIndex;
    UINT32 PresentFamilyIndex;

    VkExtensionProperties *ExtensionProperties;

    BOOLEAN Usable;
})

/// @brief An image and related data/objects
PURPL_MAKE_TAG(struct, VULKAN_IMAGE, {
    VkImage Handle;
    VmaAllocation Allocation;
    VkImageView View;
    VkFormat Format;
})

/// @brief Vulkan data
PURPL_MAKE_TAG(struct, VULKAN_DATA, {
    /// @brief Instance
    VkInstance Instance;

    /// @brief Debug messenger
    VkDebugUtilsMessengerEXT DebugMessenger;

    /// @brief Surface
    VkSurfaceKHR Surface;

    /// @brief List of GPUs
    PVULKAN_GPU_INFO Gpus;

    /// @brief Current GPU
    PVULKAN_GPU_INFO Gpu;

    /// @brief Name of current GPU
    PCHAR GpuName;

    /// @brief Index of current GPU
    UINT32 GpuIndex;

    /// @brief Logical device
    VkDevice Device;

    /// @brief VMA allocator
    VmaAllocator Allocator;

    /// @brief Graphics queue
    VkQueue GraphicsQueue;

    /// @brief Present queue
    VkQueue PresentQueue;

    /// @brief Main command pool
    VkCommandPool CommandPool;

    /// @brief Command pool for transfers
    VkCommandPool TransferCommandPool;

    /// @brief Frame command buffers
    VkCommandBuffer CommandBuffers[VULKAN_FRAME_COUNT];

    /// @brief Fences for the command buffers
    VkFence CommandBufferFences[VULKAN_FRAME_COUNT];

    /// @brief Semaphores for frame acquisition
    VkSemaphore AcquireSemaphores[VULKAN_FRAME_COUNT];

    /// @brief Semaphores for completed frames
    VkSemaphore RenderCompleteSemaphores[VULKAN_FRAME_COUNT];

    /// @brief Swap chain
    VkSwapchainKHR SwapChain;

    /// @brief Swap chain format
    VkSurfaceFormatKHR SurfaceFormat;

    /// @brief Swap chain present mode
    VkPresentModeKHR PresentMode;

    /// @brief Swap chain resolution
    VkExtent2D SwapChainExtent;

    /// @brief Images in the swap chain
    VkImage *SwapChainImages;

    /// @brief Views of the images in the swap chain
    VkImageView *SwapChainImageViews;

    /// @brief The current index in the swap chain
    UINT32 SwapChainIndex;

    /// @brief Color render target
    VULKAN_IMAGE ColorTarget;

    /// @brief Depth render target
    VULKAN_IMAGE DepthTarget;

    /// @brief Main render pass, outputs to the screen
    VkRenderPass MainRenderPass;

    /// @brief Framebuffers for the screen, tied to swap chain images
    VkFramebuffer *ScreenFramebuffers;

    /// @brief Main descriptor pool
    VkDescriptorPool DescriptorPool;

    /// @brief Scene descriptor set layout
    VkDescriptorSetLayout SceneDescriptorLayout;

    /// @brief Object descriptor set layout
    VkDescriptorSetLayout ObjectDescriptorLayout;

    /// @brief Pipeline layout
    VkPipelineLayout PipelineLayout;

    /// @brief Scene descriptor set, binds main uniform buffer
    VkDescriptorSet SceneDescriptorSet;

    /// @brief Pipeline cache
    VkPipelineCache PipelineCache;

    /// @brief Current frame index
    UINT8 FrameIndex;

    /// @brief Whether Vulkan is initialized
    BOOLEAN Initialized;

    /// @brief Whether the swap chain was recreated
    BOOLEAN Resized;

    /// @brief Uniform buffer
    VULKAN_BUFFER UniformBuffer;

    /// @brief Address where uniform buffer is mapped
    PVULKAN_SCENE_UNIFORM UniformBufferAddress;

    /// @brief Sampler
    VkSampler Sampler;
})

extern VULKAN_DATA VlkData;

/// @brief Create the instance
extern VOID VlkCreateInstance(VOID);

/// @brief Get allocation callbacks
///
/// @return The allocation callbacks
extern CONST VkAllocationCallbacks *VlkGetAllocationCallbacks(VOID);

/// @brief Get the name of a result
///
/// @param[in] Result The result to get the name of
extern PCSTR VlkGetResultString(VkResult Result);

/// @brief Set an object's name
///
/// @param[in] Object The Vulkan object to name
/// @param[in] ObjectType The type of the object
/// @param[in] Name The name to set (supports printf-style formatting)
/// @param[in] ... Formatting arguments
extern VOID VlkSetObjectName(_In_ UINT64 Object, _In_ VkObjectType ObjectType, _In_z_ _Printf_format_string_ PCSTR Name,
                             ...);

/// @brief Debug callback to log things
///
/// @param[in] MessageSeverity The severity of the message
/// @param[in] MessageTypes The types of the message
/// @param[in] CallbackData Callback data
/// @param[in] UserData Not used
///
/// @return Always returns TRUE
extern VkBool32 VKAPI_CALL VlkDebugCallback(_In_ VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
                                            _In_ VkDebugUtilsMessageTypeFlagsEXT MessageTypes,
                                            _In_ CONST VkDebugUtilsMessengerCallbackDataEXT *CallbackData,
                                            _In_opt_ PVOID UserData);

/// @brief Get devices
extern VOID VlkEnumeratePhysicalDevices(VOID);

/// @brief Create the logical device
extern VOID VlkCreateLogicalDevice(VOID);

/// @brief Create presentation semaphores
extern VOID VlkCreateSemaphores(VOID);

/// @brief Create the main and transfer command pools
extern VOID VlkCreateCommandPools(VOID);

/// @brief Allocate the main command buffers
extern VOID VlkAllocateCommandBuffers(VOID);

/// @brief Allocate a buffer
///
/// @param[in] Size The size of the buffer
/// @param[in] Usage The usage flags of the buffer
/// @param[in] Flags The memory flags of the buffer
/// @param[out] Buffer The buffer
extern VOID VlkAllocateBuffer(_In_ VkDeviceSize Size, _In_ VkBufferUsageFlags Usage, _In_ VkMemoryPropertyFlags Flags,
                              _Out_ PVULKAN_BUFFER Buffer);

/// @brief Allocate a buffer and copy data into it
///
/// @param[in] Data The data to copy into the buffer
/// @param[in] Size The size of the buffer and the data
/// @param[in] Usage The usage of the buffer
/// @param[in] Flags The flags of the buffer
/// @param[out] Buffer The buffer
extern VOID VlkAllocateBufferWithData(_In_ PVOID Data, _In_ VkDeviceSize Size, _In_ VkBufferUsageFlags Usage,
                                      _In_ VkMemoryPropertyFlags Flags, _Out_ PVULKAN_BUFFER Buffer);

/// @brief Name a buffer
extern VOID VlkNameBuffer(_Inout_ PVULKAN_BUFFER Buffer, _In_z_ PCSTR Name, ...);

/// @brief Free a buffer
///
/// @param[in] Buffer The buffer to free
extern VOID VlkFreeBuffer(_Inout_ PVULKAN_BUFFER Buffer);

/// @brief Create a temporary command buffer in the transfer command pool
///
/// @return A command buffer
extern VkCommandBuffer VlkBeginTransfer(VOID);

/// @brief Submits and frees a command buffer from VlkBeginTransfer
///
/// @param[in] TransferBuffer The transfer command buffer from VlkBeginTransfer to
/// submit and free
extern VOID VlkEndTransfer(_In_ VkCommandBuffer TransferBuffer);

/// @brief Copy a buffer to another buffer
///
/// @param[in] Source The source buffer
/// @param[in] Destination The destination buffer
/// @param[in] Size The size of the copy
extern VOID VlkCopyBuffer(_In_ PVULKAN_BUFFER Source, _In_ PVULKAN_BUFFER Destination, _In_ VkDeviceSize Size);

/// @brief Create an image view
///
/// @param[out] ImageView This parameter receives the created image view
/// @param[in] Image The image to create the view for
/// @param[in] Format The format of the image
/// @param[in] Aspect The aspect of the image for the view
extern VOID VlkCreateImageView(_Out_ VkImageView *ImageView, _In_ VkImage Image, _In_ VkFormat Format,
                               _In_ VkImageAspectFlags Aspect);

/// @brief Chooses a format
///
/// @param[in] Formats The list of formats to pick from
/// @param[in] FormatCount The number of formats in the list
/// @param[in] ImageTiling The image tiling
/// @param[in] FormatFeatures The features
///
/// @return A format that can be used
extern VkFormat VlkChooseFormat(_In_ VkFormat *Formats, _In_ UINT32 FormatCount, _In_ VkImageTiling ImageTiling,
                                _In_ VkFormatFeatureFlags FormatFeatures);

/// @brief Changes an image's layout
///
/// @param[in,out] Image The image to transition the layout of
/// @param[in] OldLayout The current layout of the image
/// @param[in] NewLayout The layout to transition it to
extern VOID VlkTransitionImageLayout(_Inout_ VkImage Image, _In_ VkImageLayout OldLayout, _In_ VkImageLayout NewLayout);

/// @brief Copy a buffer into an image
///
/// @param[in] Buffer The buffer to copy into the image
/// @param[out] Image The image to copy the buffer into
/// @param[in] Width The width of the image
/// @param[in] Height The height of the image
extern VOID VlkCopyBufferToImage(_In_ VkBuffer Buffer, _Out_ VkImage Image, _In_ UINT32 Width, _In_ UINT32 Height);

/// @brief Create an image
///
/// @param[in] Width The width of the image
/// @param[in] Height The height of the image
/// @param[in] Format The format of the image
/// @param[in] Layout The layout of the image
/// @param[in] Usage The usage of the image
/// @param[in] MemoryUsage The memory usage of the image (where to store it)
/// @param[in] Aspect The aspect of the image
/// @param[out] Image This parameter receives the created image
extern VOID VlkCreateImage(_In_ UINT32 Width, _In_ UINT32 Height, _In_ VkFormat Format, _In_ VkImageLayout Layout,
                           _In_ VkImageUsageFlags Usage, _In_ VmaMemoryUsage MemoryUsage,
                           _In_ VkImageAspectFlags Aspect, _Out_ PVULKAN_IMAGE Image);

/// @brief Create an initialized image
///
/// @param[in] Data The pixel data for the image
/// @param[in] Size The size of the data
/// @param[in] Width The width of the image
/// @param[in] Height The height of the image
/// @param[in] Format The format of the image
/// @param[in] Layout The layout of the image
/// @param[in] Usage The usage of the image
/// @param[in] MemoryUsage Where to store the image
/// @param[in] Aspect The aspect of the image
/// @param[out] Image This parameter receives the created image
extern VOID VlkCreateImageWithData(_In_ PVOID Data, _In_ VkDeviceSize Size, _In_ UINT32 Width, _In_ UINT32 Height,
                                   _In_ VkFormat Format, _In_ VkImageLayout Layout, _In_ VkImageUsageFlags Usage,
                                   _In_ VmaMemoryUsage MemoryUsage, _In_ VkImageAspectFlags Aspect,
                                   _Out_ PVULKAN_IMAGE Image);

/// @brief Destroy an image
///
/// @param[in,out] Image The image to destroy
extern VOID VlkDestroyImage(_Inout_ PVULKAN_IMAGE Image);

/// @brief Choose a surface format
///
/// @return The surface format
extern VkSurfaceFormatKHR VlkChooseSurfaceFormat(VOID);

/// @brief Choose a present mode
///
/// @return The present mode
extern VkPresentModeKHR VlkChoosePresentMode(VOID);

/// @brief Get the extent of the main surface
///
/// @return The extent of the main surface
extern VkExtent2D VlkGetSurfaceExtent(VOID);

/// @brief Create the main swap chain
extern VOID VlkCreateSwapChain(VOID);

/// @brief Destroy the main swap chain
extern VOID VlkDestroySwapChain(VOID);

/// @brief Create a render pass
///
/// @param[in] Attachments The attachments for the render pass
/// @param[in] AttachmentCount The number of attachments
/// @param[in] Subpasses The subpasses of the render pass
/// @param[in] SubpassCount The number of subpasses
/// @param[in] SubpassDependencies The subpass dependencies for the render pass
/// @param[in] SubpassDependencyCount The number of subpass dependencies
///
/// @return A render pass
extern VkRenderPass VlkCreateRenderPass(_In_ VkAttachmentDescription *Attachments, _In_ UINT32 AttachmentCount,
                                        _In_ VkSubpassDescription *Subpasses, _In_ UINT32 SubpassCount,
                                        _In_ VkSubpassDependency *SubpassDependencies,
                                        _In_ UINT32 SubpassDependencyCount);

/// @brief Create the main render pass
extern VOID VlkCreateMainRenderPass(VOID);

/// @brief Create render targets
extern VOID VlkCreateRenderTargets(VOID);

/// @brief Destroy render targets
extern VOID VlkDestroyRenderTargets(VOID);

/// @brief Create framebuffers for the swap chain images
extern VOID VlkCreateScreenFramebuffers(VOID);

/// @brief Destroy screen framebuffers
extern VOID VlkDestroyScreenFramebuffers(VOID);

/// @brief Create the sampler
extern VOID VlkCreateSampler(VOID);

/// @brief Create the descriptor pool
extern VOID VlkCreateDescriptorPool(VOID);

/// @brief Create the descriptor set layout
extern VOID VlkCreateDescriptorSetLayout(VOID);

/// @brief Create the pipeline layout
extern VOID VlkCreatePipelineLayout(VOID);

/// @brief Create a uniform buffer
extern VOID VlkCreateUniformBuffer(_Out_ PVULKAN_BUFFER UniformBuffer, _Out_ PVOID *UniformBufferAddress,
                                   _In_ SIZE_T Size);

/// @brief Create the scene's descriptor set
extern VOID VlkCreateSceneDescriptorSet(VOID);

/// @brief Load a shader
extern RENDER_HANDLE VlkLoadShader(_In_z_ PCSTR Name);

/// @brief Destroy a shader
extern VOID VlkDestroyShader(_In_ RENDER_HANDLE Shader);

/// @brief Use a texture
extern RENDER_HANDLE VlkUseTexture(_In_ PTEXTURE Texture, _In_z_ PCSTR Name);

/// @brief Destroy a texture
extern VOID VlkDestroyTexture(_In_ RENDER_HANDLE Handle);

/// @brief Create a model (load a mesh onto the GPU)
extern VOID VlkCreateModel(_In_z_ PCSTR Name, _Inout_ PMODEL Model, _In_ PMESH Mesh);

/// @brief Draw a model
extern VOID VlkDrawModel(_In_ PMODEL Model, _In_ PRENDER_OBJECT_UNIFORM Uniform, _In_ PRENDER_OBJECT_DATA Data);

/// @brief Destroy a model
extern VOID VlkDestroyModel(_Inout_ PMODEL Model);

/// @brief Initialize an object
extern VOID VlkInitializeObject(_In_z_ PCSTR Name, _Inout_ PRENDER_OBJECT_DATA Data, _In_ PMODEL Model);

/// @brief Destroy an object
extern VOID VlkDestroyObject(_Inout_ PRENDER_OBJECT_DATA Data);
