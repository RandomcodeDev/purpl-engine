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
    LogDebug("Creating deferred render pass");
   
    memset(
        DeferredPass,
        0,
        sizeof(VULKAN_DEFERRED_PASS)
        );

    LogDebug("Creating position buffer");
    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &DeferredPass->PositionBuffer
        );
    LogDebug("Creating normal buffer");
    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_R16G16B16A16_SFLOAT,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &DeferredPass->NormalBuffer
        );
    LogDebug("Creating albedo buffer");
    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_COLOR_BIT,
        &DeferredPass->AlbedoBuffer
        );
    LogDebug("Creating depth buffer");
    VlkCreateImage(
        RdrGetWidth(),
        RdrGetHeight(),
        VK_FORMAT_D32_SFLOAT,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        VK_IMAGE_ASPECT_DEPTH_BIT,
        &DeferredPass->DepthBuffer
        );
}

VOID
VlkDestroyDeferredPass(
    _Inout_ PVULKAN_DEFERRED_PASS DeferredPass
    )
{
    LogDebug("Destroying deferred pass");
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