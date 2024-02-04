#include "dx12.h"

EXTERN_C
VOID Dx12CreateDevice(VOID)
{
    LogDebug("Getting preferred adapter");
    // TODO: add support for choosing the power saving adapter
    HRESULT_CHECK(Dx12Data.Factory->EnumAdapterByGpuPreference(
        0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
        IID_PPV_ARGS(&Dx12Data.Adapter)));
    HRESULT_CHECK(Dx12Data.Adapter->GetDesc(&Dx12Data.AdapterDescription));

    LogDebug(
        "Creating device using feature level %s on adapter %ls [%04x:%04x]",
        PURPL_STRINGIZE_EXPAND(DIRECTX12_TARGET_FEATURE_LEVEL),
        Dx12Data.AdapterDescription.Description,
        Dx12Data.AdapterDescription.VendorId,
        Dx12Data.AdapterDescription.DeviceId);

    HRESULT_CHECK(D3D12CreateDevice(Dx12Data.Adapter,
                                    DIRECTX12_TARGET_FEATURE_LEVEL,
                                    IID_PPV_ARGS(&Dx12Data.Device)));
}
