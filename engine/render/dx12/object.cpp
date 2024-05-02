#include "dx12.h"

EXTERN_C
VOID Dx12InitializeObject(_In_z_ PCSTR Name, _Inout_ PRENDER_OBJECT_DATA Data, _In_ PMODEL Model)
{
    PDIRECTX12_OBJECT_DATA ObjectData = CmnAllocType(1, DIRECTX12_OBJECT_DATA);
    if (!Data->Handle)
    {
        CmnError("Failed to allocate per-object data for %s: %s", Name, strerror(errno));
    }

    Dx12CreateUniformBuffer(&ObjectData->UniformBuffer, (PVOID *)&ObjectData->UniformBufferAddress,
                            sizeof(DIRECTX12_OBJECT_UNIFORM));
    Dx12NameObject(ObjectData->UniformBuffer.Resource, "Uniform buffer for %s", Name);

    Data->Handle = (RENDER_HANDLE)ObjectData;
}

EXTERN_C
VOID Dx12DestroyObject(_Inout_ PRENDER_OBJECT_DATA Data)
{
    PDIRECTX12_OBJECT_DATA ObjectData = (PDIRECTX12_OBJECT_DATA)Data->Handle;

    CD3DX12_RANGE Range(0, 0);
    ObjectData->UniformBuffer.Resource->Unmap(0, &Range);
    ObjectData->UniformBuffer.Resource->Release();
    CmnFree(ObjectData);
    Data->Handle = 0;
}
