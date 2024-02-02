#include "vk.h"

VkRenderPass VlkCreateRenderPass(_In_ VkAttachmentDescription *Attachments,
                                 _In_ SIZE_T AttachmentCount,
                                 _In_ VkSubpassDescription *Subpasses,
                                 _In_ SIZE_T SubpassCount,
                                 _In_ VkSubpassDependency *SubpassDependencies,
                                 _In_ SIZE_T SubpassDependencyCount)
{
    LogDebug("Creating render pass with %zu attachment(s) and %zu subpass(es)",
             AttachmentCount, SubpassCount);

    VkRenderPassCreateInfo RenderPassCreateInformation = {0};
    RenderPassCreateInformation.sType =
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInformation.pAttachments = Attachments;
    RenderPassCreateInformation.attachmentCount = AttachmentCount;
    RenderPassCreateInformation.pSubpasses = Subpasses;
    RenderPassCreateInformation.subpassCount = SubpassCount;
    RenderPassCreateInformation.pDependencies = SubpassDependencies;
    RenderPassCreateInformation.dependencyCount = SubpassDependencyCount;

    VkRenderPass RenderPass = NULL;
    VULKAN_CHECK(vkCreateRenderPass(VlkData.Device,
                                    &RenderPassCreateInformation,
                                    VlkGetAllocationCallbacks(), &RenderPass));

    return RenderPass;
}

VOID VlkCreateMainRenderPass(VOID)
{
    LogDebug("Creating main render render pass");

    VkAttachmentDescription ColorAttachment = {0};
    ColorAttachment.format = VlkData.SurfaceFormat.format;
    ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ColorAttachmentReference = {0};
    ColorAttachmentReference.attachment = 0;
    ColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription MainSubpass = {0};
    MainSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    MainSubpass.colorAttachmentCount = 1;
    MainSubpass.pColorAttachments = &ColorAttachmentReference;

    VkSubpassDescription PostProcessSubpass = {0};
    PostProcessSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    PostProcessSubpass.colorAttachmentCount = 1;
    PostProcessSubpass.pColorAttachments = &ColorAttachmentReference;

    VkSubpassDescription Subpasses[] = {MainSubpass, PostProcessSubpass};

    VlkData.MainRenderPass = VlkCreateRenderPass(
        &ColorAttachment, 1, Subpasses, PURPL_ARRAYSIZE(Subpasses), NULL, 0);
}
