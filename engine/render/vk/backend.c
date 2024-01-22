/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    backend.c

Abstract:

    This module implements the Vulkan backend.

--*/

#include "vk.h"

//
// Global Vulkan state
//

VULKAN_DATA VlkData;

VOID
VlkCreateSurface(
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
    VlkData.Surface = PlatCreateVulkanSurface(
        VlkData.Instance,
        NULL
        );
}

VOID
VlkCreateAllocator(
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

    AllocatorCreateInformation.instance = VlkData.Instance;
    AllocatorCreateInformation.physicalDevice = VlkData.Gpu->Device;
    AllocatorCreateInformation.device = VlkData.Device;
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
        &VlkData.Allocator
        ));
}

static
VOID
Initialize(
    VOID
    )
{
    LogDebug("Initializing Vulkan backend");

    LogDebug("Initializing volk");
#ifdef PURPL_SWITCH
    volkInitializeCustom(PlatGetVulkanFunction);
#else
    volkInitialize();
#endif

    VlkCreateInstance();
    VlkCreateSurface();
    VlkEnumeratePhysicalDevices();
    VlkCreateLogicalDevice();
    VlkCreateSemaphores();
    VlkCreateCommandPools();
    VlkAllocateCommandBuffers();
    VlkCreateAllocator();
    VlkCreateSwapChain();

    VlkData.FrameIndex = 0;
    VlkData.Initialized = TRUE;

    LogDebug("Successfully initialized Vulkan backend");
}

static
VOID
IncrementFrameIndex(
    VOID
    )
{
    VlkData.FrameIndex = (VlkData.FrameIndex + 1) % VULKAN_FRAME_COUNT;
}

static
VOID
BeginFrame(
    VOID
    )
{
    if ( !VlkData.Initialized )
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
    if ( !VlkData.Initialized )
    {
        return;
    }
}

static
VOID
Shutdown(
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
    VlkData.Initialized = FALSE;
    vkDeviceWaitIdle(VlkData.Device);

    if ( VlkData.Sampler )
    {
        LogDebug("Destroying sampler 0x%llX", (UINT64)VlkData.Sampler);
        vkDestroySampler(
            VlkData.Device,
            VlkData.Sampler,
            NULL
            );
        VlkData.Sampler = NULL;
    }

    LogDebug("Freeing uniform buffers");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        VlkFreeBuffer(&VlkData.UniformBuffers[i]);
    }

    if ( VlkData.DescriptorPool )
    {
        LogDebug("Destroying descriptor pool 0x%llX", (UINT64)VlkData.DescriptorPool);
        vkDestroyDescriptorPool(
            VlkData.Device,
            VlkData.DescriptorPool,
            NULL
            );
        VlkData.DescriptorPool = NULL;
    }

    VlkDestroySwapChain();

    LogDebug("Destroying command fences");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        if ( VlkData.CommandBufferFences[i] )
        {
            vkDestroyFence(
                VlkData.Device,
                VlkData.CommandBufferFences[i],
                NULL
                );
            VlkData.CommandBufferFences[i] = NULL;
        }
    }

    if ( VlkData.TransferCommandPool )
    {
        LogDebug("Destroying transfer command pool 0x%llX", (UINT64)VlkData.TransferCommandPool);
        vkDestroyCommandPool(
            VlkData.Device,
            VlkData.TransferCommandPool,
            NULL
            );
        VlkData.TransferCommandPool = NULL;
    }

    if ( VlkData.CommandPool )
    {
        LogDebug("Destroying command pool 0x%llX", (UINT64)VlkData.CommandPool);
        vkDestroyCommandPool(
            VlkData.Device,
            VlkData.CommandPool,
            NULL
            );
        VlkData.CommandPool = NULL;
    }

    LogDebug("Destroying semaphores");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        if ( VlkData.AcquireSemaphores[i] )
        {
            vkDestroySemaphore(
                VlkData.Device,
                VlkData.AcquireSemaphores[i],
                NULL
                );
            VlkData.AcquireSemaphores[i] = NULL;
        }
        if ( VlkData.RenderCompleteSemaphores[i] )
        {
            vkDestroySemaphore(
                VlkData.Device,
                VlkData.RenderCompleteSemaphores[i],
                NULL
                );
            VlkData.RenderCompleteSemaphores[i] = NULL;
        }
    }

    if ( VlkData.Surface )
    {
        LogDebug("Destroying VkSurfaceKHR 0x%llX", (UINT64)VlkData.Surface);
        vkDestroySurfaceKHR(
            VlkData.Instance,
            VlkData.Surface,
            NULL
            );
        VlkData.Surface = NULL;
    }

    if ( VlkData.Allocator )
    {
        LogDebug("Destroying VmaAllocator 0x%llX", (UINT64)VlkData.Device);
        vmaDestroyAllocator(VlkData.Allocator);
        VlkData.Allocator = NULL;
    }

    if ( VlkData.Device )
    {
        LogDebug("Destroying VkDevice 0x%llX", (UINT64)VlkData.Device);
        vkDestroyDevice(
            VlkData.Device,
            NULL
            );
        VlkData.Device = NULL;
    }

    if ( VlkData.Gpus )
    {
        LogDebug("Freeing GPU list");
        for ( i = 0; i < stbds_arrlenu(VlkData.Gpus); i++ )
        {
            if ( VlkData.Gpus[i].QueueFamilyProperties )
            {
                stbds_arrfree(VlkData.Gpus[i].QueueFamilyProperties);
            }
            if ( VlkData.Gpus[i].ExtensionProperties )
            {
                stbds_arrfree(VlkData.Gpus[i].ExtensionProperties);
            }
            if ( VlkData.Gpus[i].SurfaceFormats )
            {
                stbds_arrfree(VlkData.Gpus[i].SurfaceFormats);
            }
            if ( VlkData.Gpus[i].PresentModes )
            {
                stbds_arrfree(VlkData.Gpus[i].PresentModes);
            }
            memset(
                &VlkData.Gpus[i],
                0,
                sizeof(VULKAN_GPU_INFO)
                );
        }
        VlkData.Gpu = NULL;
        stbds_arrfree(VlkData.Gpus);
        VlkData.Gpus = NULL;
    }

    if ( VlkData.Instance )
    {
        LogDebug("Destroying VkInstance 0x%llX", (UINT64)VlkData.Instance);
        vkDestroyInstance(
            VlkData.Instance,
            NULL
            );
        VlkData.Instance = NULL;
    }

    memset(
        &VlkData,
        0,
        sizeof(VULKAN_DATA)
        );
    LogDebug("Successfully shut down Vulkan");
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
