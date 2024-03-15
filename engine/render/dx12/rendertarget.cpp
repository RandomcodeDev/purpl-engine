#include "dx12.h"

EXTERN_C
VOID Dx12CreateRenderTargetViews(VOID)
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(
#ifdef __GNUC__
        *Dx12Data.RtvHeap->GetCPUDescriptorHandleForHeapStart(NULL)
#else
        Dx12Data.RtvHeap->GetCPUDescriptorHandleForHeapStart()
#endif
    );
    UINT32 i;

    for (i = 0; i < PURPL_ARRAYSIZE(Dx12Data.RenderTargets); i++)
    {
        HRESULT_CHECK(Dx12Data.SwapChain->GetBuffer(i, IID_PPV_ARGS(&Dx12Data.RenderTargets[i])));
        Dx12Data.Device->CreateRenderTargetView(Dx12Data.RenderTargets[i], nullptr, RtvHandle);
        RtvHandle.Offset(1, Dx12Data.RtvDescriptorSize);
        Dx12NameObject(Dx12Data.RenderTargets[i], "Render target %u", i);
    }
}
