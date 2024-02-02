/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    dx12.h

Abstract:

    This module contains definitions used by the DirectX 12 backend.

--*/

#pragma once

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#include "common/alloc.h"
#include "common/common.h"
#include "common/log.h"

#include "engine/render/render.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"
END_EXTERN_C

#define D3D12MA_SYSTEM_ALIGNED_MALLOC CmnAlignedAlloc
#define D3D12MA_SYSTEM_ALIGNED_FREE CmnAlignedFree
#define D3D12MA_DEBUG_LOG_FORMAT(format, ...) LogDebug((format), __VA_ARGS__)
#define D3D12MA_HEAVY_ASSERT(expr)                                             \
    if (!(expr))                                                               \
    {                                                                          \
        CmnError("D3D12MA assertion failed: " #expr);                          \
    }
#ifdef PURPL_DEBUG
#define D3D12MA_STATS_STRING_ENABLED 1
#endif
#define D3D12MA_USING_DIRECTX_HEADERS 1
#include "D3D12MemAlloc.h"

#define DIRECTX12_FRAME_COUNT 3
#define DIRECTX12_TARGET_FEATURE_LEVEL D3D_FEATURE_LEVEL_12_1

//
// Check if something failed
//

#define HRESULT_CHECK(Call, ...)                                               \
    do                                                                         \
    {                                                                          \
        HRESULT Result = (Call);                                               \
        if (!SUCCEEDED(Result) __VA_ARGS__)                                    \
        {                                                                      \
            _com_error Error(Result);                                          \
            CmnError("COM call " #Call                                         \
                     " at %s:%d failed: %s (HRESULT 0x%08X)",                  \
                     __FILE__, __LINE__, Error.ErrorMessage(), Result);        \
        }                                                                      \
    } while (0)

//
// Swap chains are different under vkd3d
//

#ifdef PURPL_WIN32
typedef IDXGISwapChain1 DIRECTX12_SWAPCHAIN;
#else
#include "vkd3d.h"
#include "vkd3d_sonames.h"
#include "vkd3d_swapchain_factory.h"

typedef IDXGIVkSwapChain DIRECTX12_SWAPCHAIN;
#endif

typedef struct DIRECTX12_DATA
{
    IDXGIFactory4 *Factory;
    IDXGIAdapter3 *Adapters;
    ID3D12Device *Device;
    DIRECTX12_SWAPCHAIN *SwapChain;
    ID3D12Resource *RenderTargets[DIRECTX12_FRAME_COUNT];
    ID3D12CommandAllocator *CommandAllocator;
    ID3D12CommandQueue *CommandQueue;
    ID3D12RootSignature *RootSignature;
    ID3D12DescriptorHeap *RenderTargetViewHeap;
    ID3D12PipelineState *PipelineState;
    ID3D12GraphicsCommandList9 *CommandList;

    UINT32 FrameIndex;
    PVOID FenceEvent;
    ID3D12Fence *Fence;
    UINT64 FenceValue;

    BOOLEAN Initialized;
} DIRECTX12_DATA, *PDIRECTX12_DATA;

extern DIRECTX12_DATA Dx12Data;

BEGIN_EXTERN_C

//
// Enable the debug layer
//

extern VOID Dx12EnableDebugLayer(VOID);

//
// Get adapters
//

extern VOID Dx12EnumerateAdapters(VOID);

//
// Set the device
//

extern VOID Dx12SetDevice(SIZE_T Index);

//
// Create the device
//

extern VOID Dx12CreateDevice(VOID);

//
// Create the command queue
//

extern VOID Dx12CreateCommandQueue(VOID);

//
// Create the command allocator
//

extern VOID Dx12CreateCommandAllocator(VOID);

//
// Create the command list
//

extern VOID Dx12CreateCommandList(VOID);

//
// Create the swap chain
//

extern VOID Dx12CreateSwapChain(VOID);

//
// Create the render target view heap
//

extern VOID Dx12CreateRenderTargetViewHeap(VOID);

//
// Create the render target views
//

extern VOID Dx12CreateRenderTargetViews(VOID);

//
// Create the root signature (defines what shaders receive as parameters)
//

extern VOID Dx12CreateRootSignature(VOID);

//
// Load core shaders
//

extern VOID Dx12LoadCoreShaders(VOID);

//
// Create the vertex input layout
//

extern VOID Dx12CreateVertexInputLayout(VOID);

//
// Create the pipeline state object
//

extern VOID Dx12CreatePipelineStateObject(VOID);

//
// Create the fence
//

extern VOID Dx12CreateMainFence(VOID);

END_EXTERN_C
