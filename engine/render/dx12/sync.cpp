#include "dx12.h"

EXTERN_C
VOID Dx12CreateMainFence(VOID)
{
    LogDebug("Creating main fence");

    HRESULT_CHECK(Dx12Data.Device->CreateFence(Dx12Data.FenceValues[Dx12Data.FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Dx12Data.Fence)));
    Dx12Data.FenceValues[Dx12Data.FrameIndex] = 1;

    Dx12Data.FenceEvent = CreateEventA(nullptr, FALSE, FALSE, "Fence event");
    if (!Dx12Data.FenceEvent)
    {
        HRESULT_CHECK(HRESULT_FROM_WIN32(GetLastError()));
    }
}
