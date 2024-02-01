#include "dx12.h"

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

    HRESULT_CHECK(Dx12Data.Factory->CreateSwapChainForHwnd(
        Dx12Data.CommandQueue, (HWND)VidGetObject(), &SwapChainDescription,
        nullptr, nullptr, &Dx12Data.SwapChain));
}
