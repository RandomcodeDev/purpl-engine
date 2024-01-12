#include "vk.h"

VOID
VlkCreateImageView(
    _Out_ VkImageView* ImageView,
    _In_ VkImage Image,
    _In_ VkFormat Format,
    _In_ VkImageAspectFlags Aspect
    )
{
    VkImageViewCreateInfo ImageViewCreateInformation = {0};
    ImageViewCreateInformation.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

    ImageViewCreateInformation.image = Image;

    ImageViewCreateInformation.viewType = VK_IMAGE_VIEW_TYPE_2D;

    ImageViewCreateInformation.format = Format;

    ImageViewCreateInformation.subresourceRange.aspectMask = Aspect;
    ImageViewCreateInformation.subresourceRange.baseMipLevel = 0;
    ImageViewCreateInformation.subresourceRange.levelCount = 1;
    ImageViewCreateInformation.subresourceRange.baseArrayLayer = 0;
    ImageViewCreateInformation.subresourceRange.layerCount = 1;
    ImageViewCreateInformation.flags = 0;

    LogTrace("Creating image view");
    VULKAN_CHECK(vkCreateImageView(
        VlkData.Device,
        &ImageViewCreateInformation,
        NULL,
        ImageView
        ));
}

VkFormat
VlkChooseFormat(
    VkFormat* Formats,
    UINT32 FormatCount,
    VkImageTiling ImageTiling,
    VkFormatFeatureFlags FormatFeatures
    )
/*++

Routine Description:

    Chooses a format from the list given.

Arguments:

    Formats - The formats to choose from.

    FormatCount - The number of formats.

    ImageTiling - The desired tiling mode.

    FormatFeatures - The desired features.

Return Value:

    A suitable format or VK_FORMAT_UNDEFINED if none could be found.

--*/
{
    UINT32 i;

    LogDebug("Choosing a format");

    for ( i = 0; i < FormatCount; i++ )
    {
        VkFormatProperties Properties;
        vkGetPhysicalDeviceFormatProperties(
            VlkData.Gpu->Device,
            Formats[i],
            &Properties
            );

        if ( (ImageTiling == VK_IMAGE_TILING_LINEAR &&
             (Properties.linearTilingFeatures & FormatFeatures) == FormatFeatures) ||
             (ImageTiling == VK_IMAGE_TILING_OPTIMAL &&
             (Properties.optimalTilingFeatures & FormatFeatures) == FormatFeatures) )
        {
            return Formats[i];
        }
    }

    CmnError("Failed to find supported format");

    return VK_FORMAT_UNDEFINED;
}