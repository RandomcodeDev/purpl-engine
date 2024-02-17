#include "vk.h"

VkRenderPass VlkCreateRenderPass(_In_ VkAttachmentDescription *Attachments, _In_ SIZE_T AttachmentCount,
                                 _In_ VkSubpassDescription *Subpasses, _In_ SIZE_T SubpassCount,
                                 _In_ VkSubpassDependency *SubpassDependencies, _In_ SIZE_T SubpassDependencyCount)
{
    LogDebug("Creating render pass with %zu attachment(s) and %zu subpass(es)", AttachmentCount, SubpassCount);

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
    MainColorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference MainColorAttachmentReference = {0};
    MainColorAttachmentReference.attachment = 0;
    MainColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription PostProcessColorAttachment = {0};
    PostProcessColorAttachment.format = VlkData.SurfaceFormat.format;
    PostProcessColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    PostProcessColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    PostProcessColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    PostProcessColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    PostProcessColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    PostProcessColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    PostProcessColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference PostProcessColorAttachmentReference = {0};
    PostProcessColorAttachmentReference.attachment = 1;
    PostProcessColorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription DepthStencilAttachment = {0};
    DepthStencilAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    DepthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    DepthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    DepthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    DepthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    DepthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthStencilAttachmentReference = {0};
    DepthStencilAttachmentReference.attachment = 2;
    DepthStencilAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription MainSubpass = {0};
    MainSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    MainSubpass.colorAttachmentCount = 1;
    MainSubpass.pColorAttachments = &MainColorAttachmentReference;
    MainSubpass.pDepthStencilAttachment = &DepthStencilAttachmentReference;

    VkAttachmentReference PostProcessAttachments[] = {MainColorAttachmentReference,
                                                      PostProcessColorAttachmentReference};

    VkSubpassDescription PostProcessSubpass = {0};
    PostProcessSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    PostProcessSubpass.colorAttachmentCount = PURPL_ARRAYSIZE(PostProcessAttachments);
    PostProcessSubpass.pColorAttachments = PostProcessAttachments;
    PostProcessSubpass.pDepthStencilAttachment = &DepthStencilAttachmentReference;

    VkSubpassDependency MainDependency = {0};
    MainDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    MainDependency.srcAccessMask = 0;
    MainDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    MainDependency.dstSubpass = 0;
    MainDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    MainDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkSubpassDependency PostProcessDependency = {0};
    PostProcessDependency.srcSubpass = 0;
    PostProcessDependency.srcAccessMask = 0;
    PostProcessDependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    PostProcessDependency.dstSubpass = 1;
    PostProcessDependency.dstStageMask = VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    PostProcessDependency.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkSubpassDependency EndDependency = {0};
    EndDependency.srcSubpass = 0;
    EndDependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    EndDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    EndDependency.dstSubpass = VK_SUBPASS_EXTERNAL;
    EndDependency.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    EndDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkAttachmentDescription Attachments[] = {MainColorAttachment, PostProcessColorAttachment, DepthStencilAttachment};
    VkSubpassDescription Subpasses[] = {MainSubpass, PostProcessSubpass};
    VkSubpassDependency SubpassDependencies[] = {MainDependency, PostProcessDependency, EndDependency};

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
