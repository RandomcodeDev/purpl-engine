#include "dx12.h"

EXTERN_C
VOID Dx12CreateRootSignature(VOID)
{
    ID3DBlob *Signature;
    ID3DBlob *Error;

    LogDebug("Creating root signature");

    CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDescription;
    RootSignatureDescription.Init(
        0, nullptr, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    HRESULT_CHECK(D3D12SerializeRootSignature(&RootSignatureDescription,
                                              D3D_ROOT_SIGNATURE_VERSION_1,
                                              &Signature, &Error));
    HRESULT_CHECK(Dx12Data.Device->CreateRootSignature(
        0, Signature->GetBufferPointer(), Signature->GetBufferSize(),
        IID_PPV_ARGS(&Dx12Data.RootSignature)));
}

EXTERN_C
VOID Dx12LoadCoreShaders(VOID)
{
    PCHAR VertexShaderData;
    SIZE_T VertexShaderSize;
    PCHAR PixelShaderData;
    SIZE_T PixelShaderSize;
    ID3DBlob* VertexShader;
    ID3DBlob* PixelShader;

#ifdef PURPL_DEBUG
    CONST UINT32 CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    CONST UINT32 CompileFlags = 0;
#endif

    LogDebug("Reading shader bytecode");

    VertexShaderSize = 0;
    VertexShaderData = (PCHAR)FsReadFile("assets/shaders/directx12/shaders.vs.cso", 0, &VertexShaderSize, 0);
    PURPL_ASSERT(VertexShaderData != nullptr && VertexShaderSize > 0);

    PixelShaderSize = 0;
    PixelShaderData = (PCHAR)FsReadFile("assets/shaders/directx12/shaders.ps.cso", 0, &PixelShaderSize, 0);
    PURPL_ASSERT(PixelShaderData != nullptr && PixelShaderSize > 0);

    HRESULT_CHECK(D3DCreateBlob(VertexShaderSize, &VertexShader));
    HRESULT_CHECK(D3DCreateBlob(PixelShaderSize, &PixelShader));
    
    memcpy(VertexShader->GetBufferPointer(), VertexShaderData, VertexShaderSize);
    memcpy(PixelShader->GetBufferPointer(), PixelShaderData, PixelShaderSize);
    
    CmnFree(VertexShaderData);
    CmnFree(PixelShaderData);
}
