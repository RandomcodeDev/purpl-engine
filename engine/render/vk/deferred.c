/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    deferred.c

Abstract:

    This module implements creation and destruction of deferred render passes.
    It's based on this code:
    https://github.com/SaschaWillems/Vulkan/blob/master/examples/deferred/deferred.cpp

--*/

#include "vk.h"

VOID
VlkCreateDeferredPass(
    _Out_ PVULKAN_DEFERRED_PASS DeferredPass
    )
{
    UINT32 i;

    LogDebug("Creating deferred render pass");
   
    memset(
        DeferredPass,
        0,
        sizeof(VULKAN_DEFERRED_PASS)
        );

    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &DeferredPass->PositionBuffer
        );
    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &DeferredPass->NormalBuffer
        );
    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &DeferredPass->AlbedoBuffer
        );
    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &DeferredPass->DepthBuffer
        );

    VkAttachmentDescription Attachments[4] = {0};
    for ( i = 0; i < PURPL_ARRAYSIZE(Attachments); i++ )
    {
        Attachments[i].samples = VK_SAMPLE_COUNT_1_BIT;
        Attachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        Attachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        Attachments[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        Attachments[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        Attachments[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    }

    // Position
    Attachments[0].format = DeferredPass->PositionBuffer.Format;
    Attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Normal
    Attachments[1].format = DeferredPass->NormalBuffer.Format;
    Attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Albedo
    Attachments[2].format = DeferredPass->AlbedoBuffer.Format;
    Attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Depth
    Attachments[3].format = DeferredPass->DepthBuffer.Format;
    Attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ColorReferences[3] = {0};
    ColorReferences[0].attachment = 0;
    ColorReferences[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ColorReferences[1].attachment = 1;
    ColorReferences[1].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    ColorReferences[2].attachment = 2;
    ColorReferences[2].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference DepthReference = {0};
    DepthReference.attachment = 3;
    DepthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription Subpass = {0};
    Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    Subpass.pColorAttachments = ColorReferences;
    Subpass.colorAttachmentCount = PURPL_ARRAYSIZE(ColorReferences);
    Subpass.pDepthStencilAttachment = &DepthReference;

    // These are used to transition the layouts of the attachments
    VkSubpassDependency SubpassDependencies[2] = {0};
    SubpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependencies[0].dstSubpass = 0;
	SubpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	SubpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	SubpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	SubpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	SubpassDependencies[1].srcSubpass = 0;
	SubpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	SubpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	SubpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	SubpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	SubpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	SubpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    DeferredPass->RenderPass = VlkCreateRenderPass(
        Attachments,
        PURPL_ARRAYSIZE(Attachments),
        &Subpass,
        1,
        SubpassDependencies,
        PURPL_ARRAYSIZE(SubpassDependencies)
        );

    VkImageView AttachmentViews[4] = {
        DeferredPass->PositionBuffer.View,
        DeferredPass->NormalBuffer.View,
        DeferredPass->AlbedoBuffer.View,
        DeferredPass->DepthBuffer.View,
    };

    VkFramebufferCreateInfo FramebufferCreateInformation = {0};
    FramebufferCreateInformation.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    FramebufferCreateInformation.renderPass = DeferredPass->RenderPass;
    FramebufferCreateInformation.pAttachments = AttachmentViews;
    FramebufferCreateInformation.attachmentCount = PURPL_ARRAYSIZE(AttachmentViews);
    FramebufferCreateInformation.width = RdrGetWidth();
    FramebufferCreateInformation.height = RdrGetHeight();
    FramebufferCreateInformation.layers = 1;

    VULKAN_CHECK(vkCreateFramebuffer(
        VlkData.Device,
        &FramebufferCreateInformation,
        VlkGetAllocationCallbacks(),
        &DeferredPass->Framebuffer
        ));
}

VOID
VlkDestroyDeferredPass(
    _Inout_ PVULKAN_DEFERRED_PASS DeferredPass
    )
{
    LogDebug("Destroying deferred pass");

    if ( DeferredPass->Framebuffer )
    {
        vkDestroyFramebuffer(
            VlkData.Device,
            DeferredPass->Framebuffer,
            VlkGetAllocationCallbacks()
            );
    }

    if ( DeferredPass->RenderPass )
    {
        vkDestroyRenderPass(
            VlkData.Device,
            DeferredPass->RenderPass,
            VlkGetAllocationCallbacks()
            );
    }
    VlkDestroyImage(&DeferredPass->PositionBuffer);
    VlkDestroyImage(&DeferredPass->NormalBuffer);
    VlkDestroyImage(&DeferredPass->AlbedoBuffer);
    VlkDestroyImage(&DeferredPass->DepthBuffer);

    memset(
        DeferredPass,
        0,
        sizeof(VULKAN_DEFERRED_PASS)
        );
}