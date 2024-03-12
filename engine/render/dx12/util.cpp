#include "dx12.h"

EXTERN_C
VOID Dx12EnableDebugLayer(VOID)
{
    ID3D12Debug6 *Debug;
    IDXGIDebug1 *DxgiDebug;

    LogDebug("Enabling DXGI debug features");

    HRESULT Result = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&DxgiDebug));
    if (SUCCEEDED(Result))
    {
        DxgiDebug->EnableLeakTrackingForThread();
    }
    else
    {
        _com_error Error(Result);
        LogError("Failed to enable debug features: %s (HRESULT 0x%08X)", Error.ErrorMessage(), Result);
    }

    LogDebug("Enabling DirectX 12 debug features");

    Result = D3D12GetDebugInterface(IID_PPV_ARGS(&Debug));
    if (SUCCEEDED(Result))
    {
        Debug->EnableDebugLayer();
        Debug->SetEnableAutoName(TRUE);
        LogDebug("Debug layer enabled");
    }
    else
    {
        _com_error Error(Result);
        LogError("Failed to enable debug features: %s (HRESULT 0x%08X)", Error.ErrorMessage(), Result);
    }
}
