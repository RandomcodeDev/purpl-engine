#include "vk.h"

VkRenderPass VlkCreateRenderPass(_In_ VkAttachmentDescription *Attachments, _In_ UINT32 AttachmentCount,
                                 _In_ VkSubpassDescription *Subpasses, _In_ UINT32 SubpassCount,
                                 _In_ VkSubpassDependency *SubpassDependencies, _In_ UINT32 SubpassDependencyCount)
{
    LogDebug("Creating render pass with %u attachment(s), %u subpass(es), and %u subpass dependenc(ies)",
             AttachmentCount, SubpassCount, SubpassDependencyCount);

    VkRenderPassCreateInfo RenderPassCreateInformation = {0};
    RenderPassCreateInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInformation.pAttachments = Attachments;
    RenderPassCreateInformation.attachmentCount = AttachmentCount;
    RenderPassCreateInformation.pSubpasses = Subpasses;
    RenderPassCreateInformation.subpassCount = SubpassCount;
    RenderPassCreateInformation.pDependencies = SubpassDependencies;
    RenderPassCreateInformation.dependencyCount = SubpassDependencyCount;

    VkRenderPass RenderPass = VK_NULL_HANDLE;
    VULKAN_CHECK(
        vkCreateRenderPass(VlkData.Device, &RenderPassCreateInformation, VlkGetAllocationCallbacks(), &RenderPass));

    return RenderPass;
}

VOID VlkCreateMainRenderPass(VOID)
{
    LogDebug("Creating main render render pass");

    VkAttachmentDescription MainColorAttachment = {0};
    MainColorAttachment.format = VlkData.SurfaceFormat.format;
    MainColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    MainColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    MainColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    MainColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    MainColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    MainColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    MainColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference MainColorAttachmentReference = {0};
    MainColorAttachmentReference.attachment = 0;
    MainColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription DepthStencilAttachment = {0};
    DepthStencilAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    DepthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    DepthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    DepthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthStencilAttachmentReference = {0};
    DepthStencilAttachmentReference.attachment = 1;
    DepthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription MainSubpass = {0};
    MainSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    MainSubpass.colorAttachmentCount = 1;
    MainSubpass.pColorAttachments = &MainColorAttachmentReference;
    MainSubpass.pDepthStencilAttachment = &DepthStencilAttachmentReference;

    VkSubpassDependency MainDependency = {0};
    MainDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    MainDependency.srcAccessMask = 0;
    MainDependency.srcStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    MainDependency.dstSubpass = 0;
    MainDependency.dstStageMask =
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    MainDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkAttachmentDescription Attachments[] = {MainColorAttachment, DepthStencilAttachment};
    VkSubpassDescription Subpasses[] = {MainSubpass};
    VkSubpassDependency SubpassDependencies[] = {MainDependency};

    VlkData.MainRenderPass =
        VlkCreateRenderPass(Attachments, PURPL_ARRAYSIZE(Attachments), Subpasses, PURPL_ARRAYSIZE(Subpasses),
                            SubpassDependencies, PURPL_ARRAYSIZE(SubpassDependencies));
}

VOID VlkCreateRenderTargets(VOID)
{
    LogDebug("Creating color image");
    VlkCreateImage(RdrGetWidth(), RdrGetHeight(), VlkData.SurfaceFormat.format,
                   VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                   VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_ASPECT_COLOR_BIT, &VlkData.ColorTarget);

    LogDebug("Creating depth image");
    VlkCreateImage(RdrGetWidth(), RdrGetHeight(), VK_FORMAT_D32_SFLOAT_S8_UINT,
                   VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                   VMA_MEMORY_USAGE_GPU_ONLY, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_DEPTH_BIT,
                   &VlkData.DepthTarget);
}

VOID VlkDestroyRenderTargets(VOID)
{
    LogDebug("Destroying depth image");
    VlkDestroyImage(&VlkData.DepthTarget);

    LogDebug("Destroying color image");
    VlkDestroyImage(&VlkData.ColorTarget);
}
