#include "dx12.h"

EXTERN_C
VOID Dx12CreateCommandQueue(VOID)
{
    LogDebug("Creating command queue");

    D3D12_COMMAND_QUEUE_DESC CommandQueueDescription = {};
    CommandQueueDescription.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    CommandQueueDescription.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    HRESULT_CHECK(Dx12Data.Device->CreateCommandQueue(&CommandQueueDescription, IID_PPV_ARGS(&Dx12Data.CommandQueue)));
    Dx12NameObject(Dx12Data.CommandQueue, "Command queue");
}

EXTERN_C
VOID Dx12CreateCommandAllocators(VOID)
{
    LogDebug("Creating command allocators");

    for (UINT32 i = 0; i < PURPL_ARRAYSIZE(Dx12Data.CommandAllocators); i++)
    {
        HRESULT_CHECK(Dx12Data.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                              IID_PPV_ARGS(&Dx12Data.CommandAllocators[i])));
        Dx12NameObject(Dx12Data.CommandAllocators[i], "Main command allocator %u", i);
    }

    HRESULT_CHECK(Dx12Data.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                          IID_PPV_ARGS(&Dx12Data.TransferCommandAllocator)));
    Dx12NameObject(Dx12Data.TransferCommandAllocator, "Transfer command allocator");
}

EXTERN_C
VOID Dx12CreateCommandLists(VOID)
{
    LogDebug("Creating command lists");

    HRESULT_CHECK(Dx12Data.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                     Dx12Data.CommandAllocators[Dx12Data.FrameIndex], NULL,
                                                     IID_PPV_ARGS(&Dx12Data.CommandList)));
    Dx12NameObject(Dx12Data.CommandList, "Main command list");
    HRESULT_CHECK(Dx12Data.CommandList->Close());

    HRESULT_CHECK(Dx12Data.Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                     Dx12Data.TransferCommandAllocator,
                                                     NULL, IID_PPV_ARGS(&Dx12Data.TransferCommandList)));
    Dx12NameObject(Dx12Data.TransferCommandList, "Transfer command list");
    HRESULT_CHECK(Dx12Data.TransferCommandList->Close());
    Dx12Data.TransferCommandList->Reset(Dx12Data.TransferCommandAllocator, nullptr);
}
