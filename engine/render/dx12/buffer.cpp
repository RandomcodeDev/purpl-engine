#include "dx12.h"

VOID Dx12CreateBuffer(_Out_ PDIRECTX12_BUFFER Buffer, _In_ UINT64 Size,
                      _In_ CONST D3D12_HEAP_PROPERTIES *HeapProperties, _In_ D3D12_HEAP_FLAGS HeapFlags,
                      _In_ CONST D3D12_RESOURCE_DESC *ResourceDescription, _In_ D3D12_RESOURCE_STATES ResourceState)
{
    if (!Dx12Data.Initialized || !Buffer)
    {
        return;
    }

    memset(Buffer, 0, sizeof(DIRECTX12_BUFFER));
    Buffer->Size = Size;

    HRESULT_CHECK(Dx12Data.Device->CreateCommittedResource(HeapProperties, HeapFlags, ResourceDescription,
                                                           ResourceState, nullptr, IID_PPV_ARGS(&Buffer->Resource)));
}

VOID Dx12CopyDataToCpuBuffer(_Inout_ PDIRECTX12_BUFFER Buffer, _In_ PVOID Data, _In_ UINT64 Size)
{
    if (!Buffer)
    {
        return;
    }

    PVOID BufferMapping = nullptr;
    Buffer->Resource->Map(0, nullptr, &BufferMapping);
    memcpy(BufferMapping, Data, PURPL_MIN(Size, Buffer->Size));
    Buffer->Resource->Unmap(0, nullptr);
}

VOID Dx12UploadDataToBuffer(_Inout_ PDIRECTX12_BUFFER Buffer, _In_ PVOID Data, _In_ UINT64 Size)
{
    if (!Buffer)
    {
        return;
    }

    DIRECTX12_BUFFER UploadBuffer = {};

    D3D12_HEAP_PROPERTIES UploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    D3D12_RESOURCE_DESC UploadBufferDescription = CD3DX12_RESOURCE_DESC::Buffer(Size);
    Dx12CreateBuffer(&UploadBuffer, Size, &UploadHeapProperties, D3D12_HEAP_FLAG_NONE, &UploadBufferDescription,
                     D3D12_RESOURCE_STATE_GENERIC_READ);

    Dx12CopyDataToCpuBuffer(&UploadBuffer, Data, Size);

    D3D12_RESOURCE_BARRIER Barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        Buffer->Resource, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COPY_DEST);
    Dx12Data.TransferCommandList->ResourceBarrier(1, &Barrier);
    Dx12Data.TransferCommandList->CopyResource(Buffer->Resource, UploadBuffer.Resource);
    Dx12Data.TransferCommandList->Close();
    Dx12Data.CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&Dx12Data.TransferCommandList);
}

VOID Dx12CreateBufferWithData(_Out_ PDIRECTX12_BUFFER Buffer, PVOID Data, _In_ SIZE_T Size,
                              _In_ CONST D3D12_HEAP_PROPERTIES *HeapProperties, _In_ D3D12_HEAP_FLAGS HeapFlags,
                              _In_ CONST D3D12_RESOURCE_DESC *ResourceDescription,
                              _In_ D3D12_RESOURCE_STATES ResourceState)
{
    Dx12CreateBuffer(Buffer, Size, HeapProperties, HeapFlags, ResourceDescription, D3D12_RESOURCE_STATE_COPY_DEST);
    Dx12UploadDataToBuffer(Buffer, Data, Size);
    D3D12_RESOURCE_BARRIER Barrier =
        CD3DX12_RESOURCE_BARRIER::Transition(Buffer->Resource, D3D12_RESOURCE_STATE_COPY_DEST, ResourceState);
    Dx12Data.TransferCommandList->ResourceBarrier(1, &Barrier);
    Dx12Data.CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList **)&Dx12Data.TransferCommandList);
}
