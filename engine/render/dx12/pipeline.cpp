#include "dx12.h"

EXTERN_C
BOOLEAN Dx12HavePipelineStateCache(VOID)
{
    return FALSE;
}

EXTERN_C
VOID Dx12LoadPipelineStateCache(VOID)
{
}

EXTERN_C
VOID Dx12CreatePipelineStateObject(VOID)
{
    PBYTE VertexShader;
    UINT64 VertexShaderSize;
    PBYTE PixelShader;
    UINT64 PixelShaderSize;

    LogDebug("Creating pipeline state object");

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
    VertexShader = (PBYTE)FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "directx12/main.vs.cso"), 0, 0, &VertexShaderSize, 0);
    PixelShaderSize = 0;
    PixelShader = (PBYTE)FsReadFile(FALSE, EngGetAssetPath(EngAssetDirectoryShaders, "directx12/main.ps.cso"), 0, 0,
                                    &PixelShaderSize, 0);

    if (!VertexShaderSize || !PixelShaderSize)
    {
        CmnError("DirectX 12 shaders could not be loaded");
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
    PsoDescription.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    PsoDescription.SampleDesc.Count = 1;

    HRESULT_CHECK(Dx12Data.Device->CreateGraphicsPipelineState(&PsoDescription, IID_PPV_ARGS(&Dx12Data.PipelineState)));
}

EXTERN_C
VOID Dx12CachePipelineState(VOID)
{
}
