/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    backend.c

Abstract:

    This file implements the Vulkan backend.

--*/

#include "vk.h"

//
// Global Vulkan state
//

VULKAN_DATA VlkData;

VOID VlkCreateSurface(VOID)
{
    VlkData.Surface = VidCreateVulkanSurface(VlkData.Instance, VlkGetAllocationCallbacks(), NULL);
}

VOID VlkCreateAllocator(VOID)
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
    AllocatorCreateInformation.pAllocationCallbacks = VlkGetAllocationCallbacks();

    VULKAN_CHECK(vmaCreateAllocator(&AllocatorCreateInformation, &VlkData.Allocator));
}

static VOID Initialize(VOID)
{
    if (VlkData.Initialized)
    {
        return;
    }

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
    VlkCreateMainRenderPass();
    VlkCreateRenderTargets();
    VlkCreateScreenFramebuffers();
    VlkCreateSampler();
    VlkCreateDescriptorPool();
    VlkCreateDescriptorSetLayout();
    VlkCreatePipelineLayout();
    VlkCreateUniformBuffer(&VlkData.UniformBuffer, &VlkData.UniformBufferAddress,
                           sizeof(VULKAN_SCENE_UNIFORM) * VULKAN_FRAME_COUNT);
    VlkCreateSceneDescriptorSet();

    VlkData.FrameIndex = 0;
    VlkData.Initialized = TRUE;
    VlkData.Resized = TRUE;

    LogDebug("Successfully initialized Vulkan backend");
}

static VOID IncrementFrameIndex(VOID)
{
    VlkData.FrameIndex = (VlkData.FrameIndex + 1) % VULKAN_FRAME_COUNT;
}

static VOID Wait(VOID)
{
    // https://stackoverflow.com/questions/70762372/how-to-recreate-swapchain-after-vkacquirenextimagekhr-is-vk-suboptimal-khr
    CONST VkPipelineStageFlags WaitDestinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInformation.waitSemaphoreCount = 1;
    SubmitInformation.pWaitSemaphores = &VlkData.AcquireSemaphores[VlkData.FrameIndex];
    SubmitInformation.pWaitDstStageMask = &WaitDestinationStage;
    vkQueueSubmit(VlkData.PresentQueue, 1, &SubmitInformation, VK_NULL_HANDLE);

    vkDeviceWaitIdle(VlkData.Device);
}

static VOID HandleResize(VOID)
{
    if (!VlkData.Initialized)
    {
        return;
    }

    LogDebug("Handling resize");

    vkDeviceWaitIdle(VlkData.Device);

    VlkDestroyScreenFramebuffers();
    vkDestroyRenderPass(VlkData.Device, VlkData.MainRenderPass, VlkGetAllocationCallbacks());
    VlkDestroyRenderTargets();
    VlkDestroySwapChain();
    VlkCreateSwapChain();
    VlkCreateRenderTargets();
    VlkCreateMainRenderPass();
    VlkCreateScreenFramebuffers();

    Wait();
}

