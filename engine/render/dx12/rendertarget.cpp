#include "dx12.h"

EXTERN_C
VOID Dx12CreateDepthTarget(VOID)
{
    LogDebug("Creating depth stencil target");

    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);
    CD3DX12_RESOURCE_DESC ImageDescription = CD3DX12_RESOURCE_DESC::Tex2D(
        DXGI_FORMAT_D32_FLOAT, RdrGetWidth(), RdrGetHeight(), 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
    D3D12_CLEAR_VALUE ClearValue = {};
    ClearValue.Format = DXGI_FORMAT_D32_FLOAT;
    ClearValue.DepthStencil.Depth = 1.0f;
    ClearValue.DepthStencil.Stencil = 0;

    HRESULT_CHECK(Dx12Data.Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &ImageDescription,
                                                           D3D12_RESOURCE_STATE_DEPTH_WRITE, &ClearValue,
                                                           IID_PPV_ARGS(&Dx12Data.DepthStencil)));

    D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilDescription = {};
    DepthStencilDescription.Format = DXGI_FORMAT_D32_FLOAT;
    DepthStencilDescription.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    DepthStencilDescription.Flags = D3D12_DSV_FLAG_NONE;

    Dx12NameObject(Dx12Data.DepthStencil, "Depth stencil target");
    Dx12Data.Device->CreateDepthStencilView(Dx12Data.DepthStencil, &DepthStencilDescription,
                                            Dx12Data.DsvHeap->GetCPUDescriptorHandleForHeapStart());
}

EXTERN_C
VOID Dx12CreateRenderTargetViews(VOID)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(DIRECTX12_GET_DESCRIPTOR_HANDLE_FOR_HEAP_START(Dx12Data.RtvHeap, CPU));
    UINT32 i;

    LogDebug("Creating %u render target views", DIRECTX12_FRAME_COUNT);

    D3D12_RENDER_TARGET_VIEW_DESC RtvDescription = {};
    RtvDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    RtvDescription.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

    for (i = 0; i < PURPL_ARRAYSIZE(Dx12Data.RenderTargets); i++)
    {
        HRESULT_CHECK(Dx12Data.SwapChain->GetBuffer(i, IID_PPV_ARGS(&Dx12Data.RenderTargets[i])));
        Dx12Data.Device->CreateRenderTargetView(Dx12Data.RenderTargets[i], &RtvDescription, RtvHandle);
        RtvHandle.Offset(1, Dx12Data.RtvDescriptorSize);
        Dx12NameObject(Dx12Data.RenderTargets[i], "Render target %u", i);
    }
}
