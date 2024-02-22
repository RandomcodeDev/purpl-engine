#include "vk.h"

VOID VlkCreateScreenFramebuffers(VOID)
{
    VkImageView Attachments[3] = {0};
    UINT32 i;

    Attachments[2] = VlkData.DepthTarget.View;

    stbds_arrsetlen(VlkData.ScreenFramebuffers, stbds_arrlenu(VlkData.SwapChainImages));

    LogDebug("Creating %d framebuffers", stbds_arrlenu(VlkData.ScreenFramebuffers));

    VkFramebufferCreateInfo FramebufferCreateInformation = {0};
    FramebufferCreateInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferCreateInformation.renderPass = VlkData.MainRenderPass;
    FramebufferCreateInformation.attachmentCount = PURPL_ARRAYSIZE(Attachments);
    FramebufferCreateInformation.pAttachments = Attachments;
    FramebufferCreateInformation.width = RdrGetWidth();
    FramebufferCreateInformation.height = RdrGetHeight();
    FramebufferCreateInformation.layers = 1;

    for (i = 0; i < stbds_arrlenu(VlkData.ScreenFramebuffers); i++)
    {
        Attachments[0] = VlkData.ColorTarget.View;
        Attachments[1] = VlkData.SwapChainImageViews[i];
        VULKAN_CHECK(vkCreateFramebuffer(VlkData.Device, &FramebufferCreateInformation, VlkGetAllocationCallbacks(),
                                         &VlkData.ScreenFramebuffers[i]));
    }
}

VOID VlkDestroyScreenFramebuffers(VOID)
{
    UINT32 i;

    if (!VlkData.ScreenFramebuffers)
    {
        return;
    }

    for (i = 0; i < stbds_arrlenu(VlkData.ScreenFramebuffers); i++)
    {
        if (VlkData.ScreenFramebuffers[i])
        {
            LogDebug("Destroying framebuffer %u/%u", i + 1, stbds_arrlenu(VlkData.ScreenFramebuffers));
            vkDestroyFramebuffer(VlkData.Device, VlkData.ScreenFramebuffers[i], VlkGetAllocationCallbacks());
        }
    }

    stbds_arrfree(VlkData.ScreenFramebuffers);
}
