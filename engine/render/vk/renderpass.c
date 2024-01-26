/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    backend.c

Abstract:

    This module implements render pass manipulation in Vulkan.

--*/

#include "vk.h"

VkRenderPass
VlkCreateRenderPass(
    _In_ VkAttachmentDescription* Attachments,
    _In_ SIZE_T AttachmentCount,
    _In_ VkSubpassDescription* Subpasses,
    _In_ SIZE_T SubpassCount
    )
{
    LogDebug("Creating render pass with %zu attachment(s) and %zu subpass(es)", AttachmentCount, SubpassCount);

    VkRenderPassCreateInfo RenderPassCreateInformation = {0};
    RenderPassCreateInformation.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    RenderPassCreateInformation.pAttachments = Attachments;
    RenderPassCreateInformation.attachmentCount = AttachmentCount;
    RenderPassCreateInformation.pSubpasses = Subpasses;
    RenderPassCreateInformation.subpassCount = SubpassCount;

    VkRenderPass RenderPass = NULL;
    VULKAN_CHECK(vkCreateRenderPass(
        VlkData.Device,
        &RenderPassCreateInformation,
        NULL,
        &RenderPass
        ));

    return RenderPass;
}

VOID
VlkCreateLightingPass(
    VOID
    )
{
    LogDebug("Creating lighting render pass");

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

    VkSubpassDescription LightingSubpass = {0};
    LightingSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    LightingSubpass.colorAttachmentCount = 1;
    LightingSubpass.pColorAttachments = &ColorAttachmentReference;

    VkSubpassDescription PostProcessSubpass = {0};
    LightingSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    LightingSubpass.colorAttachmentCount = 1;
    LightingSubpass.pColorAttachments = &ColorAttachmentReference;

    VkSubpassDescription Subpasses[] = {
        LightingSubpass,
        PostProcessSubpass
    };

    VlkData.LightingPass = VlkCreateRenderPass(
        &ColorAttachment,
        1,
        Subpasses,
        PURPL_ARRAYSIZE(Subpasses)
        );
}
