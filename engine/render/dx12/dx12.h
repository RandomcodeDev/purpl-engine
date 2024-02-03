/// @file dx12.h
///
/// @brief This module contains definitions used by the DirectX 12 backend.
///
/// @copyright (c) 2024 Randomcode Developers

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

/// @brief Hard error if an HRESULT isn't a success value
///
/// @param[in] Call The call/expression to check
/// @param[in] ... Anything extra to put in the if statement
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

// Swap chains are different in vkd3d

#ifdef PURPL_WIN32
typedef IDXGISwapChain1 DIRECTX12_SWAPCHAIN;
#else
#include "vkd3d.h"
#include "vkd3d_sonames.h"
#include "vkd3d_swapchain_factory.h"

typedef IDXGIVkSwapChain DIRECTX12_SWAPCHAIN;
#endif

/// @brief Data for the DirectX 12 backend
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

/// @brief Enable the debug layer
extern VOID Dx12EnableDebugLayer(VOID);

/// @brief Enumerate the adapters and keep ones that are usable
extern VOID Dx12EnumerateAdapters(VOID);

/// @brief Set the current adapter (have to recreate the device and everything else)
///
/// @param[in] Index The index of the adapter
extern VOID Dx12SetDevice(SIZE_T Index);

/// @brief Create the device
extern VOID Dx12CreateDevice(VOID);

/// @brief Create the command queue
extern VOID Dx12CreateCommandQueue(VOID);

/// @brief Create the command allocator
extern VOID Dx12CreateCommandAllocator(VOID);

/// @brief Create the command list
extern VOID Dx12CreateCommandList(VOID);

/// @brief Create the swap chain
extern VOID Dx12CreateSwapChain(VOID);

/// @brief Create the render target view heap
extern VOID Dx12CreateRenderTargetViewHeap(VOID);

/// @brief Create the render target views
extern VOID Dx12CreateRenderTargetViews(VOID);

/// @brief Create the root signature (defines what shaders receive as parameters)
extern VOID Dx12CreateRootSignature(VOID);

/// @brief Load core shaders
extern VOID Dx12LoadCoreShaders(VOID);

/// @brief Create the vertex input layout
extern VOID Dx12CreateVertexInputLayout(VOID);

/// @brief Create the pipeline state object
extern VOID Dx12CreatePipelineStateObject(VOID);

/// @brief Create the fence
extern VOID Dx12CreateMainFence(VOID);

END_EXTERN_C
