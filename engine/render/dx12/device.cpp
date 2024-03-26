#include "dx12.h"

EXTERN_C
VOID Dx12CreateDevice(VOID)
{
    LogDebug("Getting preferred adapter");
    // TODO: add support for choosing the power saving adapter
    HRESULT_CHECK(Dx12Data.Factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                                                               IID_PPV_ARGS(&Dx12Data.Adapter)));
    HRESULT_CHECK(Dx12Data.Adapter->GetDesc(&Dx12Data.AdapterDescription));

    Dx12Data.AdapterName = CmnFormatString("%ls [%04x:%04x]", Dx12Data.AdapterDescription.Description,
                                           Dx12Data.AdapterDescription.VendorId, Dx12Data.AdapterDescription.DeviceId);

    LogDebug("Creating device using feature level %s on adapter %s",
             PURPL_STRINGIZE_EXPAND(DIRECTX12_TARGET_FEATURE_LEVEL), Dx12Data.AdapterName);

    HRESULT Result =
        D3D12CreateDevice(Dx12Data.Adapter, DIRECTX12_TARGET_FEATURE_LEVEL, IID_PPV_ARGS(&Dx12Data.Device));
    if (!SUCCEEDED(Result))
    {
        LogError("Failed to create hardware device, retrying with WARP adapter: HRESULT 0x%08X", Result);
        HRESULT_CHECK(Dx12Data.Factory->EnumWarpAdapter(IID_PPV_ARGS(&Dx12Data.Adapter)));
        CmnFree(Dx12Data.AdapterName);
        Dx12Data.AdapterName = CmnFormatString("WARP adapter");
        HRESULT_CHECK(
            D3D12CreateDevice(Dx12Data.Adapter, DIRECTX12_TARGET_FEATURE_LEVEL, IID_PPV_ARGS(&Dx12Data.Device)));
    }
    Dx12NameObject(Dx12Data.Device, "Device");
}
