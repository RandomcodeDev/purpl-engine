#include "vk.h"

VkSurfaceFormatKHR VlkChooseSurfaceFormat(VOID)
{
    LogDebug("Choosing surface format");

    if (stbds_arrlenu(VlkData.Gpu->SurfaceFormats) == 1 && VlkData.Gpu->SurfaceFormats[0].format == VK_FORMAT_UNDEFINED)
    {
        VkSurfaceFormatKHR Format = {0};
        Format.format = VK_FORMAT_B8G8R8A8_SRGB;
        Format.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return Format;
    }
    else
    {
        UINT32 i;

        for (i = 0; i < stbds_arrlenu(VlkData.Gpu->SurfaceFormats); i++)
        {
            VkSurfaceFormatKHR *Format = &VlkData.Gpu->SurfaceFormats[i];
            if (Format->format == VK_FORMAT_B8G8R8A8_SRGB && Format->colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                return *Format;
            }
        }
    }

    return VlkData.Gpu->SurfaceFormats[0];
}

VkPresentModeKHR VlkChoosePresentMode(VOID)
{
    UINT32 i;

    LogDebug("Choosing presentation mode");

    for (i = 0; i < stbds_arrlenu(VlkData.Gpu->PresentModes); i++)
    {
        if (VlkData.Gpu->PresentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return VlkData.Gpu->PresentModes[i];
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VlkGetSurfaceExtent(VOID)
{
    VkExtent2D Extent = {0};

    LogDebug("Determining surface extent");

    if (VlkData.Gpu->SurfaceCapabilities.currentExtent.width == UINT32_MAX)
    {
        VidGetSize(&Extent.width, &Extent.height);
    }
    else
    {
        Extent = VlkData.Gpu->SurfaceCapabilities.currentExtent;
    }

    return Extent;
}

VOID VlkCreateSwapChain(VOID)
{
    UINT32 i;

    VlkData.SurfaceFormat = VlkChooseSurfaceFormat();
    VlkData.PresentMode = VlkChoosePresentMode();
    VidGetSize(&VlkData.SwapChainExtent.width, &VlkData.SwapChainExtent.height);

    VkSurfaceCapabilitiesKHR SurfaceCapabilities = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VlkData.Gpu->Device, VlkData.Surface, &SurfaceCapabilities);
    if (VlkData.SwapChainExtent.width > SurfaceCapabilities.maxImageExtent.width ||
        VlkData.SwapChainExtent.height > SurfaceCapabilities.maxImageExtent.height)
    {
        VlkData.SwapChainExtent.width = SurfaceCapabilities.maxImageExtent.width;
        VlkData.SwapChainExtent.height = SurfaceCapabilities.maxImageExtent.height;
    }

    VkSwapchainCreateInfoKHR SwapChainCreateInformation = {0};
    SwapChainCreateInformation.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapChainCreateInformation.surface = VlkData.Surface;

    SwapChainCreateInformation.minImageCount = SurfaceCapabilities.minImageCount + 1;
    PURPL_ASSERT(SurfaceCapabilities.maxImageCount == 0 ||
                 SwapChainCreateInformation.minImageCount <= SurfaceCapabilities.maxImageCount);

    LogDebug("Creating %ux%u swap chain with minimum of %u images", VlkData.SwapChainExtent.width,
             VlkData.SwapChainExtent.height, SwapChainCreateInformation.minImageCount);

    SwapChainCreateInformation.imageFormat = VlkData.SurfaceFormat.format;
    SwapChainCreateInformation.imageColorSpace = VlkData.SurfaceFormat.colorSpace;
    SwapChainCreateInformation.imageExtent = VlkData.SwapChainExtent;
    SwapChainCreateInformation.imageArrayLayers = 1;

    SwapChainCreateInformation.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (VlkData.Gpu->GraphicsFamilyIndex != VlkData.Gpu->PresentFamilyIndex)
    {
        UINT32 Indices[] = {VlkData.Gpu->GraphicsFamilyIndex, VlkData.Gpu->PresentFamilyIndex};

        SwapChainCreateInformation.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        SwapChainCreateInformation.queueFamilyIndexCount = 2;
        SwapChainCreateInformation.pQueueFamilyIndices = Indices;
    }
    else
    {
        SwapChainCreateInformation.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    SwapChainCreateInformation.preTransform = SurfaceCapabilities.currentTransform;
    SwapChainCreateInformation.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapChainCreateInformation.presentMode = VlkData.PresentMode;

    SwapChainCreateInformation.clipped = TRUE;

    VULKAN_CHECK(vkCreateSwapchainKHR(VlkData.Device, &SwapChainCreateInformation, VlkGetAllocationCallbacks(),
                                      &VlkData.SwapChain));
    VlkSetObjectName((UINT64)VlkData.SwapChain, VK_OBJECT_TYPE_SWAPCHAIN_KHR, "Swap chain");

    LogDebug("Creating swap chain image views");

    UINT32 ImageCount = 0;

    VULKAN_CHECK(vkGetSwapchainImagesKHR(VlkData.Device, VlkData.SwapChain, &ImageCount, NULL));
    if (ImageCount < 1)
    {
        CmnError("Swap chain has no images");
    }

    stbds_arrsetlen(VlkData.SwapChainImages, ImageCount);
    VULKAN_CHECK(vkGetSwapchainImagesKHR(VlkData.Device, VlkData.SwapChain, &ImageCount, VlkData.SwapChainImages));

    stbds_arrsetlen(VlkData.SwapChainImageViews, stbds_arrlenu(VlkData.SwapChainImages));
    for (i = 0; i < stbds_arrlenu(VlkData.SwapChainImageViews); i++)
    {
        VlkCreateImageView(&VlkData.SwapChainImageViews[i], VlkData.SwapChainImages[i], VlkData.SurfaceFormat.format,
                           VK_IMAGE_ASPECT_COLOR_BIT);
        VlkSetObjectName((UINT64)VlkData.SwapChainImages[i], VK_OBJECT_TYPE_IMAGE, "Swap chain image %u", i);
        VlkSetObjectName((UINT64)VlkData.SwapChainImageViews[i], VK_OBJECT_TYPE_IMAGE_VIEW, "Swap chain image view %u", i);
    }
}

VOID VlkDestroySwapChain(VOID)
{
    UINT32 i;

    if (VlkData.SwapChainImageViews)
    {
        for (i = 0; i < stbds_arrlenu(VlkData.SwapChainImageViews); i++)
        {
            if (VlkData.SwapChainImageViews[i])
            {
                LogDebug("Destroying swap chain image view %u/%u", i + 1, stbds_arrlenu(VlkData.SwapChainImageViews));
                vkDestroyImageView(VlkData.Device, VlkData.SwapChainImageViews[i], VlkGetAllocationCallbacks());
                VlkData.SwapChainImageViews[i] = VK_NULL_HANDLE;
            }
        }
        stbds_arrfree(VlkData.SwapChainImageViews);
        VlkData.SwapChainImageViews = NULL;
    }

    if (VlkData.SwapChainImages)
    {
        LogDebug("Freeing swap chain image list");
        stbds_arrfree(VlkData.SwapChainImages);
        VlkData.SwapChainImages = NULL;
    }

    if (VlkData.SwapChain)
    {
        LogDebug("Destroying swap chain 0x%llX", (UINT64)VlkData.SwapChain);
        vkDestroySwapchainKHR(VlkData.Device, VlkData.SwapChain, VlkGetAllocationCallbacks());
        VlkData.SwapChain = VK_NULL_HANDLE;
    }
}
