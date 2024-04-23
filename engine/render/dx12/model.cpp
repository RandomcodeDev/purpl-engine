#include "dx12.h"

EXTERN_C
VOID Dx12CreateModel(_Inout_ PMODEL Model, _In_ PMESH Mesh, _In_z_ PCSTR Name)
{
    PDIRECTX12_MODEL_DATA Data = CmnAllocType(1, DIRECTX12_MODEL_DATA);
    if (!Data)
    {
        CmnError("Failed to allocate backend data for model: %s", strerror(errno));
    }

    Model->MeshHandle = (RENDER_HANDLE)Data;

    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC ResourceDescription = CD3DX12_RESOURCE_DESC::Buffer(Mesh->VertexCount * sizeof(VERTEX));
    Dx12CreateBufferWithData(&Data->VertexBuffer, Mesh->Vertices, &HeapProperties, D3D12_HEAP_FLAG_NONE,
                             &ResourceDescription, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    Dx12NameObject(Data->VertexBuffer.Resource, "Vertex buffer %s", Name);
    ResourceDescription = CD3DX12_RESOURCE_DESC::Buffer(Mesh->IndexCount * sizeof(ivec3));
    Dx12CreateBufferWithData(&Data->IndexBuffer, Mesh->Indices, &HeapProperties, D3D12_HEAP_FLAG_NONE,
                             &ResourceDescription, D3D12_RESOURCE_STATE_INDEX_BUFFER);
    Dx12NameObject(Data->IndexBuffer.Resource, "Index buffer %s", Name);

    Dx12CreateUniformBuffer(&Data->UniformBuffer, (PVOID *)&Data->UniformBufferAddress,
                            sizeof(DIRECTX12_OBJECT_UNIFORM));
    Dx12NameObject(Data->UniformBuffer.Resource, "Uniform buffer %s", Name);
}

VOID Dx12DrawModel(_In_ PMODEL Model, _In_ PRENDER_OBJECT_UNIFORM Uniform)
{
    if (!Dx12Data.InFrame || !Model->MeshHandle)
    {
        return;
    }

    PDIRECTX12_MODEL_DATA ModelData = (PDIRECTX12_MODEL_DATA)Model->MeshHandle;

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    VertexBufferView.BufferLocation = ModelData->VertexBuffer.Resource->GetGPUVirtualAddress();
    VertexBufferView.SizeInBytes = ModelData->VertexBuffer.Size;
    VertexBufferView.StrideInBytes = sizeof(VERTEX);
    Dx12Data.CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

    D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
    IndexBufferView.BufferLocation = ModelData->IndexBuffer.Resource->GetGPUVirtualAddress();
    IndexBufferView.SizeInBytes = ModelData->IndexBuffer.Size;
    IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    Dx12Data.CommandList->IASetIndexBuffer(&IndexBufferView);

    DIRECTX12_SET_UNIFORM(ModelData->UniformBufferAddress, Uniform);

    Dx12Data.CommandList->SetGraphicsRootConstantBufferView(Dx12RootParameterObjectUniform,
                                                            ModelData->UniformBuffer.Resource->GetGPUVirtualAddress() +
                                                                Dx12Data.FrameIndex * sizeof(DIRECTX12_OBJECT_UNIFORM));

    PDIRECTX12_TEXTURE TextureData = (PDIRECTX12_TEXTURE)Model->Material->TextureHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE TextureDescriptor(
        DIRECTX12_GET_DESCRIPTOR_HANDLE_FOR_HEAP_START(Dx12Data.SrvHeap, GPU), TextureData->Index);
    Dx12Data.CommandList->SetGraphicsRootDescriptorTable(Dx12RootParameterSampler, TextureDescriptor);

    Dx12Data.CommandList->SetPipelineState((ID3D12PipelineState *)Model->Material->ShaderHandle);
    Dx12Data.CommandList->DrawIndexedInstanced(IndexBufferView.SizeInBytes / sizeof(INT32), 1, 0, 0, 0);
}

VOID Dx12DestroyModel(_Inout_ PMODEL Model)
{
    // TODO: if I'm waiting for the GPU to finish what it's doing beforehand, why the fuck are these still in use
    Dx12WaitForGpu();
    PDIRECTX12_MODEL_DATA ModelData = (PDIRECTX12_MODEL_DATA)Model->MeshHandle;
    CD3DX12_RANGE Range(0, 0);
    ModelData->UniformBuffer.Resource->Unmap(0, &Range);
    ModelData->UniformBuffer.Resource->Release();
    ModelData->VertexBuffer.Resource->Release();
    ModelData->IndexBuffer.Resource->Release();
    CmnFree(Model->MeshHandle);
}
