/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    d3d12.cpp

Abstract:

    This module implements the DirectX 12 backend.

--*/

#include "dxgi1_6.h"
#include "directx/d3d12.h"
#include "directx/d3dx12.h"

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#include "render.h"
END_EXTERN_C

#define HRESULT_CHECK(Call, ...) \
    do \
    { \
        HRESULT Result = (Call); \
        if ( !SUCCEEDED(Result) __VA_ARGS__ ) \
        { \
            CmnError("Windows call " #Call " failed: HRESULT 0x%08X", Result); \
        } \
    } while ( 0 );

static ID3D12Debug6* Debug;
static IDXGIFactory7* Factory;

static IDXGIAdapter4** Adapters;
static UINT AdapterIndex;

static ID3D12Device10* Device;

#define REQUIRED_FEATURE_LEVEL D3D_FEATURE_LEVEL_12_0

static
VOID
EnableDebug(
    VOID
    )
{
    LogDebug("Enabling up debug layer");
    HRESULT_CHECK(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
    Debug->EnableDebugLayer();
    Debug->Release();
}

static
VOID
CreateFactory(
    VOID
    )
{
    UINT Flags =
#ifdef PURPL_DEBUG
        DXGI_CREATE_FACTORY_DEBUG;
#else
        0;
#endif

    LogDebug("Creating DXGI factory");
    HRESULT_CHECK(CreateDXGIFactory2(
        Flags,
        IID_PPV_ARGS(&Factory)
        ));
}

static
VOID
EnumerateAdapters(
    VOID
    )
{
    UINT i;
    IDXGIAdapter4* Adapter;
    DXGI_ADAPTER_DESC3 AdapterDescription;
    BOOLEAN Supported;

    LogDebug("Getting adapters");

    i = 0;
    Adapter = NULL;
    while ( Factory->EnumAdapterByGpuPreference(
                i++,
                DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
                IID_PPV_ARGS(&Adapter)
                ) != DXGI_ERROR_NOT_FOUND)
    {
        Supported = SUCCEEDED(D3D12CreateDevice(
            Adapter,
            REQUIRED_FEATURE_LEVEL,
            _uuidof(ID3D12Device),
            nullptr
            ));

        Adapter->GetDesc3(&AdapterDescription);
        LogDebug("Got adapter %d:", i);
        LogDebug("\tName: %ls", AdapterDescription.Description);
        LogDebug("\tPCI ID: %04x:%04x", AdapterDescription.VendorId, AdapterDescription.DeviceId);
        LogDebug("\tSupports DirectX 12: %s", Supported ? "yes" : "no");
        LogDebug("\tVideo memory: %s", CmnFormatSize(AdapterDescription.DedicatedVideoMemory));

        if ( Supported )
        {
            stbds_arrput(
                Adapters,
                Adapter
                );
        }
    }
}

static
VOID
CreateDevice(
    VOID
    )
{
    if ( Device )
    {
        LogDebug("Recreating device");
        Device->Release();
    }
    else
    {
        LogDebug("Creating device");
    }

    HRESULT_CHECK(D3D12CreateDevice(
        Adapters[AdapterIndex],
        REQUIRED_FEATURE_LEVEL,
        IID_PPV_ARGS(&Device)
        ));
}

static
VOID
SetAdapter(
    _In_ UINT Index
    )
{
    AdapterIndex = Index;
    CreateDevice();
}

static
VOID
Initialize(
    VOID
    )
{
    LogDebug("Initializing DirectX 12 backend");

#ifdef PURPL_DEBUG
    EnableDebug();
#endif
    CreateFactory();
    EnumerateAdapters();
    SetAdapter(0);

    LogDebug("DirectX 12 backend initialization succeeded");
}

static
VOID
BeginFrame(
    VOID
    )
{

}

static
VOID
EndFrame(
    VOID
    )
{

}

static
VOID
Shutdown(
    VOID
    )
{
    LogDebug("Shutting down DirectX 12 backend");

    LogDebug("DirectX 12 backend shutdown succeeded");
}

EXTERN_C
VOID
D3d12SetupBackend(
    _In_ PRENDER_BACKEND Backend
    )
{
    memset(
        Backend,
        0,
        sizeof(RENDER_BACKEND)
        );
    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->Shutdown = Shutdown;
}
