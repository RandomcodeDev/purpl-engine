#include "vk.h"

VOID VlkCreateScreenFramebuffers(VOID)
{
    VkImageView Attachments[3] = {0};
    UINT32 i;

    Attachments[2] = VlkData.DepthTarget.View;

    LogDebug("Creating %d framebuffers", VULKAN_FRAME_COUNT);

    VkFramebufferCreateInfo FramebufferCreateInformation = {0};
    FramebufferCreateInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferCreateInformation.renderPass = VlkData.MainRenderPass;
    FramebufferCreateInformation.attachmentCount = PURPL_ARRAYSIZE(Attachments);
    FramebufferCreateInformation.pAttachments = Attachments;
    FramebufferCreateInformation.width = RdrGetWidth();
    FramebufferCreateInformation.height = RdrGetHeight();
    FramebufferCreateInformation.layers = 1;

    for (i = 0; i < VULKAN_FRAME_COUNT; i++)
    {
        Attachments[0] = VlkData.SwapChainImageViews[i];
        Attachments[1] = VlkData.ColorTarget.View;
        VULKAN_CHECK(vkCreateFramebuffer(VlkData.Device, &FramebufferCreateInformation, VlkGetAllocationCallbacks(),
                                         &VlkData.ScreenFramebuffers[i]));
    }
}

VOID VlkDestroyScreenFramebuffers(VOID)
{
    UINT32 i;

    LogDebug("Destroying framebuffers");
    for (i = 0; i < VULKAN_FRAME_COUNT; i++)
    {
        if (VlkData.ScreenFramebuffers[i])
        {
            vkDestroyFramebuffer(VlkData.Device, VlkData.ScreenFramebuffers[i], VlkGetAllocationCallbacks());
        }
    }
}
