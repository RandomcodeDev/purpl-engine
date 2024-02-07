#include "dx12.h"

EXTERN_C
VOID Dx12CreateRootSignature(VOID)
{
    ID3DBlob *Signature;
    ID3DBlob *Error;

    LogDebug("Creating root signature");

    // This is required to use a versioned root signature
    D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};
    FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
    if (FAILED(Dx12Data.Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData))))
    {
        FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    CD3DX12_DESCRIPTOR_RANGE1 DescriptorRanges[3];
    DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    DescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    
    CD3DX12_ROOT_PARAMETER1 RootParameters[2];
    RootParameters[0].InitAsDescriptorTable(2, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_VERTEX);
    RootParameters[1].InitAsDescriptorTable(1, &DescriptorRanges[2], D3D12_SHADER_VISIBILITY_PIXEL);

    D3D12_STATIC_SAMPLER_DESC Sampler = {};
    Sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
    Sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    Sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    Sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
    Sampler.MipLODBias = 0;
    Sampler.MaxAnisotropy = 0;
    Sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    Sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
    Sampler.MinLOD = 0.0;
    Sampler.MaxLOD = D3D12_FLOAT32_MAX;
    Sampler.ShaderRegister = 0;
    Sampler.RegisterSpace = 0;
    Sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDescription;
    RootSignatureDescription.Init_1_1(PURPL_ARRAYSIZE(RootParameters), RootParameters, 1, &Sampler,
                                      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    HRESULT_CHECK(D3DX12SerializeVersionedRootSignature(&RootSignatureDescription, FeatureData.HighestVersion,
                                                        &Signature, &Error));
    HRESULT_CHECK(Dx12Data.Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(),
                                                       IID_PPV_ARGS(&Dx12Data.RootSignature)));
}
