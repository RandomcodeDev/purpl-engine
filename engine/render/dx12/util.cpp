#include "dx12.h"

EXTERN_C
VOID Dx12EnableDebugLayer(VOID)
{
    HRESULT Result;
    ID3D12Debug6 *Debug;

    LogDebug("Enabling DirectX 12 debug layer");

    Result = D3D12GetDebugInterface(IID_PPV_ARGS(&Debug));
    if (SUCCEEDED(Result))
    {
        Debug->EnableDebugLayer();
        LogDebug("Debug layer enabled");
    }
    else
    {
        _com_error Error(Result);
        LogError("Failed to enable debug layer: %s (HRESULT 0x%08X)",
                 Error.ErrorMessage(), Result);
    }
}
