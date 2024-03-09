/// @file dx12.h
///
/// @brief This file contains definitions used by the DirectX 12 backend.
///
/// All the DirectX 12 files are C++, but only because COM code in C looks ugly.
/// No other C++ features are used, and it's still C. It's just to call COM functions
/// in a way that looks OK (plus the semantics are slightly better, because the
/// variable can be used instead of just the interface type when creating objects,
/// which means they don't have to be kept in sync).
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

BEGIN_EXTERN_C
#include "common/alloc.h"
#include "common/common.h"
#include "common/log.h"

#include "engine/engine.h"

#include "engine/render/render.h"

#include "platform/video.h"

#include "util/mesh.h"
#include "util/texture.h"
END_EXTERN_C

#include "directx/d3d12.h"
#include "directx/d3dx12.h"
#include <d3dcompiler.h>
#include <dxgi1_6.h>

#define D3D12MA_SYSTEM_ALIGNED_MALLOC CmnAlignedAlloc
#define D3D12MA_SYSTEM_ALIGNED_FREE CmnAlignedFree
#define D3D12MA_DEBUG_LOG_FORMAT(format, ...) LogDebug((format), __VA_ARGS__)
#define D3D12MA_HEAVY_ASSERT(expr)                                                                                     \
    if (!(expr))                                                                                                       \
    {                                                                                                                  \
        CmnError("D3D12MA assertion failed: " #expr);                                                                  \
    }
#ifdef PURPL_DEBUG
#define D3D12MA_STATS_STRING_ENABLED 1
#endif
#define D3D12MA_USING_DIRECTX_HEADERS 1
#include "D3D12MemAlloc.h"

#define DIRECTX12_FRAME_COUNT 3
#ifdef PURPL_GDKX
// Xbox always has DirectX 12 Ultimate
#define DIRECTX12_TARGET_FEATURE_LEVEL D3D_FEATURE_LEVEL_12_2
#else
#define DIRECTX12_TARGET_FEATURE_LEVEL D3D_FEATURE_LEVEL_12_1
#endif

/// @brief Hard error if an HRESULT isn't a success value
///
/// @param[in] Call The call/expression to check
/// @param[in] ... Anything extra to put in the if statement
#define HRESULT_CHECK(Call, ...)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        HRESULT Result_ = (Call);                                                                                      \
        if (!SUCCEEDED(Result_) __VA_ARGS__)                                                                           \
        {                                                                                                              \
            _com_error Error_(Result_);                                                                                \
            CmnError("COM call " #Call " at %s:%d failed: %s (HRESULT 0x%08X)", __FILE__, __LINE__,                    \
                     Error_.ErrorMessage(), Result_);                                                                  \
        }                                                                                                              \
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
    IDXGIFactory7 *Factory;
    IDXGIAdapter1 *Adapter;
    DXGI_ADAPTER_DESC AdapterDescription;
    ID3D12Device7 *Device;
    DIRECTX12_SWAPCHAIN *SwapChain;
    ID3D12CommandAllocator *CommandAllocators[DIRECTX12_FRAME_COUNT];
    ID3D12CommandQueue *CommandQueue;
    ID3D12RootSignature *RootSignature;
    ID3D12DescriptorHeap *RtvHeap;
    UINT32 RtvDescriptorSize;
    ID3D12Resource *RenderTargets[DIRECTX12_FRAME_COUNT];
    ID3D12PipelineState *PipelineState;
    ID3D12GraphicsCommandList7 *CommandList;
    ID3D12GraphicsCommandList7 *TransferCommandList;

    UINT32 FrameIndex;
    PVOID FenceEvent;
    ID3D12Fence *Fence;
    UINT64 FenceValues[DIRECTX12_FRAME_COUNT];

    BOOLEAN Initialized;
} DIRECTX12_DATA, *PDIRECTX12_DATA;

extern DIRECTX12_DATA Dx12Data;

