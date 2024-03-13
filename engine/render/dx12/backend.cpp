/**
@file
@brief This file manages the DirectX 12 backend
*/

// Define GUIDs used
#define INITGUID

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
    Dx12CreateCommandLists();
    Dx12CreateMainFence();

    LogDebug("Successfully initialized DirectX 12 backend");
}

static VOID WaitForGpu(VOID)
{
    HRESULT_CHECK(Dx12Data.CommandQueue->Signal(Dx12Data.Fence, Dx12Data.FenceValues[Dx12Data.FrameIndex]));

    HRESULT_CHECK(Dx12Data.Fence->SetEventOnCompletion(Dx12Data.FenceValues[Dx12Data.FrameIndex], Dx12Data.FenceEvent));
    WaitForSingleObjectEx(Dx12Data.FenceEvent, INFINITE, FALSE);

    Dx12Data.FenceValues[Dx12Data.FrameIndex]++;
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

    if (Dx12Data.FenceEvent)
    {
        LogDebug("Closing fence event");
        CloseHandle(Dx12Data.FenceEvent);
    }

    if (Dx12Data.Fence)
    {
        LogDebug("Releasing fence");
        Dx12Data.Fence->Release();
    }

    if (Dx12Data.TransferCommandList)
    {
        LogDebug("Releasing transfer command list");
        Dx12Data.TransferCommandList->Release();
    }

    if (Dx12Data.CommandList)
    {
        LogDebug("Releasing command list");
        Dx12Data.CommandList->Release();
    }

    if (Dx12Data.RootSignature)
    {
        LogDebug("Releasing root signature");
        Dx12Data.RootSignature->Release();
    }

    if (Dx12Data.TransferCommandAllocator)
    {
        LogDebug("Releasing transfer command allocator");
        Dx12Data.TransferCommandAllocator->Release();
    }

    if (Dx12Data.CommandAllocators)
    {
        for (i = 0; i < PURPL_ARRAYSIZE(Dx12Data.CommandAllocators); i++)
        {
            LogDebug("Releasing command allocator %u/%u", i + 1, PURPL_ARRAYSIZE(Dx12Data.CommandAllocators));
            Dx12Data.CommandAllocators[i]->Release();
        }
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

#ifdef PURPL_DEBUG
    IDXGIDebug1 *Debug;
    HRESULT Result = DXGIGetDebugInterface1(0, IID_PPV_ARGS(&Debug));
    if (SUCCEEDED(Result))
    {
        DXGI_DEBUG_RLO_FLAGS Verbosity;
        if (CONFIGVAR_GET_BOOLEAN("verbose"))
        {
            Verbosity = DXGI_DEBUG_RLO_ALL;
        }
        else
        {
            Verbosity = DXGI_DEBUG_RLO_DETAIL;
        }
        Debug->ReportLiveObjects(DXGI_DEBUG_ALL, Verbosity);
    }
#endif

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

    Backend->LoadShader = Dx12LoadShader;
    Backend->DestroyShader = Dx12DestroyShader;

    memset(&Dx12Data, 0, sizeof(DIRECTX12_DATA));
}
