#include "dx12.h"

EXTERN_C
VOID Dx12CreateRtvHeap(VOID)
{
    LogDebug("Creating render target view descriptor heap");

    D3D12_DESCRIPTOR_HEAP_DESC RtvHeapDescription = {};
    RtvHeapDescription.NumDescriptors = DIRECTX12_FRAME_COUNT;
    RtvHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RtvHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT_CHECK(Dx12Data.Device->CreateDescriptorHeap(
        &RtvHeapDescription, IID_PPV_ARGS(&Dx12Data.RtvHeap)));
    Dx12Data.RtvDescriptorSize = Dx12Data.Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

EXTERN_C
VOID Dx12CreateRenderTargetViews(VOID)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(Dx12Data.RtvHeap->GetCPUDescriptorHandleForHeapStart());
    UINT32 i;

    for (i = 0; i < PURPL_ARRAYSIZE(Dx12Data.RenderTargets); i++)
    {
        HRESULT_CHECK(Dx12Data.SwapChain->GetBuffer(
            0, IID_PPV_ARGS(&Dx12Data.RenderTargets[i])));
        Dx12Data.Device->CreateRenderTargetView(Dx12Data.RenderTargets[i], nullptr, RtvHandle);
        RtvHandle.Offset(1, Dx12Data.RtvDescriptorSize);
    }
}
