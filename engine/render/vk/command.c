#include "vk.h"

VOID VlkCreateCommandPools(VOID)
{
    VkCommandPoolCreateInfo CommandPoolCreateInformation = {0};

    LogDebug("Creating command pools");

    CommandPoolCreateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    CommandPoolCreateInformation.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    CommandPoolCreateInformation.queueFamilyIndex = VlkData.Gpu->GraphicsFamilyIndex;

    LogTrace("Creating primary command pool");
    VULKAN_CHECK(vkCreateCommandPool(VlkData.Device, &CommandPoolCreateInformation, VlkGetAllocationCallbacks(),
                                     &VlkData.CommandPool));
    VlkSetObjectName(VlkData.CommandPool, VK_OBJECT_TYPE_COMMAND_POOL, "Command pool");

    CommandPoolCreateInformation.flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

    LogTrace("Creating transfer command pool");
    VULKAN_CHECK(vkCreateCommandPool(VlkData.Device, &CommandPoolCreateInformation, VlkGetAllocationCallbacks(),
                                     &VlkData.TransferCommandPool));
    VlkSetObjectName(VlkData.TransferCommandPool, VK_OBJECT_TYPE_COMMAND_POOL, "Transfer command pool");
}

VOID VlkAllocateCommandBuffers(VOID)
{
    VkCommandBufferAllocateInfo CommandBufferAllocateInformation = {0};
    UINT32 i;

    LogDebug("Allocating command buffers");

    CommandBufferAllocateInformation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInformation.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInformation.commandPool = VlkData.CommandPool;
    CommandBufferAllocateInformation.commandBufferCount = VULKAN_FRAME_COUNT;

    VULKAN_CHECK(vkAllocateCommandBuffers(VlkData.Device, &CommandBufferAllocateInformation, VlkData.CommandBuffers));
    for (i = 0; i < VULKAN_FRAME_COUNT; i++)
    {
        VlkSetObjectName(VlkData.CommandBuffers[i], VK_OBJECT_TYPE_COMMAND_BUFFER, "Command buffer %u", i);
    }

    LogDebug("Creating command buffer fences");

    VkFenceCreateInfo FenceCreateInformation = {0};
    FenceCreateInformation.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceCreateInformation.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (i = 0; i < VULKAN_FRAME_COUNT; i++)
    {
        VULKAN_CHECK(vkCreateFence(VlkData.Device, &FenceCreateInformation, VlkGetAllocationCallbacks(),
                                   &VlkData.CommandBufferFences[i]));
        VlkSetObjectName(VlkData.CommandBufferFences[i], VK_OBJECT_TYPE_FENCE, "Command buffer fence %u", i);
    }
}
