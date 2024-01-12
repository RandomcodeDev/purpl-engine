#include "vk.h"

VOID
VlkCreateSemaphores(
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
            VlkData.Device,
            &SemaphoreCreateInformation,
            NULL,
            &VlkData.AcquireSemaphores[i]
            ));
        VlkSetObjectName(
            VlkData.AcquireSemaphores[i],
            VK_OBJECT_TYPE_SEMAPHORE,
            "Acquisition semaphore %u",
            i
            );
        VULKAN_CHECK(vkCreateSemaphore(
            VlkData.Device,
            &SemaphoreCreateInformation,
            NULL,
            &VlkData.RenderCompleteSemaphores[i]
            ));
        VlkSetObjectName(
            VlkData.RenderCompleteSemaphores[i],
            VK_OBJECT_TYPE_SEMAPHORE,
            "Render completion semaphore %u",
            i
            );
    }
}