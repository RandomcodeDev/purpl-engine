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

    CD3DX12_DESCRIPTOR_RANGE1 DescriptorRanges[1];
    DescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0,
                             D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE);

    CD3DX12_ROOT_PARAMETER1 RootParameters[3];
    RootParameters[0].InitAsConstantBufferView(0, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                                               D3D12_SHADER_VISIBILITY_VERTEX);
    RootParameters[1].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC,
                                               D3D12_SHADER_VISIBILITY_VERTEX);
    RootParameters[2].InitAsDescriptorTable(1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);

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

static CONST D3D12_INPUT_ELEMENT_DESC MeshInputElementDescriptions[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VERTEX, Position),
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(VERTEX, Colour),
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(VERTEX, TextureCoordinate),
     D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, offsetof(VERTEX, Normal), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
     0}};

EXTERN_C
RENDER_HANDLE Dx12LoadShader(_In_z_ PCSTR Name)
{
    PBYTE VertexShader;
    UINT64 VertexShaderSize;
    PBYTE PixelShader;
    UINT64 PixelShaderSize;

    LogDebug("Creating pipeline state object for shader %s", Name);

    VertexShaderSize = 0;
    VertexShader = (PBYTE)FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "directx12/%s.vs.cso", Name), 0,
                                     0, &VertexShaderSize, 0);
    PixelShaderSize = 0;
    PixelShader = (PBYTE)FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "directx12/%s.ps.cso", Name), 0, 0,
                                    &PixelShaderSize, 0);

    if (!VertexShaderSize || !PixelShaderSize)
    {
        CmnError("DirectX 12 shader for %s not found", Name);
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDescription = {};
    PsoDescription.InputLayout.pInputElementDescs = MeshInputElementDescriptions;
    PsoDescription.InputLayout.NumElements = PURPL_ARRAYSIZE(MeshInputElementDescriptions);
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
    PsoDescription.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    PsoDescription.SampleDesc.Count = 1;

    D3D12_DEPTH_STENCIL_DESC DepthStencilDescription = {};
    DepthStencilDescription.DepthEnable = TRUE;
    DepthStencilDescription.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    DepthStencilDescription.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    DepthStencilDescription.StencilEnable = FALSE;

    PsoDescription.DepthStencilState = DepthStencilDescription;

    ID3D12PipelineState *PipelineState;
    HRESULT_CHECK(Dx12Data.Device->CreateGraphicsPipelineState(&PsoDescription, IID_PPV_ARGS(&PipelineState)));
    Dx12NameObject(PipelineState, "Pipeline state object for shader %s", Name);

    return (RENDER_HANDLE)PipelineState;
}

EXTERN_C
VOID Dx12DestroyShader(_In_ RENDER_HANDLE Shader)
{
    if (Shader)
    {
        ID3D12PipelineState *PipelineState = (ID3D12PipelineState *)Shader;
        PipelineState->Release();
    }
}

EXTERN_C
VOID Dx12CreateUniformBuffer(_Out_ PDIRECTX12_BUFFER UniformBuffer, _Out_ PVOID *Address, _In_ UINT64 Size)
{
    static UINT64 LastUniformBufferIndex = 0;

    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_UPLOAD);
    CD3DX12_RESOURCE_DESC BufferDescription = CD3DX12_RESOURCE_DESC::Buffer(Size * DIRECTX12_FRAME_COUNT);
    Dx12CreateBuffer(UniformBuffer, &HeapProperties, D3D12_HEAP_FLAG_NONE, &BufferDescription,
                     D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

    D3D12_CONSTANT_BUFFER_VIEW_DESC UniformViewDescription = {};
    UniformViewDescription.BufferLocation = UniformBuffer->Resource->GetGPUVirtualAddress();
    UniformViewDescription.SizeInBytes = Size;

    CD3DX12_CPU_DESCRIPTOR_HANDLE CpuHandle(DIRECTX12_GET_DESCRIPTOR_HANDLE_FOR_HEAP_START(Dx12Data.ShaderHeap, CPU), 1,
                                            Dx12Data.ShaderDescriptorSize);

    for (UINT32 i = 0; i < DIRECTX12_FRAME_COUNT; i++)
    {
        Dx12Data.Device->CreateConstantBufferView(&UniformViewDescription, CpuHandle);

        UniformViewDescription.BufferLocation += Size;
        CpuHandle.Offset(Dx12Data.ShaderDescriptorSize);
    }

    CD3DX12_RANGE Range(0, 0);
    HRESULT_CHECK(UniformBuffer->Resource->Map(0, &Range, Address));
}
