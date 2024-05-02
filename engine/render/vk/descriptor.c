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
    LogDebug("Creating descriptor set layouts");

    VkDescriptorSetLayoutBinding SceneUboBinding = {0};
    SceneUboBinding.binding = RENDER_SHADER_SCENE_UBO_REGISTER;
    SceneUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    SceneUboBinding.descriptorCount = 1;
    SceneUboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding ObjectUboBinding = {0};
    ObjectUboBinding.binding = RENDER_SHADER_OBJECT_UBO_REGISTER;
    ObjectUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ObjectUboBinding.descriptorCount = 1;
    ObjectUboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding SamplerBinding = {0};
    SamplerBinding.binding = RENDER_SHADER_SAMPLER_REGISTER;
    SamplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    SamplerBinding.descriptorCount = 1;
    SamplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding SceneBindings[] = {SceneUboBinding};
    VkDescriptorSetLayoutBinding ObjectBindings[] = {ObjectUboBinding, SamplerBinding};

    VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutInformation = {0};
    DescriptorSetLayoutInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    DescriptorSetLayoutInformation.pBindings = SceneBindings;
    DescriptorSetLayoutInformation.bindingCount = PURPL_ARRAYSIZE(SceneBindings);

    VULKAN_CHECK(vkCreateDescriptorSetLayout(VlkData.Device, &DescriptorSetLayoutInformation,
                                             VlkGetAllocationCallbacks(), &VlkData.SceneDescriptorLayout));

    DescriptorSetLayoutInformation.pBindings = ObjectBindings;
    DescriptorSetLayoutInformation.bindingCount = PURPL_ARRAYSIZE(ObjectBindings);

    VULKAN_CHECK(vkCreateDescriptorSetLayout(VlkData.Device, &DescriptorSetLayoutInformation,
                                             VlkGetAllocationCallbacks(), &VlkData.ObjectDescriptorLayout));
}

VOID VlkCreateSceneDescriptorSet(VOID)
{
    VkDescriptorSetAllocateInfo AllocateInformation = {0};
    AllocateInformation.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    AllocateInformation.descriptorPool = VlkData.DescriptorPool;
    AllocateInformation.descriptorSetCount = 1;
    AllocateInformation.pSetLayouts = &VlkData.SceneDescriptorLayout;

    VULKAN_CHECK(vkAllocateDescriptorSets(VlkData.Device, &AllocateInformation, &VlkData.SceneDescriptorSet));

    VkDescriptorBufferInfo UniformInformation = {0};
    UniformInformation.buffer = VlkData.UniformBuffer.Buffer;
    UniformInformation.offset = offsetof(VULKAN_UNIFORM_DATA, Scene);
    UniformInformation.range = sizeof(RENDER_SCENE_UNIFORM);

    VkWriteDescriptorSet Write = {0};
    Write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    Write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    Write.descriptorCount = 1;
    Write.dstSet = VlkData.SceneDescriptorSet;
    Write.dstBinding = RENDER_SHADER_SCENE_UBO_REGISTER;
    Write.pBufferInfo = &UniformInformation;

    vkUpdateDescriptorSets(VlkData.Device, 1, &Write, 0, NULL);
}
