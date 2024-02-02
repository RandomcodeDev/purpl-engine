#include "vk.h"

VOID VlkCreateSemaphores(VOID)
{
    VkSemaphoreCreateInfo SemaphoreCreateInformation = {0};
    SemaphoreCreateInformation.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    UINT32 i;

    // Render completion and acquisition semaphores for each frame, plus
    // deferred semaphore
    LogDebug("Creating %d semaphores", VULKAN_FRAME_COUNT * 2 + 1);

    VULKAN_CHECK(vkCreateSemaphore(VlkData.Device, &SemaphoreCreateInformation,
                                   VlkGetAllocationCallbacks(),
                                   &VlkData.DeferredSemaphore));
    VlkSetObjectName(VlkData.DeferredSemaphore, VK_OBJECT_TYPE_SEMAPHORE,
                     "Deferred semaphore");

    for (i = 0; i < VULKAN_FRAME_COUNT; i++)
    {
        VULKAN_CHECK(vkCreateSemaphore(
            VlkData.Device, &SemaphoreCreateInformation,
            VlkGetAllocationCallbacks(), &VlkData.AcquireSemaphores[i]));
        VlkSetObjectName(VlkData.AcquireSemaphores[i], VK_OBJECT_TYPE_SEMAPHORE,
                         "Acquisition semaphore %u", i);
        VULKAN_CHECK(vkCreateSemaphore(
            VlkData.Device, &SemaphoreCreateInformation,
            VlkGetAllocationCallbacks(), &VlkData.RenderCompleteSemaphores[i]));
        VlkSetObjectName(VlkData.RenderCompleteSemaphores[i],
                         VK_OBJECT_TYPE_SEMAPHORE,
                         "Render completion semaphore %u", i);
    }
}
