#include "vk.h"

VOID VlkInitializeObject(_In_z_ PCSTR Name, _Inout_ PRENDER_OBJECT_DATA Data, _In_ PMODEL Model)
{
    PVULKAN_OBJECT_DATA ObjectData = CmnAllocType(1, VULKAN_OBJECT_DATA);
    if (!ObjectData)
    {
        CmnError("Failed to allocate per-object data for %s: %s", Name, strerror(errno));
    }

    VkDescriptorSetAllocateInfo AllocateInformation = {0};
    AllocateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocateInformation.descriptorPool = VlkData.DescriptorPool;
    AllocateInformation.descriptorSetCount = 1;
    AllocateInformation.pSetLayouts = &VlkData.ObjectDescriptorLayout;
    VULKAN_CHECK(vkAllocateDescriptorSets(VlkData.Device, &AllocateInformation, &ObjectData->DescriptorSet));

    VkDescriptorBufferInfo UniformInformation = {0};
    UniformInformation.buffer = VlkData.UniformBuffer.Buffer;
    UniformInformation.offset = offsetof(VULKAN_UNIFORM_DATA, Object);
    UniformInformation.range = sizeof(RENDER_OBJECT_UNIFORM);

    PVULKAN_IMAGE Texture = (PVULKAN_IMAGE)Model->Material->TextureHandle;
    VkDescriptorImageInfo TextureInformation = {0};
    TextureInformation.sampler = VlkData.Sampler;
    TextureInformation.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    TextureInformation.imageView = Texture->View;

    VkWriteDescriptorSet Writes[2] = {0};
    Writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Writes[0].descriptorCount = 1;
    Writes[0].dstSet = ObjectData->DescriptorSet;
    Writes[0].dstBinding = RENDER_SHADER_OBJECT_UBO_REGISTER;
    Writes[0].pBufferInfo = &UniformInformation;

    Writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    Writes[1].descriptorCount = 1;
    Writes[1].dstSet = ObjectData->DescriptorSet;
    Writes[1].dstBinding = RENDER_SHADER_SAMPLER_REGISTER;
    Writes[1].pImageInfo = &TextureInformation;

    vkUpdateDescriptorSets(VlkData.Device, PURPL_ARRAYSIZE(Writes), Writes, 0, NULL);

    Data->Handle = (RENDER_HANDLE)ObjectData;
}

VOID VlkDestroyObject(_Inout_ PRENDER_OBJECT_DATA Data)
{
    PVULKAN_OBJECT_DATA ObjectData = (PVULKAN_OBJECT_DATA)Data->Handle;
    vkFreeDescriptorSets(VlkData.Device, VlkData.DescriptorPool, 1, &ObjectData->DescriptorSet);
    CmnFree(Data->Handle);
}
