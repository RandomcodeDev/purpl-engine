#include "vk.h"

VkSurfaceFormatKHR
VlkChooseSurfaceFormat(
    VOID
    )
/*++

Routine Description:

    Tries to find a specific format, forces it if only one undefined
    format, and returns the first format if those fail.

Arguments:

    None.

Return Value:

    The surface format to use.

--*/
{
    LogDebug("Choosing surface format");

    if ( stbds_arrlenu(VlkData.Gpu->SurfaceFormats) == 1 &&
         VlkData.Gpu->SurfaceFormats[0].format == VK_FORMAT_UNDEFINED )
    {
        VkSurfaceFormatKHR Format = {0};
        Format.format = VK_FORMAT_B8G8R8_UNORM;
        Format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return Format;
    }
    else
    {
        UINT32 i;

        for (i = 0; i < stbds_arrlenu(VlkData.Gpu->SurfaceFormats); i++)
        {
            VkSurfaceFormatKHR* Format = &VlkData.Gpu->SurfaceFormats[i];
            if (Format->format == VK_FORMAT_B8G8R8A8_UNORM &&
                Format->colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                return *Format;
            }
        }
    }

    return VlkData.Gpu->SurfaceFormats[0];
}

VkPresentModeKHR
VlkChoosePresentMode(
    VOID
    )
/*++

Routine Description:

    Looks for the mailbox present mode and returns FIFO if that
    can't be found.

Arguments:

    None.

Return Value:

    A suitable presentation mode.

--*/
{
    UINT32 i;

    LogDebug("Choosing presentation mode");

    for ( i = 0; i < stbds_arrlenu(VlkData.Gpu->PresentModes); i++ )
    {
        if ( VlkData.Gpu->PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR )
        {
            return VlkData.Gpu->PresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
VlkGetSurfaceExtent(
    VOID
    )
/*++

Routine Description:

    Determines the bounds of the surface for the swapchain.

Arguments:

    None.

Return Value:

    The size of the window or surface.

--*/
{
    VkExtent2D Extent = {0};

    LogDebug("Determining surface extent");

    if ( VlkData.Gpu->SurfaceCapabilities.currentExtent.width == UINT32_MAX )
    {
        VidGetSize(
            &Extent.width,
            &Extent.height
            );
    }
    else
    {
        Extent = VlkData.Gpu->SurfaceCapabilities.currentExtent;
    }

    return Extent;
}


VOID
VlkCreateSwapChain(
    VOID
    )
/*++

Routine Description:

    Creates the swap chain.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 i;

    VlkData.SurfaceFormat = VlkChooseSurfaceFormat();
    VlkData.PresentMode = VlkChoosePresentMode();
    VidGetSize(
        &VlkData.SwapChainExtent.width,
        &VlkData.SwapChainExtent.height
        );

    VkSurfaceCapabilitiesKHR SurfaceCapabilities = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        VlkData.Gpu->Device,
        VlkData.Surface,
        &SurfaceCapabilities
        );
    // if ( SwapChainExtent.width != SurfaceCapabilities.currentExtent.width ||
    //      SwapChainExtent.height != SurfaceCapabilities.currentExtent.height )
    // {
    //     SwapChainExtent.width = SurfaceCapabilities.currentExtent.width;
    //     SwapChainExtent.height = SurfaceCapabilities.currentExtent.height;
    // }

    LogDebug("Creating swap chain");

    VkSwapchainCreateInfoKHR SwapChainCreateInformation = {0};
    SwapChainCreateInformation.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapChainCreateInformation.surface = VlkData.Surface;

    SwapChainCreateInformation.minImageCount = VULKAN_FRAME_COUNT;

    SwapChainCreateInformation.imageFormat = VlkData.SurfaceFormat.format;
    SwapChainCreateInformation.imageColorSpace = VlkData.SurfaceFormat.colorSpace;
    SwapChainCreateInformation.imageExtent = VlkData.SwapChainExtent;
    SwapChainCreateInformation.imageArrayLayers = 1;

    SwapChainCreateInformation.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    if ( VlkData.Gpu->GraphicsFamilyIndex != VlkData.Gpu->PresentFamilyIndex )
    {
        UINT32 Indices[] = {
            VlkData.Gpu->GraphicsFamilyIndex,
            VlkData.Gpu->PresentFamilyIndex
        };

        SwapChainCreateInformation.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInformation.queueFamilyIndexCount = 2;
        SwapChainCreateInformation.pQueueFamilyIndices = Indices;
    }
    else
    {
        SwapChainCreateInformation.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    SwapChainCreateInformation.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    SwapChainCreateInformation.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapChainCreateInformation.presentMode = VlkData.PresentMode;

    SwapChainCreateInformation.clipped = TRUE;

    VULKAN_CHECK(vkCreateSwapchainKHR(
        VlkData.Device,
        &SwapChainCreateInformation,
        NULL,
        &VlkData.SwapChain
        ));
    VlkSetObjectName(
        VlkData.SwapChain,
        VK_OBJECT_TYPE_SWAPCHAIN_KHR,
        "Swap chain"
        );

    LogDebug("Creating swap chain image views");

    UINT32 ImageCount = 0;

    VULKAN_CHECK(vkGetSwapchainImagesKHR(
        VlkData.Device,
        VlkData.SwapChain,
        &ImageCount,
        NULL
        ));
    if ( ImageCount < 1 )
    {
        CmnError("Swap chain has no images");
    }

    stbds_arrsetlen(
        VlkData.SwapChainImages,
        ImageCount
        );
    VULKAN_CHECK(vkGetSwapchainImagesKHR(
        VlkData.Device,
        VlkData.SwapChain,
        &ImageCount,
        VlkData.SwapChainImages
        ));

    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        VlkCreateImageView(
            &VlkData.SwapChainImageViews[i],
            VlkData.SwapChainImages[i],
            VlkData.SurfaceFormat.format,
            VK_IMAGE_ASPECT_COLOR_BIT
            );
        VlkSetObjectName(
            VlkData.SwapChainImages[i],
            VK_OBJECT_TYPE_IMAGE,
            "Swap chain image %u",
            i
            );
        VlkSetObjectName(
            VlkData.SwapChainImageViews[i],
            VK_OBJECT_TYPE_IMAGE_VIEW,
            "Swap chain image view %u",
            i
            );
    }
}

VOID
VlkDestroySwapChain(
    VOID
    )
/*++

Routine Description:

    Destroys the swap chain and its image views.

Arguments:

    None.

Return Value:

    None.

--*/
{
    UINT32 i;

    LogDebug("Destroying swap chain image views");
    for ( i = 0; i < VULKAN_FRAME_COUNT; i++ )
    {
        if ( VlkData.SwapChainImageViews[i] )
        {
            vkDestroyImageView(
                VlkData.Device,
                VlkData.SwapChainImageViews[i],
                NULL
                );
            VlkData.SwapChainImageViews[i] = NULL;
        }
    }

    if ( VlkData.SwapChainImages )
    {
        LogDebug("Freeing swap chain image list");
        stbds_arrfree(VlkData.SwapChainImages);
        VlkData.SwapChainImages = NULL;
    }

    if ( VlkData.SwapChain )
    {
        LogDebug("Destroying swap chain 0x%llX", (UINT64)VlkData.SwapChain);
        vkDestroySwapchainKHR(
            VlkData.Device,
            VlkData.SwapChain,
            NULL
            );
        VlkData.SwapChain = NULL;
    }
}