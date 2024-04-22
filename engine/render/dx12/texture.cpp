#include "dx12.h"

RENDER_HANDLE Dx12UseTexture(_In_ PTEXTURE Texture)
{
    PDIRECTX12_TEXTURE TextureData = CmnAllocType(1, DIRECTX12_TEXTURE);
    if (!TextureData)
    {
        CmnError("Failed to allocate DirectX 12 texture data: %s", strerror(errno));
    }

    CD3DX12_HEAP_PROPERTIES HeapProperties(D3D12_HEAP_TYPE_DEFAULT);

    DXGI_FORMAT Format;
    switch (Texture->Format)
    {
    case TextureFormatDepth:
        Format = DXGI_FORMAT_D32_FLOAT;
        break;
    case TextureFormatRgba8:
    default:
        Format = DXGI_FORMAT_R8G8B8A8_UINT;
        break;
    }

    CD3DX12_RESOURCE_DESC ResourceDescription = CD3DX12_RESOURCE_DESC::Tex2D(Format, Texture->Width, Texture->Height);
    Dx12CreateBufferWithData(&TextureData->Buffer, Texture->Pixels, &HeapProperties, D3D12_HEAP_FLAG_NONE,
                             &ResourceDescription, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

    D3D12_SHADER_RESOURCE_VIEW_DESC SrvDescription = {};
    SrvDescription.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    SrvDescription.Format = Format;
    SrvDescription.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SrvDescription.Texture2D.MipLevels = 1;
    Dx12Data.Device->CreateShaderResourceView(TextureData->Buffer.Resource, &SrvDescription,
                                              DIRECTX12_GET_DESCRIPTOR_HANDLE_FOR_HEAP_START(Dx12Data.SrvHeap, CPU));

    return (RENDER_HANDLE)TextureData;
}

VOID Dx12ReleaseTexture(_In_ RENDER_HANDLE Handle)
{
    PDIRECTX12_TEXTURE TextureData = (PDIRECTX12_TEXTURE)Handle;
    TextureData->Buffer.Resource->Release();
    CmnFree(TextureData);
}
