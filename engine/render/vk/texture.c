#include "vk.h"

RENDER_HANDLE VlkUseTexture(_In_ PTEXTURE Texture, _In_z_ PCSTR Name)
{
    VkFormat Format;
    switch (Texture->Format)
    {
    default:
    case TextureFormatRgba8:
        Format = VK_FORMAT_R8G8B8A8_SRGB;
        break;
    case TextureFormatDepth:
        Format = VK_FORMAT_D32_SFLOAT;
        break;
    }

    PVULKAN_IMAGE Image = CmnAllocType(1, VULKAN_IMAGE);
    if (!Image)
    {
        CmnError("Failed to allocate image data for %s: %s", Name, strerror(errno));
    }

    VlkCreateImageWithData(
        Texture->Pixels, GetTextureSize(*Texture), Texture->Width, Texture->Height, Format,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        Texture->Format == TextureFormatDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT, Image);

    return (RENDER_HANDLE)Image;
}

VOID VlkDestroyTexture(_In_ RENDER_HANDLE Handle)
{
    PVULKAN_IMAGE Image = (PVULKAN_IMAGE)Handle;
    VlkDestroyImage(Image);
    CmnFree(Image);
}
