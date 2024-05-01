#include "dx12.h"

EXTERN_C
VOID Dx12InitializeObject(_Inout_ PRENDER_OBJECT_DATA Data)
{
    PDIRECTX12_OBJECT_DATA ObjectData = CmnAllocType(1, DIRECTX12_OBJECT_DATA);
    if (!Data->Data)
    {
        CmnError("Failed to allocate per-object data: %s", strerror(errno));
    }

    Dx12CreateUniformBuffer(&ObjectData->UniformBuffer, (PVOID *)&ObjectData->UniformBufferAddress,
                            sizeof(DIRECTX12_OBJECT_UNIFORM));
    Dx12NameObject(ObjectData->UniformBuffer.Resource, "Uniform buffer %s", Name);

    Data->Data = (RENDER_HANDLE)ObjectData;
}

EXTERN_C
VOID Dx12DestroyObject(_Inout_ PRENDER_OBJECT_DATA Data)
{
    PDIRECTX12_OBJECT_DATA ObjectData = (PDIRECTX12_OBJECT_DATA)Data->Data;

    CD3DX12_RANGE Range(0, 0);
    ObjectData->UniformBuffer.Resource->Unmap(0, &Range);
    ObjectData->UniformBuffer.Resource->Release();
    CmnFree(ObjectData);
    Data->Data = 0;
}

