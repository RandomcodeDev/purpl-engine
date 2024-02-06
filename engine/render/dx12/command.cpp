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
VOID Dx12CreateCommandAllocator(VOID)
{
    LogDebug("Creating command allocator");
    HRESULT_CHECK(Dx12Data.Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                                          IID_PPV_ARGS(&Dx12Data.CommandAllocator)));
}

EXTERN_C
VOID Dx12CreateCommandList(VOID)
{
}
