#include "vk.h"

VOID VlkCreateDescriptorPool(VOID)
{
    LogDebug("Creating descriptor pool");

    static CONST VkDescriptorPoolSize PoolSizes[] = {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VULKAN_MAX_DESCRIPTOR_SETS},
        {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VULKAN_MAX_DESCRIPTOR_SETS}};

    VkDescriptorPoolCreateInfo CreateInformation = {0};
    CreateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    CreateInformation.flags =
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    CreateInformation.pPoolSizes = PoolSizes;
    CreateInformation.poolSizeCount = PURPL_ARRAYSIZE(PoolSizes);
    CreateInformation.maxSets = VULKAN_MAX_DESCRIPTOR_SETS * CreateInformation.poolSizeCount;
    VULKAN_CHECK(vkCreateDescriptorPool(VlkData.Device, &CreateInformation, VlkGetAllocationCallbacks(),
                                        &VlkData.DescriptorPool));
    VlkSetObjectName((UINT64)VlkData.DescriptorPool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "Descriptor pool");
}

VOID VlkCreateDescriptorSetLayout(VOID)
{
    LogDebug("Creating descriptor set layout");

    VkDescriptorSetLayoutBinding SceneUboBinding = {0};
    SceneUboBinding.binding = 0;
    SceneUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    SceneUboBinding.descriptorCount = 1;
    SceneUboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding ObjectUboBinding = {0};
    ObjectUboBinding.binding = 1;
    ObjectUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ObjectUboBinding.descriptorCount = 1;
    ObjectUboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding SamplerBinding = {0};
    SamplerBinding.binding = 2;
    SamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    SamplerBinding.descriptorCount = 1;
    SamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding Bindings[] = {SceneUboBinding, ObjectUboBinding, SamplerBinding};

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutInformation = {0};
    DescriptorSetLayoutInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DescriptorSetLayoutInformation.pBindings = Bindings;
    DescriptorSetLayoutInformation.bindingCount = PURPL_ARRAYSIZE(Bindings);

    VULKAN_CHECK(vkCreateDescriptorSetLayout(VlkData.Device, &DescriptorSetLayoutInformation,
                                             VlkGetAllocationCallbacks(), &VlkData.DescriptorSetLayout));
}
