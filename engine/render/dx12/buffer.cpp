#include "dx12.h"

EXTERN_C
VOID Dx12CreateHeaps(VOID)
{
    LogDebug("Creating descriptor heaps");

    D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDescription = {};
    RtvHeapDescription.NumDescriptors = DIRECTX12_FRAME_COUNT;
    RtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT_CHECK(Dx12Data.Device->CreateDescriptorHeap(&RtvHeapDescription, IID_PPV_ARGS(&Dx12Data.RtvHeap)));
    Dx12Data.RtvDescriptorSize = Dx12Data.Device->GetDescriptorHandleIncrementSize(RtvHeapDescription.Type);
    Dx12NameObject(Dx12Data.RtvHeap, "Render target view descriptor heap");

    D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDescription = {};
    DsvHeapDescription.NumDescriptors = 1;
    DsvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    DsvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT_CHECK(Dx12Data.Device->CreateDescriptorHeap(&DsvHeapDescription, IID_PPV_ARGS(&Dx12Data.DsvHeap)));
    Dx12Data.DsvDescriptorSize = Dx12Data.Device->GetDescriptorHandleIncrementSize(DsvHeapDescription.Type);
    Dx12NameObject(Dx12Data.DsvHeap, "Depth stencil view descriptor heap");

    D3D12_DESCRIPTOR_HEAP_DESC SrvHeapDescription = {};
    SrvHeapDescription.NumDescriptors =
        RENDER_MAX_TEXTURE_COUNT + (RENDER_MAX_OBJECT_COUNT + 1) * DIRECTX12_FRAME_COUNT;
    SrvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    SrvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HRESULT_CHECK(Dx12Data.Device->CreateDescriptorHeap(&SrvHeapDescription, IID_PPV_ARGS(&Dx12Data.SrvHeap)));
    Dx12Data.SrvDescriptorSize = Dx12Data.Device->GetDescriptorHandleIncrementSize(SrvHeapDescription.Type);
    Dx12NameObject(Dx12Data.SrvHeap, "Shader resource view descriptor heap");
}

EXTERN_C
VOID Dx12CreateBuffer(_Out_ PDIRECTX12_BUFFER Buffer, _In_ CONST D3D12_HEAP_PROPERTIES *HeapProperties,
                      _In_ D3D12_HEAP_FLAGS HeapFlags, _In_ CONST D3D12_RESOURCE_DESC *ResourceDescription,
                      _In_ D3D12_RESOURCE_STATES ResourceState)
{
    if (!Buffer)
    {
        return;
    }

    memset(Buffer, 0, sizeof(DIRECTX12_BUFFER));
    if (ResourceDescription->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        Buffer->Size = ResourceDescription->Width * ResourceDescription->Height *
                       Dx12GetBytesPerPixel(ResourceDescription->Format);
    }
    else
    {
        Buffer->Size = ResourceDescription->Width;
    }

    HRESULT_CHECK(Dx12Data.Device->CreateCommittedResource(HeapProperties, HeapFlags, ResourceDescription,
                                                           D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                           IID_PPV_ARGS(&Buffer->Resource)));

    if (ResourceState != D3D12_RESOURCE_STATE_COMMON)
    {
        D3D12_RESOURCE_BARRIER Barrier =
            CD3DX12_RESOURCE_BARRIER::Transition(Buffer->Resource, D3D12_RESOURCE_STATE_COMMON, ResourceState);
        Dx12Data.TransferCommandList->ResourceBarrier(1, &Barrier);
        Dx12Data.TransferCommandList->Close();
        Dx12Data.CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&Dx12Data.TransferCommandList);

        Dx12WaitForGpu();
        Dx12Data.TransferCommandAllocator->Reset();
        Dx12Data.TransferCommandList->Reset(Dx12Data.TransferCommandAllocator, nullptr);
    }
}

EXTERN_C
VOID Dx12CopyDataToCpuBuffer(_Inout_ PDIRECTX12_BUFFER Buffer, _In_ PVOID Data, _In_ UINT64 Size)
{
    if (!Buffer)
    {
        return;
    }

    PVOID BufferMapping = nullptr;
    CD3DX12_RANGE ReadRange(0, 0);
    Buffer->Resource->Map(0, &ReadRange, &BufferMapping);
    memcpy(BufferMapping, Data, PURPL_MIN(Size, Buffer->Size));
    Buffer->Resource->Unmap(0, nullptr);
}

