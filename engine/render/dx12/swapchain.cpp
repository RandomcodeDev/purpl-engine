#include "dx12.h"

// TODO: if vkd3d is gonna be used, then this should just call VidCreateDx12SwapChain (like VidCreateVulkanSurface)

EXTERN_C
VOID Dx12CreateSwapChain(VOID)
{
    LogDebug("Creating %ux%u swap chain", RdrGetWidth(), RdrGetHeight());

    DXGI_SWAP_CHAIN_DESC1 SwapChainDescription = {};
    SwapChainDescription.Width = RdrGetWidth();
    SwapChainDescription.Height = RdrGetHeight();
    SwapChainDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDescription.Stereo = FALSE;
    SwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescription.BufferCount = DIRECTX12_FRAME_COUNT;
    SwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    SwapChainDescription.Scaling = DXGI_SCALING_STRETCH;
    SwapChainDescription.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
    SwapChainDescription.SampleDesc.Count = 1;

    IDXGISwapChain1 *SwapChain;
    HRESULT_CHECK(Dx12Data.Factory->CreateSwapChainForHwnd(
        Dx12Data.CommandQueue, (HWND)VidGetObject(), &SwapChainDescription, nullptr, nullptr, &SwapChain));

    Dx12Data.SwapChain = static_cast<decltype(Dx12Data.SwapChain)>(SwapChain);

    Dx12Data.FrameIndex = (UINT8)Dx12Data.SwapChain->GetCurrentBackBufferIndex();
}
