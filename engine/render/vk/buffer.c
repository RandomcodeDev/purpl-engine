#include "vk.h"

VOID VlkAllocateBuffer(_In_ VkDeviceSize Size, _In_ VkBufferUsageFlags Usage, _In_ VkMemoryPropertyFlags Flags,
                       _Out_ PVULKAN_BUFFER Buffer)
{
    memset(Buffer, 0, sizeof(VULKAN_BUFFER));
    Buffer->Size = Size;

    VkBufferCreateInfo BufferCreateInformation = {0};
    BufferCreateInformation.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferCreateInformation.size = Buffer->Size;
    BufferCreateInformation.usage = Usage;
    BufferCreateInformation.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo AllocationCreateInformation = {0};
    AllocationCreateInformation.requiredFlags = Flags;

    // LogTrace("Allocating %zu-byte buffer 0x%llX", Size, (UINT64)Buffer);
    VULKAN_CHECK(vmaCreateBuffer(VlkData.Allocator, &BufferCreateInformation, &AllocationCreateInformation,
                                 &Buffer->Buffer, &Buffer->Allocation, NULL));
}

VOID VlkAllocateBufferWithData(_In_ PVOID Data, _In_ VkDeviceSize Size, _In_ VkBufferUsageFlags Usage,
                               _In_ VkMemoryPropertyFlags Flags, _Out_ PVULKAN_BUFFER Buffer)
{
    VULKAN_BUFFER StagingBuffer;
    PVOID BufferAddress;

    VlkAllocateBuffer(Size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &StagingBuffer);

    BufferAddress = NULL;
    vmaMapMemory(VlkData.Allocator, StagingBuffer.Allocation, &BufferAddress);
    memcpy(BufferAddress, Data, Size);
    vmaUnmapMemory(VlkData.Allocator, StagingBuffer.Allocation);

    VlkAllocateBuffer(Size, Usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Flags, Buffer);
    VlkCopyBuffer(&StagingBuffer, Buffer, Size);
    VlkFreeBuffer(&StagingBuffer);
}

VOID VlkNameBuffer(_Inout_ PVULKAN_BUFFER Buffer, _In_z_ PCSTR Name, ...)
{
    va_list Arguments;

    va_start(Arguments, Name);
    PSTR FormattedName = CmnFormatStringVarArgs(Name, Arguments);
    va_end(Arguments);

    VlkSetObjectName(Buffer->Buffer, 0x69420BFF, FormattedName);
    vmaSetAllocationName(VlkData.Allocator, Buffer->Allocation, FormattedName);

    CmnFree(FormattedName);
}

VOID VlkFreeBuffer(_Inout_ PVULKAN_BUFFER Buffer)
{
    // LogTrace("Freeing %zu-byte buffer 0x%llX", Buffer->Size, (UINT64)Buffer);
    if (Buffer->Allocation)
    {
        vmaDestroyBuffer(VlkData.Allocator, Buffer->Buffer, Buffer->Allocation);
    }
    memset(Buffer, 0, sizeof(VULKAN_BUFFER));
}

VOID VlkCopyBuffer(_In_ PVULKAN_BUFFER Source, _In_ PVULKAN_BUFFER Destination, _In_ VkDeviceSize Size)
{
    VkCommandBuffer TransferBuffer;

    // LogTrace("Copying Vulkan buffer 0x%llX to 0x%llX", (UINT64)Source,
    // (UINT64)Destination);

    TransferBuffer = VlkBeginTransfer();

    VkBufferCopy CopyRegion = {0};
    CopyRegion.size = Size;
    vkCmdCopyBuffer(TransferBuffer, Source->Buffer, Destination->Buffer, 1, &CopyRegion);

    VlkEndTransfer(TransferBuffer);
}
