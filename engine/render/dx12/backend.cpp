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
    Dx12CreateHeaps();
    Dx12CreateRenderTargetViews();
    Dx12CreateCommandAllocators();
    Dx12CreateRootSignature();
    Dx12CreateCommandLists();
    Dx12CreateMainFence();
    Dx12CreateUniformBuffer();

    LogDebug("Successfully initialized DirectX 12 backend");
}

static VOID WaitForGpu(VOID)
{
    HRESULT_CHECK(Dx12Data.CommandQueue->Signal(Dx12Data.Fence, Dx12Data.FenceValues[Dx12Data.FrameIndex]));

    HRESULT_CHECK(Dx12Data.Fence->SetEventOnCompletion(Dx12Data.FenceValues[Dx12Data.FrameIndex], Dx12Data.FenceEvent));
    WaitForSingleObjectEx(Dx12Data.FenceEvent, INFINITE, FALSE);

    Dx12Data.FenceValues[Dx12Data.FrameIndex]++;
}

static VOID HandleResize(VOID)
{
    WaitForGpu();

    for (UINT32 i = 0; i < DIRECTX12_FRAME_COUNT; i++)
    {
        Dx12Data.RenderTargets[i]->Release();
        Dx12Data.FenceValues[i] = Dx12Data.FenceValues[Dx12Data.FrameIndex];
    }

    DXGI_SWAP_CHAIN_DESC1 SwapChainDescription = {};
    Dx12Data.SwapChain->GetDesc1(&SwapChainDescription);
    HRESULT_CHECK(Dx12Data.SwapChain->ResizeBuffers(DIRECTX12_FRAME_COUNT, RdrGetWidth(), RdrGetHeight(),
                                                    SwapChainDescription.Format, SwapChainDescription.Flags));
    Dx12Data.FrameIndex = (UINT8)Dx12Data.SwapChain->GetCurrentBackBufferIndex();

    Dx12CreateRenderTargetViews();
}

static VOID BeginFrame(_In_ BOOLEAN WindowResized, _In_ PRENDER_SCENE_UNIFORM Uniform)
{
    if (WindowResized)
    {
        HandleResize();
    }

    CONST UINT8 FrameIndex = Dx12Data.FrameIndex;

    HRESULT_CHECK(Dx12Data.CommandAllocators[FrameIndex]->Reset());

    ID3D12GraphicsCommandList7 *CommandList = Dx12Data.CommandList;
    HRESULT_CHECK(CommandList->Reset(Dx12Data.CommandAllocators[FrameIndex], nullptr));

    CommandList->SetGraphicsRootSignature(Dx12Data.RootSignature);

    ID3D12DescriptorHeap *Heaps[] = {Dx12Data.ShaderHeap};
    CommandList->SetDescriptorHeaps(PURPL_ARRAYSIZE(Heaps), Heaps);

    CD3DX12_GPU_DESCRIPTOR_HANDLE UniformHandle(Dx12Data.ShaderHeap->GetGPUDescriptorHandleForHeapStart(),
                                                Dx12Data.FrameIndex + 1, Dx12Data.ShaderDescriptorSize);
    CommandList->SetGraphicsRootDescriptorTable(0, UniformHandle);
    CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    CD3DX12_VIEWPORT Viewport(0.0, 0.0, RdrGetWidth(), RdrGetHeight());
    CommandList->RSSetViewports(1, &Viewport);
    CD3DX12_RECT Scissor(0, 0, RdrGetWidth(), RdrGetHeight());
    CommandList->RSSetScissorRects(1, &Scissor);

    CD3DX12_RESOURCE_BARRIER RenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        Dx12Data.RenderTargets[FrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    CommandList->ResourceBarrier(1, &RenderTargetBarrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE RtvHandle(Dx12Data.RtvHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex,
                                            Dx12Data.RtvDescriptorSize);
    CommandList->OMSetRenderTargets(1, &RtvHandle, FALSE, nullptr);

    UINT64 ClearColourRaw = CONFIGVAR_GET_INT("rdr_clear_colour");
    vec4 ClearColour;
    ClearColour[0] = (UINT8)((ClearColourRaw >> 24) & 0xFF) / 255.0f;
    ClearColour[1] = (UINT8)((ClearColourRaw >> 16) & 0xFF) / 255.0f;
    ClearColour[2] = (UINT8)((ClearColourRaw >> 8) & 0xFF) / 255.0f;
    ClearColour[3] = (UINT8)((ClearColourRaw >> 0) & 0xFF) / 255.0f;
    CommandList->ClearRenderTargetView(RtvHandle, ClearColour, 0, nullptr);
}

static VOID Shutdown(VOID);

static VOID NextFrame(VOID)
{
    CONST UINT8 FrameIndex = Dx12Data.FrameIndex;
    CONST UINT64 CurrentFenceValue = Dx12Data.FenceValues[FrameIndex];
    HRESULT_CHECK(Dx12Data.CommandQueue->Signal(Dx12Data.Fence, CurrentFenceValue));

    Dx12Data.FrameIndex = (UINT8)Dx12Data.SwapChain->GetCurrentBackBufferIndex();

    if (Dx12Data.Fence->GetCompletedValue() < Dx12Data.FenceValues[FrameIndex])
    {
        HRESULT_CHECK(Dx12Data.Fence->SetEventOnCompletion(Dx12Data.FenceValues[FrameIndex], Dx12Data.FenceEvent));
        WaitForSingleObjectEx(Dx12Data.FenceEvent, INFINITE, FALSE);
    }

    Dx12Data.FenceValues[FrameIndex] = CurrentFenceValue + 1;
}

static VOID EndFrame(VOID)
{
    CD3DX12_RESOURCE_BARRIER RenderTargetBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        Dx12Data.RenderTargets[Dx12Data.FrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    Dx12Data.CommandList->ResourceBarrier(1, &RenderTargetBarrier);

    HRESULT_CHECK(Dx12Data.CommandList->Close());

    ID3D12CommandList *CommandLists[] = {Dx12Data.CommandList};
    Dx12Data.CommandQueue->ExecuteCommandLists(PURPL_ARRAYSIZE(CommandLists), CommandLists);

    HRESULT_CHECK(Dx12Data.SwapChain->Present(0, 0));

    NextFrame();
}

static VOID Shutdown(VOID)
{
    UINT32 i;

    LogDebug("Shutting down DirectX 12");

    if (Dx12Data.UniformBuffer.Resource)
    {
        LogDebug("Releasing uniform buffer");
        CD3DX12_RANGE Range(0, 0);
        Dx12Data.UniformBuffer.Resource->Unmap(0, &Range);
        Dx12Data.UniformBuffer.Resource->Release();
    }

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

    if (Dx12Data.ShaderHeap)
    {
        LogDebug("Releasing shader descriptor heap");
        Dx12Data.ShaderHeap->Release();
    }

    if (Dx12Data.RtvHeap)
    {
        LogDebug("Releasing render target view descriptor heap");
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