static VOID BeginFrame(_In_ BOOLEAN WindowResized, _In_ PRENDER_SCENE_UNIFORM Uniform)
{
    VkResult Result;
    VkCommandBuffer CurrentCommandBuffer;

    if (!VlkData.Initialized)
    {
        return;
    }

    VULKAN_CHECK(
        vkWaitForFences(VlkData.Device, 1, &VlkData.CommandBufferFences[VlkData.FrameIndex], TRUE, UINT64_MAX));

    VlkData.SwapChainIndex = 0;
    Result =
        vkAcquireNextImageKHR(VlkData.Device, VlkData.SwapChain, UINT64_MAX,
                              VlkData.AcquireSemaphores[VlkData.FrameIndex], VK_NULL_HANDLE, &VlkData.SwapChainIndex);
    if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || WindowResized)
    {
        if (!WindowResized)
        {
            LogDebug("Got %s (VkResult %d) when acquiring next swap chain image", VlkGetResultString(Result), Result);
        }

        HandleResize();

        VULKAN_CHECK(vkAcquireNextImageKHR(VlkData.Device, VlkData.SwapChain, UINT64_MAX,
                                           VlkData.AcquireSemaphores[VlkData.FrameIndex], VK_NULL_HANDLE,
                                           &VlkData.SwapChainIndex));
        VlkData.Resized = TRUE;
    }
    else if (Result != VK_SUCCESS)
    {
        CmnError("Failed to acquire next image: %s (VkResult %d)", VlkGetResultString(Result), Result);
    }

    CurrentCommandBuffer = VlkData.CommandBuffers[VlkData.FrameIndex];

    VULKAN_CHECK(vkResetFences(VlkData.Device, 1, &VlkData.CommandBufferFences[VlkData.FrameIndex]));
    VULKAN_CHECK(vkResetCommandBuffer(VlkData.CommandBuffers[VlkData.FrameIndex], 0));

    VkCommandBufferBeginInfo CommandBufferBeginInformation = {0};
    CommandBufferBeginInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    CommandBufferBeginInformation.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    VULKAN_CHECK(vkBeginCommandBuffer(CurrentCommandBuffer, &CommandBufferBeginInformation));

    VkClearValue ClearValues[3] = {0};

    INT64 ClearColour = CONFIGVAR_GET_INT("rdr_clear_colour");
    ClearValues[0].color.float32[0] = ((ClearColour >> 24) & 0xFF) / 255.0f;
    ClearValues[0].color.float32[1] = ((ClearColour >> 16) & 0xFF) / 255.0f;
    ClearValues[0].color.float32[2] = ((ClearColour >> 8) & 0xFF) / 255.0f;
    ClearValues[0].color.float32[3] = ((ClearColour >> 0) & 0xFF) / 255.0f;

    ClearValues[1].depthStencil.depth = 1.0f;
    ClearValues[1].depthStencil.stencil = 1.0f;

    VkRect2D Scissor = {0};
    Scissor.extent.width = RdrGetWidth();
    Scissor.extent.height = RdrGetHeight();

    VkRenderPassBeginInfo RenderPassBeginInformation = {0};
    RenderPassBeginInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    RenderPassBeginInformation.renderPass = VlkData.MainRenderPass;
    RenderPassBeginInformation.framebuffer = VlkData.ScreenFramebuffers[VlkData.SwapChainIndex];
    RenderPassBeginInformation.pClearValues = ClearValues;
    RenderPassBeginInformation.clearValueCount = PURPL_ARRAYSIZE(ClearValues);
    RenderPassBeginInformation.renderArea = Scissor;

    vkCmdBeginRenderPass(CurrentCommandBuffer, &RenderPassBeginInformation, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport Viewport = {0};
    Viewport.width = (FLOAT)Scissor.extent.width;
    Viewport.height = (FLOAT)Scissor.extent.height;
    Viewport.minDepth = 0.0f;
    Viewport.maxDepth = 1.0f;

    vkCmdSetScissor(CurrentCommandBuffer, 0, 1, &Scissor);
    vkCmdSetViewport(CurrentCommandBuffer, 0, 1, &Viewport);
    vkCmdSetPrimitiveTopology(CurrentCommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

    VULKAN_SET_UNIFORM(VlkData.UniformBufferAddress, Uniform);
}

static VOID EndFrame(VOID)
{
    VkResult Result;
    VkCommandBuffer CurrentCommandBuffer;

    if (!VlkData.Initialized)
    {
        return;
    }

    if (VlkData.Resized)
    {
        VlkData.Resized = FALSE;
    }

    CurrentCommandBuffer = VlkData.CommandBuffers[VlkData.FrameIndex];

    vkCmdEndRenderPass(CurrentCommandBuffer);
    VULKAN_CHECK(vkEndCommandBuffer(CurrentCommandBuffer));

    VkSubmitInfo SubmitInformation = {0};
    SubmitInformation.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags WaitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    SubmitInformation.pWaitDstStageMask = &WaitStage;

    SubmitInformation.pWaitSemaphores = &VlkData.AcquireSemaphores[VlkData.FrameIndex];
    SubmitInformation.waitSemaphoreCount = 1;
    SubmitInformation.pSignalSemaphores = &VlkData.RenderCompleteSemaphores[VlkData.FrameIndex];
    SubmitInformation.signalSemaphoreCount = 1;
    SubmitInformation.pCommandBuffers = &CurrentCommandBuffer;
    SubmitInformation.commandBufferCount = 1;

    VULKAN_CHECK(
        vkQueueSubmit(VlkData.GraphicsQueue, 1, &SubmitInformation, VlkData.CommandBufferFences[VlkData.FrameIndex]));

    VkPresentInfoKHR PresentInformation = {0};
    PresentInformation.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    PresentInformation.pSwapchains = &VlkData.SwapChain;
    PresentInformation.swapchainCount = 1;
    PresentInformation.pWaitSemaphores = &VlkData.RenderCompleteSemaphores[VlkData.FrameIndex];
    PresentInformation.waitSemaphoreCount = 1;
    PresentInformation.pImageIndices = &VlkData.SwapChainIndex;

    Result = vkQueuePresentKHR(VlkData.PresentQueue, &PresentInformation);
    if (Result != VK_ERROR_OUT_OF_DATE_KHR && Result != VK_SUCCESS)
    {
        CmnError("Failed to present frame: %s", VlkGetResultString(Result));
    }

    IncrementFrameIndex();
}

static VOID FinishRendering(VOID)
{
    vkDeviceWaitIdle(VlkData.Device);
}

static VOID Shutdown(VOID)
{
    UINT32 i;

    LogDebug("Shutting down Vulkan");
    VlkData.Initialized = FALSE;
    vkDeviceWaitIdle(VlkData.Device);

    if (VlkData.PipelineLayout)
    {
        LogDebug("Destroying pipeline layout 0x%llX", (UINT64)VlkData.PipelineLayout);
        vkDestroyPipelineLayout(VlkData.Device, VlkData.PipelineLayout, VlkGetAllocationCallbacks());
        VlkData.PipelineLayout = VK_NULL_HANDLE;
    }

    if (VlkData.ObjectDescriptorLayout)
    {
        LogDebug("Destroying descriptor set layout 0x%llX", (UINT64)VlkData.ObjectDescriptorLayout);
        vkDestroyDescriptorSetLayout(VlkData.Device, VlkData.ObjectDescriptorLayout, VlkGetAllocationCallbacks());
        VlkData.ObjectDescriptorLayout = VK_NULL_HANDLE;
    }

    if (VlkData.SceneDescriptorLayout)
    {
        LogDebug("Destroying descriptor set layout 0x%llX", (UINT64)VlkData.SceneDescriptorLayout);
        vkDestroyDescriptorSetLayout(VlkData.Device, VlkData.SceneDescriptorLayout, VlkGetAllocationCallbacks());
        VlkData.SceneDescriptorLayout = VK_NULL_HANDLE;
    }

    VlkDestroyScreenFramebuffers();

    VlkDestroyRenderTargets();

    if (VlkData.MainRenderPass)
    {
        LogDebug("Destroying main render pass 0x%llX", (UINT64)VlkData.MainRenderPass);
        vkDestroyRenderPass(VlkData.Device, VlkData.MainRenderPass, VlkGetAllocationCallbacks());
        VlkData.MainRenderPass = VK_NULL_HANDLE;
    }

    if (VlkData.Sampler)
    {
        LogDebug("Destroying sampler 0x%llX", (UINT64)VlkData.Sampler);
        vkDestroySampler(VlkData.Device, VlkData.Sampler, VlkGetAllocationCallbacks());
        VlkData.Sampler = VK_NULL_HANDLE;
    }

    LogDebug("Freeing uniform buffer");
    vmaUnmapMemory(VlkData.Allocator, VlkData.UniformBuffer.Allocation);
    VlkFreeBuffer(&VlkData.UniformBuffer);

    if (VlkData.DescriptorPool)
    {
        LogDebug("Destroying descriptor pool 0x%llX", (UINT64)VlkData.DescriptorPool);
        vkDestroyDescriptorPool(VlkData.Device, VlkData.DescriptorPool, VlkGetAllocationCallbacks());
        VlkData.DescriptorPool = VK_NULL_HANDLE;
    }

    if (VlkData.Sampler)
    {
        LogDebug("Destroying sampler 0x%llX", (UINT64)VlkData.Sampler);
        vkDestroySampler(VlkData.Device, VlkData.Sampler, VlkGetAllocationCallbacks());
    }

    VlkDestroySwapChain();

    LogDebug("Destroying command fences");
    for (i = 0; i < VULKAN_FRAME_COUNT; i++)
    {
        if (VlkData.CommandBufferFences[i])
        {
            vkDestroyFence(VlkData.Device, VlkData.CommandBufferFences[i], VlkGetAllocationCallbacks());
            VlkData.CommandBufferFences[i] = VK_NULL_HANDLE;
        }
    }

    if (VlkData.TransferCommandPool)
    {
        LogDebug("Destroying transfer command pool 0x%llX", (UINT64)VlkData.TransferCommandPool);
        vkDestroyCommandPool(VlkData.Device, VlkData.TransferCommandPool, VlkGetAllocationCallbacks());
        VlkData.TransferCommandPool = VK_NULL_HANDLE;
    }

    if (VlkData.CommandPool)
    {
        LogDebug("Destroying command pool 0x%llX", (UINT64)VlkData.CommandPool);
        vkDestroyCommandPool(VlkData.Device, VlkData.CommandPool, VlkGetAllocationCallbacks());
        VlkData.CommandPool = VK_NULL_HANDLE;
    }

    for (i = 0; i < VULKAN_FRAME_COUNT; i++)
    {
        if (VlkData.AcquireSemaphores[i])
        {
            LogDebug("Destroying acquisition semaphore %u/%u", i + 1, VULKAN_FRAME_COUNT);
            vkDestroySemaphore(VlkData.Device, VlkData.AcquireSemaphores[i], VlkGetAllocationCallbacks());
            VlkData.AcquireSemaphores[i] = VK_NULL_HANDLE;
        }
        if (VlkData.RenderCompleteSemaphores[i])
        {
            LogDebug("Destroying render completion semaphore %u/%u", i + 1, VULKAN_FRAME_COUNT);
            vkDestroySemaphore(VlkData.Device, VlkData.RenderCompleteSemaphores[i], VlkGetAllocationCallbacks());
            VlkData.RenderCompleteSemaphores[i] = VK_NULL_HANDLE;
        }
    }

    if (VlkData.Surface)
    {
        LogDebug("Destroying VkSurfaceKHR 0x%llX", (UINT64)VlkData.Surface);
        vkDestroySurfaceKHR(VlkData.Instance, VlkData.Surface, VlkGetAllocationCallbacks());
        VlkData.Surface = VK_NULL_HANDLE;
    }

    if (VlkData.Allocator)
    {
        LogDebug("Destroying VmaAllocator 0x%llX", (UINT64)VlkData.Device);
        vmaDestroyAllocator(VlkData.Allocator);
        VlkData.Allocator = VK_NULL_HANDLE;
    }

    if (VlkData.Device)
    {
        LogDebug("Destroying VkDevice 0x%llX", (UINT64)VlkData.Device);
        vkDestroyDevice(VlkData.Device, VlkGetAllocationCallbacks());
        VlkData.Device = VK_NULL_HANDLE;
    }

    if (VlkData.Gpus)
    {
        LogDebug("Freeing GPU list");
        for (i = 0; i < stbds_arrlenu(VlkData.Gpus); i++)
        {
            if (VlkData.Gpus[i].QueueFamilyProperties)
            {
                stbds_arrfree(VlkData.Gpus[i].QueueFamilyProperties);
            }
            if (VlkData.Gpus[i].ExtensionProperties)
            {
                stbds_arrfree(VlkData.Gpus[i].ExtensionProperties);
            }
            if (VlkData.Gpus[i].SurfaceFormats)
            {
                stbds_arrfree(VlkData.Gpus[i].SurfaceFormats);
            }
            if (VlkData.Gpus[i].PresentModes)
            {
                stbds_arrfree(VlkData.Gpus[i].PresentModes);
            }
            memset(&VlkData.Gpus[i], 0, sizeof(VULKAN_GPU_INFO));
        }
        VlkData.Gpu = NULL;
        stbds_arrfree(VlkData.Gpus);
        VlkData.Gpus = NULL;
    }

#if defined PURPL_VULKAN_DEBUG && !defined PURPL_SWITCH
    if (VlkData.DebugMessenger)
    {
        LogDebug("Destroying VkDebugUtilsMessengerEXT 0x%llX", (UINT64)VlkData.DebugMessenger);
        vkDestroyDebugUtilsMessengerEXT(VlkData.Instance, VlkData.DebugMessenger, VlkGetAllocationCallbacks());
    }
#endif

    if (VlkData.Instance)
    {
        LogDebug("Destroying VkInstance 0x%llX", (UINT64)VlkData.Instance);
        vkDestroyInstance(VlkData.Instance, VlkGetAllocationCallbacks());
        VlkData.Instance = VK_NULL_HANDLE;
    }

    memset(&VlkData, 0, sizeof(VULKAN_DATA));

    LogDebug("Successfully shut down Vulkan");
}

PCSTR GetGpuName(VOID)
{
    return VlkData.GpuName;
}

VOID VlkInitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    LogDebug("Filling out render backend for Vulkan");

    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->FinishRendering = FinishRendering;
    Backend->Shutdown = Shutdown;

    Backend->LoadShader = VlkLoadShader;
    Backend->DestroyShader = VlkDestroyShader;

    Backend->UseTexture = VlkUseTexture;
    Backend->ReleaseTexture = VlkDestroyTexture;

    Backend->CreateModel = VlkCreateModel;
    Backend->DrawModel = VlkDrawModel;
    Backend->DestroyModel = VlkDestroyModel;

    Backend->InitializeObject = VlkInitializeObject;
    Backend->DestroyObject = VlkDestroyObject;

    Backend->GetGpuName = GetGpuName;

    memset(&VlkData, 0, sizeof(VULKAN_DATA));
}