EXTERN_C
VOID Dx12UploadDataToBuffer(_Inout_ PDIRECTX12_BUFFER Buffer, _In_ PVOID Data, _In_ UINT64 Size,
                            _In_ CONST D3D12_RESOURCE_DESC *ResourceDescription)
{
    if (!Buffer)
    {
        return;
    }

    DIRECTX12_BUFFER UploadBuffer = {};

    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC UploadResourceDescription = CD3DX12_RESOURCE_DESC::Buffer(Size);
    Dx12CreateBuffer(&UploadBuffer, &HeapProperties, D3D12_HEAP_FLAG_NONE, &UploadResourceDescription,
                     D3D12_RESOURCE_STATE_COPY_SOURCE);

    Dx12CopyDataToCpuBuffer(&UploadBuffer, Data, Size);

    if (ResourceDescription->Dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
    {
        D3D12_SUBRESOURCE_DATA SubresourceData = {};
        SubresourceData.pData = Data;
        SubresourceData.RowPitch = ResourceDescription->Width * Dx12GetBytesPerPixel(ResourceDescription->Format);
        SubresourceData.SlicePitch = ResourceDescription->Height * Dx12GetBytesPerPixel(ResourceDescription->Format);
        UpdateSubresources(Dx12Data.TransferCommandList, Buffer->Resource, UploadBuffer.Resource, 0, 0, 1,
                           &SubresourceData);
    }
    else
    {
        Dx12Data.TransferCommandList->CopyResource(Buffer->Resource, UploadBuffer.Resource);
    }
    Dx12Data.TransferCommandList->Close();
    Dx12Data.CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&Dx12Data.TransferCommandList);

    Dx12WaitForGpu();
    Dx12Data.TransferCommandAllocator->Reset();
    Dx12Data.TransferCommandList->Reset(Dx12Data.TransferCommandAllocator, nullptr);

    UploadBuffer.Resource->Release();
}

EXTERN_C
VOID Dx12CreateBufferWithData(_Out_ PDIRECTX12_BUFFER Buffer, PVOID Data,
                              _In_ CONST D3D12_HEAP_PROPERTIES *HeapProperties, _In_ D3D12_HEAP_FLAGS HeapFlags,
                              _In_ CONST D3D12_RESOURCE_DESC *ResourceDescription,
                              _In_ D3D12_RESOURCE_STATES ResourceState)
{
    Dx12CreateBuffer(Buffer, HeapProperties, HeapFlags, ResourceDescription, D3D12_RESOURCE_STATE_COPY_DEST);
    Dx12UploadDataToBuffer(Buffer, Data, Buffer->Size, ResourceDescription);
    D3D12_RESOURCE_BARRIER Barrier =
        CD3DX12_RESOURCE_BARRIER::Transition(Buffer->Resource, D3D12_RESOURCE_STATE_COPY_DEST, ResourceState);
    Dx12Data.TransferCommandList->ResourceBarrier(1, &Barrier);
    Dx12Data.TransferCommandList->Close();
    Dx12Data.CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&Dx12Data.TransferCommandList);

    Dx12WaitForGpu();
    Dx12Data.TransferCommandAllocator->Reset();
    Dx12Data.TransferCommandList->Reset(Dx12Data.TransferCommandAllocator, nullptr);
}

VOID Dx12CreateGeometryBuffers(VOID)
{
    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC ResourceDescription =
        CD3DX12_RESOURCE_DESC::Buffer(RENDER_MAX_GEOMETRY_SIZE * sizeof(MESH_VERTEX));
    CD3DX12_RANGE ReadRange(0, 0);
    Dx12CreateBuffer(&Dx12Data.GeometryVertexBuffer, &HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescription,
                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    Dx12NameObject(Dx12Data.GeometryVertexBuffer.Resource, "Geometry vertex buffer");
    Dx12Data.GeometryVertexBuffer.Resource->Map(0, &ReadRange, (PVOID *)&Dx12Data.GeometryVertexBufferAddress);
    ResourceDescription = CD3DX12_RESOURCE_DESC::Buffer(RENDER_MAX_GEOMETRY_SIZE / 3 * sizeof(ivec3));
    Dx12CreateBuffer(&Dx12Data.GeometryIndexBuffer, &HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDescription,
                     D3D12_RESOURCE_STATE_INDEX_BUFFER);
    Dx12NameObject(Dx12Data.GeometryIndexBuffer.Resource, "Geometry index buffer");
    Dx12Data.GeometryIndexBuffer.Resource->Map(0, &ReadRange, (PVOID *)&Dx12Data.GeometryIndexBufferAddress);
}
