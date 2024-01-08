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
            CmnError("Call " #Call " failed: HRESULT 0x%08X", Result); \
        } \
    } while ( 0 );

#define FRAME_COUNT 3

// Global resouces

static ID3D12Debug6* Debug;
static IDXGIFactory7* Factory;
static IDXGIAdapter4** Adapters;
static UINT AdapterIndex;

// Device resources
static ID3D12Device10* Device;
static ID3D12CommandQueue* CommandQueue;
// Is it swap chain or swapchain? DirectX (and Visual Studio's autocorrect) has it as two words, but Vulkan has it as one.
static IDXGISwapChain4* SwapChain;
static UINT SwapChainIndex;
static ID3D12DescriptorHeap* RenderTargetViewHeap;
static UINT RtvDescriptorHeapSize;
static ID3D12Resource* RenderTargetViews[FRAME_COUNT];
static ID3D12CommandAllocator* CommandAllocator;

// DirectX 12 Ultimate
#define REQUIRED_FEATURE_LEVEL D3D_FEATURE_LEVEL_12_2

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
        LogDebug("Adapter %d:", i);
        LogDebug("\tName: %ls", AdapterDescription.Description);
        LogDebug("\tPCI ID: %04x:%04x", AdapterDescription.VendorId, AdapterDescription.DeviceId);
        LogDebug("\tSupports " PURPL_STRINGIZE_EXPAND(REQUIRED_FEATURE_LEVEL) ": %s", Supported ? "yes" : "no");
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
        LogDebug("Recreating device with adapter %d", AdapterIndex);
        Device->Release();
    }
    else
    {
        LogDebug("Creating device with adapter %d", AdapterIndex);
    }

    HRESULT_CHECK(D3D12CreateDevice(
        Adapters[AdapterIndex],
        REQUIRED_FEATURE_LEVEL,
        IID_PPV_ARGS(&Device)
        ));
}

static
VOID
CreateCommandQueue(
    VOID
    )
{
    if ( CommandQueue )
    {
        LogDebug("Recreating command queue");
        CommandQueue->Release();
    }
    else
    {
        LogDebug("Creating command queue");
    }

    D3D12_COMMAND_QUEUE_DESC QueueDescription = {};
    QueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    QueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    HRESULT_CHECK(Device->CreateCommandQueue(
        &QueueDescription,
        IID_PPV_ARGS(&CommandQueue)
        ));
}

static
VOID
CreateSwapChain(
    VOID
    )
{
    if ( SwapChain )
    {
        LogDebug("Recreating swap chain");
        SwapChain->Release();
    }
    else
    {
        LogDebug("Creating swap chain");
    }

    DXGI_SWAP_CHAIN_DESC1 SwapChainDescription = {};
    VidGetSize(
        &SwapChainDescription.Width,
        &SwapChainDescription.Height
        );
    SwapChainDescription.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    SwapChainDescription.BufferCount = FRAME_COUNT;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDescription.SampleDesc.Count = 1;

    HRESULT_CHECK(Factory->CreateSwapChainForHwnd(
        CommandQueue,
        (HWND)VidGetObject(),
        &SwapChainDescription,
        nullptr,
        nullptr,
        (IDXGISwapChain1**)&SwapChain
        ));
    SwapChainIndex = SwapChain->GetCurrentBackBufferIndex();
}

static
VOID
CreateRtvDescriptorHeap(
    VOID
    )
{
    if ( RenderTargetViewHeap )
    {
        LogDebug("Recreating RTV descriptor heap");
        RenderTargetViewHeap->Release();
    }
    else
    {
        LogDebug("Creating RTV descriptor heap");
    }

    D3D12_DESCRIPTOR_HEAP_DESC RenderTargetViewHeapDescription = {};
    RenderTargetViewHeapDescription.NumDescriptors = FRAME_COUNT;
    RenderTargetViewHeapDescription.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    RenderTargetViewHeapDescription.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    HRESULT_CHECK(Device->CreateDescriptorHeap(
        &RenderTargetViewHeapDescription,
        IID_PPV_ARGS(&RenderTargetViewHeap)
        ));
    RtvDescriptorHeapSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

static
VOID
CreateRenderTargetViews(
    VOID
    )
{

}

static
VOID
SetAdapter(
    _In_ UINT Index
    )
{
    if ( !Factory )
    {
        LogError("CreateFactory has not been called, not doing anything");
        return;
    }

    if ( !Adapters )
    {
        LogError("No adapters stored, did you call EnumerateAdapters");
        return;
    }

    AdapterIndex = Index;
    CreateDevice();
    CreateCommandQueue();
    CreateSwapChain();
    CreateRtvDescriptorHeap();
    CreateRenderTargetViews();
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
    UINT i;

    LogDebug("Shutting down DirectX 12 backend");

    if ( RenderTargetViewHeap )
    {
        LogDebug("Releasing RTV descriptor heap");
        RenderTargetViewHeap->Release();
    }

    if ( SwapChain )
    {
        LogDebug("Releasing swap chain");
        SwapChain->Release();
    }

    if ( CommandQueue )
    {
        LogDebug("Releasing command queue");
        CommandQueue->Release();
    }

    if ( Device )
    {
        LogDebug("Releasing device");
        Device->Release();
    }

    if ( Adapters )
    {
        LogDebug("Releasing %zu adapters", stbds_arrlenu(Adapters));
        for ( i = 0; i < stbds_arrlenu(Adapters); i++ )
        {
            LogDebug("Releasing adapter %u", i);
            Adapters[i]->Release();
        }
        stbds_arrfree(Adapters);
    }

    if ( Factory )
    {
        LogDebug("Releasing factory");
        Factory->Release();
    }

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
