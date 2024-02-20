#include "dx12.h"

EXTERN_C
VOID Dx12CreateCommandQueue(VOID)
{
    LogDebug("Creating command queue");

    D3D12_COMMAND_QUEUE_DESC CommandQueueDescription = {};
    CommandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    CommandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    HRESULT_CHECK(Dx12Data.Device->CreateCommandQueue(&CommandQueueDescription, IID_PPV_ARGS(&Dx12Data.CommandQueue)));
}

EXTERN_C
VOID Dx12CreateCommandAllocators(VOID)
{
    LogDebug("Creating command allocator");

    for (UINT32 i = 0; i < PURPL_ARRAYSIZE(Dx12Data.CommandAllocators); i++)
    {
        HRESULT_CHECK(Dx12Data.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                              IID_PPV_ARGS(&Dx12Data.CommandAllocators[i])));
    }
}

EXTERN_C
VOID Dx12CreateCommandList(VOID)
{
    LogDebug("Creating command lists");

    HRESULT_CHECK(Dx12Data.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Dx12Data.CommandAllocators[0],
                                                     Dx12Data.PipelineState,
                                                     IID_PPV_ARGS(&Dx12Data.CommandList)));
    HRESULT_CHECK(Dx12Data.CommandList->Close());

    HRESULT_CHECK(Dx12Data.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Dx12Data.CommandAllocators[0],
                                                     Dx12Data.PipelineState,
                                                     IID_PPV_ARGS(&Dx12Data.TransferCommandList)));
    HRESULT_CHECK(Dx12Data.TransferCommandList->Close());
}
