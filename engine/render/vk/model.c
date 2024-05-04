#include "vk.h"

VOID VlkCreateModel(_In_z_ PCSTR Name, _Inout_ PMODEL Model, _In_ PMESH Mesh)
{
    PVULKAN_MODEL_DATA ModelData = CmnAllocType(1, VULKAN_MODEL_DATA);
    if (!ModelData)
    {
        CmnError("Failed to allocate model data for %s: %s", Name, strerror(errno));
    }

    VlkAllocateBufferWithData(Mesh->Vertices, Mesh->VertexCount * sizeof(VERTEX), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ModelData->VertexBuffer);
    VlkNameBuffer(&ModelData->VertexBuffer, "Vertex buffer for %s", Name);
    VlkAllocateBufferWithData(Mesh->Indices, Mesh->IndexCount * sizeof(ivec3), VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &ModelData->IndexBuffer);
    VlkNameBuffer(&ModelData->IndexBuffer, "Index buffer for %s", Name);

    Model->MeshHandle = (RENDER_HANDLE)ModelData;
}

VOID VlkDrawModel(_In_ PMODEL Model, _In_ PRENDER_OBJECT_UNIFORM Uniform, _In_ PRENDER_OBJECT_DATA Data)
{
    PVULKAN_MODEL_DATA ModelData = (PVULKAN_MODEL_DATA)Model->MeshHandle;
    PVULKAN_OBJECT_DATA ObjectData = (PVULKAN_OBJECT_DATA)Data->Handle;
    VkCommandBuffer CommandBuffer = VlkData.CommandBuffers[VlkData.FrameIndex];

    VkDescriptorSet DescriptorSets[] = {VlkData.SceneDescriptorSet, ObjectData->DescriptorSet};

    VULKAN_SET_UNIFORM(ObjectData->UniformBufferAddress, Uniform);
    VkDeviceSize Offset = 0;
    vkCmdBindVertexBuffers(CommandBuffer, 0, 1, &ModelData->VertexBuffer.Buffer, &Offset);
    vkCmdBindIndexBuffer(CommandBuffer, ModelData->IndexBuffer.Buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VlkData.PipelineLayout, 0,
                            PURPL_ARRAYSIZE(DescriptorSets), DescriptorSets, 0, NULL);
    vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (VkPipeline)Model->Material->ShaderHandle);
    vkCmdDrawIndexed(CommandBuffer, ModelData->IndexBuffer.Size / sizeof(ivec3) * 3, 1, 0, 0, 0);
}

VOID VlkDestroyModel(_Inout_ PMODEL Model)
{
    PVULKAN_MODEL_DATA ModelData = (PVULKAN_MODEL_DATA)Model->MeshHandle;
    VlkFreeBuffer(&ModelData->IndexBuffer);
    VlkFreeBuffer(&ModelData->VertexBuffer);
    CmnFree(Model->MeshHandle);
}
