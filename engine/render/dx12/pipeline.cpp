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
    ID3DBlob *VertexShader;
    ID3DBlob *PixelShader;

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

    VertexShader = Dx12LoadShader("assets/shaders/directx12/shaders.vs.cso");
    PixelShader = Dx12LoadShader("assets/shaders/directx12/shaders.ps.cso");

    D3D12_GRAPHICS_PIPELINE_STATE_DESC PsoDescription = {};
    PsoDescription.InputLayout.pInputElementDescs = InputElementDescriptions;
    PsoDescription.InputLayout.NumElements = PURPL_ARRAYSIZE(InputElementDescriptions);
    PsoDescription.pRootSignature = Dx12Data.RootSignature;
    PsoDescription.VS.pShaderBytecode = (PBYTE)VertexShader->GetBufferPointer();
    PsoDescription.VS.BytecodeLength = VertexShader->GetBufferSize();
    PsoDescription.PS.pShaderBytecode = (PBYTE)PixelShader->GetBufferPointer();
    PsoDescription.PS.BytecodeLength = PixelShader->GetBufferSize();
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
