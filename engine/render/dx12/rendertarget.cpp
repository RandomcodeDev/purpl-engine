#include "dx12.h"

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
