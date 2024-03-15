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

    CD3DX12_DESCRIPTOR_RANGE1 DescriptorRanges[2];
    DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
    DescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

    CD3DX12_ROOT_PARAMETER1 RootParameters[2];
    RootParameters[0].InitAsDescriptorTable(1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_VERTEX);
    RootParameters[1].InitAsDescriptorTable(1, &DescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);

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

EXTERN_C
PVOID Dx12LoadShader(_In_ PCSTR Name)
{
    PBYTE VertexShader;
    UINT64 VertexShaderSize;
    PBYTE PixelShader;
    UINT64 PixelShaderSize;

    LogDebug("Creating pipeline state object for shader %s", Name);

    D3D12_INPUT_ELEMENT_DESC InputElementDescriptions[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
         D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

    VertexShaderSize = 0;
    VertexShader = (PBYTE)FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "directx12/%s.vs.cso", Name), 0,
                                     0, &VertexShaderSize, 0);
    PixelShaderSize = 0;
    PixelShader = (PBYTE)FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "directx12/%s.ps.cso", Name), 0, 0,
                                    &PixelShaderSize, 0);

    if (!VertexShaderSize || !PixelShaderSize)
    {
        LogError("DirectX 12 shader for %s not found", Name);
        return NULL;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDescription = {};
    PsoDescription.InputLayout.pInputElementDescs = InputElementDescriptions;
    PsoDescription.InputLayout.NumElements = PURPL_ARRAYSIZE(InputElementDescriptions);
    PsoDescription.pRootSignature = Dx12Data.RootSignature;
    PsoDescription.VS.pShaderBytecode = VertexShader;
    PsoDescription.VS.BytecodeLength = VertexShaderSize;
    PsoDescription.PS.pShaderBytecode = PixelShader;
    PsoDescription.PS.BytecodeLength = PixelShaderSize;
    PsoDescription.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    PsoDescription.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    // TODO: figure out how to do depth
    PsoDescription.DepthStencilState.DepthEnable = FALSE;
    PsoDescription.DepthStencilState.StencilEnable = FALSE;
    PsoDescription.SampleMask = UINT32_MAX;
    PsoDescription.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    PsoDescription.NumRenderTargets = 1;
    PsoDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    PsoDescription.SampleDesc.Count = 1;

    ID3D12PipelineState *PipelineState;
    HRESULT Result = Dx12Data.Device->CreateGraphicsPipelineState(&PsoDescription, IID_PPV_ARGS(&PipelineState));
    if (!SUCCEEDED(Result))
    {
        LogError("Failed to create pipeline state object for shader %s: HRESULT 0x%08X", Name, Result);
        return NULL;
    }
    Dx12NameObject(PipelineState, "Pipeline state object for shader %s", Name);

    return PipelineState;
}

EXTERN_C
VOID Dx12DestroyShader(_In_ PVOID Shader)
{
    if (Shader)
    {
        ID3D12PipelineState *PipelineState = (ID3D12PipelineState *)Shader;
        PipelineState->Release();
    }
}

EXTERN_C
VOID Dx12CreateUniformBuffer(VOID)
{
    static CONST SIZE_T Size = PURPL_ALIGN(D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT,
                                           sizeof(RENDER_SCENE_UNIFORM) + sizeof(RENDER_OBJECT_UNIFORM));
    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC BufferDescription = CD3DX12_RESOURCE_DESC::Buffer(Size * DIRECTX12_FRAME_COUNT);
    Dx12CreateBuffer(&Dx12Data.UniformBuffer, &HeapProperties, D3D12_HEAP_FLAG_NONE, &BufferDescription,
                     D3D12_RESOURCE_STATE_GENERIC_READ);
    Dx12NameObject(Dx12Data.UniformBuffer.Resource, "Uniform buffer");

    D3D12_CONSTANT_BUFFER_VIEW_DESC UniformViewDescription = {};
    UniformViewDescription.BufferLocation = Dx12Data.UniformBuffer.Resource->GetGPUVirtualAddress();
    UniformViewDescription.SizeInBytes = Size;

    CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle(Dx12Data.ShaderHeap->GetCPUDescriptorHandleForHeapStart(), 1,
                                            Dx12Data.ShaderDescriptorSize);

    for (UINT32 i = 0; i < DIRECTX12_FRAME_COUNT; i++)
    {
        Dx12Data.Device->CreateConstantBufferView(&UniformViewDescription, CpuHandle);

        UniformViewDescription.BufferLocation += Size;
        CpuHandle.Offset(Dx12Data.ShaderDescriptorSize);
    }

    CD3DX12_RANGE Range(0, 0);
    Dx12Data.UniformBuffer.Resource->Map(0, &Range, (PVOID *)&Dx12Data.UniformBufferAddress);
}
