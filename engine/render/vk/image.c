#include "vk.h"

VOID VlkCreateImageView(_Out_ VkImageView *ImageView, _In_ VkImage Image,
                        _In_ VkFormat Format, _In_ VkImageAspectFlags Aspect)
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
    VULKAN_CHECK(vkCreateImageView(VlkData.Device, &ImageViewCreateInformation,
                                   VlkGetAllocationCallbacks(), ImageView));
}

VkFormat VlkChooseFormat(VkFormat *Formats, UINT32 FormatCount,
                         VkImageTiling ImageTiling,
                         VkFormatFeatureFlags FormatFeatures)
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

    for (i = 0; i < FormatCount; i++)
    {
        VkFormatProperties Properties;
        vkGetPhysicalDeviceFormatProperties(VlkData.Gpu->Device, Formats[i],
                                            &Properties);

        if ((ImageTiling == VK_IMAGE_TILING_LINEAR &&
             (Properties.linearTilingFeatures & FormatFeatures) ==
                 FormatFeatures) ||
            (ImageTiling == VK_IMAGE_TILING_OPTIMAL &&
             (Properties.optimalTilingFeatures & FormatFeatures) ==
                 FormatFeatures))
        {
            return Formats[i];
        }
    }

    CmnError("Failed to find supported format");

    return VK_FORMAT_UNDEFINED;
}

VOID VlkTransitionImageLayout(_Inout_ VkImage Image,
                              _In_ VkImageLayout OldLayout,
                              _In_ VkImageLayout NewLayout)
{
    VkCommandBuffer TransferBuffer;
    VkPipelineStageFlags SourceStage;
    VkPipelineStageFlags DestinationStage;

    // LogTrace("Transitioning image 0x%llX from %u to %u", (UINT64)Image,
    // OldLayout, NewLayout);

    VkImageMemoryBarrier Barrier = {0};
    Barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    Barrier.oldLayout = OldLayout;
    Barrier.newLayout = NewLayout;

    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    Barrier.image = Image;

    Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Barrier.subresourceRange.baseMipLevel = 0;
    Barrier.subresourceRange.levelCount = 1;
    Barrier.subresourceRange.baseArrayLayer = 0;
    Barrier.subresourceRange.layerCount = 1;

    if (OldLayout == NewLayout)
    {
        return;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
             NewLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        SourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        DestinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (OldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             NewLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        Barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        Barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        SourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        DestinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        LogWarning("Unknown layout transition");

        Barrier.srcAccessMask = 0;
        Barrier.dstAccessMask = 0;

        SourceStage = 0;
        DestinationStage = 0;
    }

    TransferBuffer = VlkBeginTransfer();

    vkCmdPipelineBarrier(TransferBuffer, SourceStage, DestinationStage, 0, 0,
                         NULL, 0, NULL, 1, &Barrier);

    VlkEndTransfer(TransferBuffer);
}

VOID VlkCopyBufferToImage(_In_ VkBuffer Buffer, _Out_ VkImage Image,
                          _In_ UINT32 Width, _In_ UINT32 Height)
{
    VkCommandBuffer TransferBuffer;

    // LogTrace("Copying buffer 0x%llX to image 0x%llX", (UINT64)Buffer,
    // (UINT64)Image);

    VkBufferImageCopy Region = {0};
    Region.bufferOffset = 0;
    Region.bufferRowLength = 0;
    Region.bufferImageHeight = 0;

    Region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    Region.imageSubresource.mipLevel = 0;
    Region.imageSubresource.baseArrayLayer = 0;
    Region.imageSubresource.layerCount = 1;

    Region.imageOffset.x = 0;
    Region.imageOffset.y = 0;
    Region.imageOffset.z = 0;

    Region.imageExtent.width = Width;
    Region.imageExtent.height = Height;
    Region.imageExtent.depth = 1;

    TransferBuffer = VlkBeginTransfer();

    vkCmdCopyBufferToImage(TransferBuffer, Buffer, Image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &Region);

    VlkEndTransfer(TransferBuffer);
}

VOID VlkCreateImage(_In_ UINT32 Width, _In_ UINT32 Height, _In_ VkFormat Format,
                    _In_ VkImageLayout Layout, _In_ VkImageUsageFlags Usage,
                    _In_ VmaMemoryUsage MemoryUsage,
                    _In_ VkImageAspectFlags Aspect, _Out_ PVULKAN_IMAGE Image)
{
    memset(Image, 0, sizeof(VULKAN_IMAGE));

    Image->Format = Format;

    VkImageCreateInfo ImageCreateInformation = {0};
    ImageCreateInformation.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ImageCreateInformation.imageType = VK_IMAGE_TYPE_2D;
    ImageCreateInformation.extent.width = Width;
    ImageCreateInformation.extent.height = Height;
    ImageCreateInformation.extent.depth = 1;
    ImageCreateInformation.mipLevels = 1;
    ImageCreateInformation.arrayLayers = 1;
    ImageCreateInformation.tiling = VK_IMAGE_TILING_OPTIMAL;
    ImageCreateInformation.format = Format;
    ImageCreateInformation.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    ImageCreateInformation.usage = Usage;
    ImageCreateInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ImageCreateInformation.samples = VK_SAMPLE_COUNT_1_BIT;

    VmaAllocationCreateInfo AllocationCreateInformation = {0};
    AllocationCreateInformation.usage = MemoryUsage;

    VULKAN_CHECK(vmaCreateImage(VlkData.Allocator, &ImageCreateInformation,
                                &AllocationCreateInformation, &Image->Handle,
                                &Image->Allocation, NULL));
    VlkTransitionImageLayout(Image->Handle, VK_IMAGE_LAYOUT_UNDEFINED, Layout);
    VlkCreateImageView(&Image->View, Image->Handle, Format, Aspect);
}

VOID VlkCreateImageWithData(_In_ PVOID Data, _In_ VkDeviceSize Size,
                            _In_ UINT32 Width, _In_ UINT32 Height,
                            _In_ VkFormat Format, _In_ VkImageLayout Layout,
                            _In_ VkImageUsageFlags Usage,
                            _In_ VmaMemoryUsage MemoryUsage,
                            _In_ VkImageAspectFlags Aspect,
                            _Out_ PVULKAN_IMAGE Image)
{
    VULKAN_BUFFER StagingBuffer;
    PVOID ImageBuffer;

    VlkAllocateBuffer(Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                          VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                      &StagingBuffer);

    ImageBuffer = NULL;
    vmaMapMemory(VlkData.Allocator, StagingBuffer.Allocation, &ImageBuffer);
    memcpy(ImageBuffer, Data, Size);
    vmaUnmapMemory(VlkData.Allocator, StagingBuffer.Allocation);

    VlkCreateImage(Width, Height, Format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                   Usage, MemoryUsage, Aspect, Image);

    VlkCopyBufferToImage(StagingBuffer.Buffer, Image->Handle, Width, Height);
    VlkTransitionImageLayout(Image->Handle,
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Layout);

    VlkFreeBuffer(&StagingBuffer);
}

VOID VlkDestroyImage(_Inout_ PVULKAN_IMAGE Image)
{
    vkDestroyImageView(VlkData.Device, Image->View,
                       VlkGetAllocationCallbacks());
    vmaDestroyImage(VlkData.Allocator, Image->Handle, Image->Allocation);
}