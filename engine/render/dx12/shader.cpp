#include "dx12.h"

EXTERN_C
VOID Dx12CreateRootSignature(VOID)
{
    ID3DBlob *Signature;
    ID3DBlob *Error;

    LogDebug("Creating root signature");

    CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDescription;
    RootSignatureDescription.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    HRESULT_CHECK(
        D3D12SerializeRootSignature(&RootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &Error));
    HRESULT_CHECK(Dx12Data.Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(),
                                                       IID_PPV_ARGS(&Dx12Data.RootSignature)));
}

EXTERN_C
ID3DBlob *Dx12LoadShader(_In_ PCSTR Path)
{
    PBYTE ShaderData;
    SIZE_T ShaderSize;
    ID3DBlob *ShaderBlob;

    LogDebug("Loading shader bytecode in %s", Path);

    ShaderSize = 0;
    ShaderData = (PBYTE)FsReadFile(Path, 0, &ShaderSize, 0);
    PURPL_ASSERT(ShaderData != nullptr && ShaderSize > 0);

    HRESULT_CHECK(D3DCreateBlob(ShaderSize, &ShaderBlob));

    memcpy(ShaderBlob->GetBufferPointer(), ShaderData, ShaderSize);

    CmnFree(ShaderData);

    return ShaderBlob;
}
