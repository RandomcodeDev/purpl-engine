/**
@file
@brief This file manages the DirectX 12 backend
*/

#include "dx12.h"

DIRECTX12_DATA Dx12Data;

/**

*/
static VOID Initialize(VOID)
{
    if (Dx12Data.Initialized)
    {
        return;
    }

    LogDebug("Initializing DirectX 12 backend");

#ifdef PURPL_DEBUG
    Dx12EnableDebugLayer();
#endif

    LogDebug("Creating DXGI factory");
#ifdef PURPL_DEBUG
    CONST UINT32 FactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#else
    CONST UINT32 FactoryFlags = 0;
#endif
    HRESULT_CHECK(CreateDXGIFactory2(FactoryFlags, IID_PPV_ARGS(&Dx12Data.Factory)));

    Dx12CreateDevice();
    Dx12CreateCommandQueue();
    Dx12CreateSwapChain();
    Dx12CreateRtvHeap();
    Dx12CreateRenderTargetViews();
    Dx12CreateCommandAllocators();
    Dx12CreateRootSignature();
    if (Dx12HavePipelineStateCache())
    {
        Dx12LoadPipelineStateCache();
    }
    else
    {
        Dx12CreatePipelineStateObject();
        Dx12CachePipelineState();
    }
    Dx12CreateCommandList();
    Dx12CreateMainFence();

    LogDebug("Successfully initialized DirectX 12 backend");
}

static VOID BeginFrame(_In_ BOOLEAN WindowResized)
{
    UNREFERENCED_PARAMETER(WindowResized);
}

static VOID EndFrame(VOID)
{
}

static VOID Shutdown(VOID)
{
    UINT32 i;

    LogDebug("Shutting down DirectX 12");

    for (i = 0; i < PURPL_ARRAYSIZE(Dx12Data.Fences); i++)
    {
        if (Dx12Data.Fences[i])
        {
            LogDebug("Releasing command list %u/%u", i + 1, PURPL_ARRAYSIZE(Dx12Data.Fences));
            Dx12Data.Fences[i]->Release();
        }
    }

    for (i = 0; i < PURPL_ARRAYSIZE(Dx12Data.CommandLists); i++)
    {
        if (Dx12Data.CommandLists[i])
        {
            LogDebug("Releasing command list %u/%u", i + 1, PURPL_ARRAYSIZE(Dx12Data.CommandLists));
            Dx12Data.CommandLists[i]->Release();
        }
    }

    if (Dx12Data.PipelineState)
    {
        LogDebug("Releasing pipeline state object");
        Dx12Data.PipelineState->Release();
    }

    if (Dx12Data.RootSignature)
    {
        LogDebug("Releasing root signature");
        Dx12Data.RootSignature->Release();
    }

    if (Dx12Data.CommandAllocator)
    {
        LogDebug("Releasing command allocator");
        Dx12Data.CommandAllocator->Release();
    }

    for (i = 0; i < PURPL_ARRAYSIZE(Dx12Data.RenderTargets); i++)
    {
        if (Dx12Data.RenderTargets[i])
        {
            LogDebug("Releasing render target %u/%u", i + 1, PURPL_ARRAYSIZE(Dx12Data.RenderTargets));
            Dx12Data.RenderTargets[i]->Release();
        }
    }

    if (Dx12Data.RtvHeap)
    {
        LogDebug("Releasing render target view heap");
        Dx12Data.RtvHeap->Release();
    }

    if (Dx12Data.SwapChain)
    {
        LogDebug("Releasing swap chain");
        Dx12Data.SwapChain->Release();
    }

    if (Dx12Data.CommandQueue)
    {
        LogDebug("Releasing command queue");
        Dx12Data.CommandQueue->Release();
    }

    if (Dx12Data.Device)
    {
        LogDebug("Releasing device");
        Dx12Data.Device->Release();
    }

    if (Dx12Data.Factory)
    {
        LogDebug("Releasing factory");
        Dx12Data.Factory->Release();
    }

    if (Dx12Data.Adapter)
    {
        LogDebug("Releasing adapter");
        Dx12Data.Adapter->Release();
    }

    memset(&Dx12Data, 0, sizeof(DIRECTX12_DATA));

    LogDebug("Successfully shut down DirectX 12");
}

EXTERN_C
VOID Dx12InitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    LogDebug("Filling out render backend for DirectX 12");

    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->Shutdown = Shutdown;

    memset(&Dx12Data, 0, sizeof(DIRECTX12_DATA));
}
