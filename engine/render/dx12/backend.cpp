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
    LogDebug("Initializing DirectX 12 backend");

#ifdef PURPL_DEBUG
    Dx12EnableDebugLayer();
#endif

    LogDebug("Creating DXGI factory");
    HRESULT_CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&Dx12Data.Factory)));

    Dx12EnumerateAdapters();
    Dx12CreateDevice();
    Dx12CreateCommandQueue();
    Dx12CreateSwapChain();
    Dx12CreateRenderTargetViewHeap();
    Dx12CreateRenderTargetViews();
    Dx12CreateCommandAllocator();
    Dx12CreateRootSignature();
    Dx12LoadCoreShaders();
    Dx12CreateVertexInputLayout();
    Dx12CreatePipelineStateObject();
    Dx12CreateCommandList();
    Dx12CreateMainFence();

    LogDebug("Successfully initialized DirectX 12 backend");
}

static VOID BeginFrame(VOID)
{
}

static VOID EndFrame(VOID)
{
}

static VOID Shutdown(VOID)
{
}

EXTERN_C
VOID Dx12InitializeBackend(_Out_ PRENDER_BACKEND Backend)
{
    LogDebug("Filling out render backend for DirectX 12");

    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->Shutdown = Shutdown;
}