/// @brief A buffer
typedef struct DIRECTX12_BUFFER
{
    ID3D12Resource *Resource;
    UINT64 Size;
} DIRECTX12_BUFFER, *PDIRECTX12_BUFFER;

BEGIN_EXTERN_C

/// @brief Enable the debug layer
extern VOID Dx12EnableDebugLayer(VOID);

/// @brief Create the device
extern VOID Dx12CreateDevice(VOID);

/// @brief Create the command queue
extern VOID Dx12CreateCommandQueue(VOID);

/// @brief Create the command allocator
extern VOID Dx12CreateCommandAllocators(VOID);

/// @brief Create the command lists
extern VOID Dx12CreateCommandList(VOID);

/// @brief Create the swap chain
extern VOID Dx12CreateSwapChain(VOID);

/// @brief Create the render target view heap
extern VOID Dx12CreateRtvHeap(VOID);

/// @brief Create the render target views
extern VOID Dx12CreateRenderTargetViews(VOID);

/// @brief Create the root signature (defines what shaders receive as parameters)
extern VOID Dx12CreateRootSignature(VOID);

/// @brief Determine if the pipeline state has been cached before
extern BOOLEAN Dx12HavePipelineStateCache(VOID);

/// @brief Load the cached pipeline state
extern VOID Dx12LoadPipelineStateCache(VOID);

/// @brief Create the pipeline state object
extern VOID Dx12CreatePipelineStateObject(VOID);

/// @brief Cache the pipeline state
extern VOID Dx12CachePipelineState(VOID);

/// @brief Create the fence
extern VOID Dx12CreateMainFence(VOID);

/// @brief Create a buffer
///
/// @param[out] Buffer The buffer to initialize
/// @param[in] Size The size of the buffer
/// @param[in] HeapProperties The heap properties to use
/// @param[in] HeapFlags The heap flags to use
/// @param[in] ResourceDescription The resource description to use
/// @param[in] ResourceState The state to put the buffer resource in
extern VOID Dx12CreateBuffer(_Out_ PDIRECTX12_BUFFER Buffer, _In_ UINT64 Size,
                             _In_ CONST D3D12_HEAP_PROPERTIES *HeapProperties, _In_ D3D12_HEAP_FLAGS HeapFlags,
                             _In_ CONST D3D12_RESOURCE_DESC *ResourceDescription,
                             _In_ D3D12_RESOURCE_STATES ResourceState);

/// @brief Copy data to a CPU-visible buffer
///
/// @param Buffer The buffer to copy to
/// @param Data The data to copy to the buffer
/// @param Size The size of the data
extern VOID Dx12CopyDataToCpuBuffer(_Inout_ PDIRECTX12_BUFFER Buffer, _In_ PVOID Data, _In_ UINT64 Size);

/// @brief Copy data to an upload buffer and copy that to a GPU-only buffer
///
/// @param Buffer The buffer to upload to
/// @param Data The data to upload
/// @param Size The size of the data
extern VOID Dx12UploadDataToBuffer(_Inout_ PDIRECTX12_BUFFER Buffer, _In_ PVOID Data, _In_ UINT64 Size);

/// @brief Create a buffer with data in it
///
/// @param[out] Buffer The buffer to initialize
/// @param[in] Data The data to copy to the buffer
/// @param[in] Size The size of the buffer
/// @param[in] HeapProperties The heap properties to use
/// @param[in] HeapFlags The heap flags to use
/// @param[in] ResourceDescription The resource description to use
/// @param[in] ResourceState The state to put the buffer resource in
extern VOID Dx12CreateBufferWithData(_Out_ PDIRECTX12_BUFFER Buffer, PVOID Data, _In_ SIZE_T Size,
                              _In_ CONST D3D12_HEAP_PROPERTIES *HeapProperties, _In_ D3D12_HEAP_FLAGS HeapFlags,
                              _In_ CONST D3D12_RESOURCE_DESC *ResourceDescription,
                              _In_ D3D12_RESOURCE_STATES ResourceState);

END_EXTERN_C
