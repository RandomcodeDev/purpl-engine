#include "dx12.h"

VOID Dx12DrawGeometry(_In_ PCMESH_VERTEX Vertices, _In_ SIZE_T VertexCount, _In_opt_ CONST ivec3 *CONST Indices,
                      _In_ SIZE_T IndexCount, _In_ RENDER_HANDLE Shader, _In_opt_ mat4 CONST Transform,
                      _In_ BOOLEAN Project)
{
    UINT64 DrawVertexCount = PURPL_MIN(VertexCount, RENDER_MAX_GEOMETRY_SIZE);

    memcpy(Dx12Data.GeometryVertexBufferAddress, Vertices, DrawVertexCount * sizeof(MESH_VERTEX));

    D3D12_VERTEX_BUFFER_VIEW VertexBufferView = {};
    VertexBufferView.BufferLocation = Dx12Data.GeometryVertexBuffer.Resource->GetGPUVirtualAddress();
    VertexBufferView.SizeInBytes = DrawVertexCount * sizeof(MESH_VERTEX);
    VertexBufferView.StrideInBytes = sizeof(MESH_VERTEX);
    Dx12Data.CommandList->IASetVertexBuffers(0, 1, &VertexBufferView);

    if (Indices && IndexCount)
    {
        UINT64 DrawIndexCount = PURPL_MIN(IndexCount, RENDER_MAX_GEOMETRY_SIZE / 3);

        memcpy(Dx12Data.GeometryIndexBufferAddress, Indices, DrawIndexCount * sizeof(ivec3));

        D3D12_INDEX_BUFFER_VIEW IndexBufferView = {};
        IndexBufferView.BufferLocation = Dx12Data.GeometryIndexBuffer.Resource->GetGPUVirtualAddress();
        IndexBufferView.SizeInBytes = DrawIndexCount * sizeof(ivec3);
        IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
        Dx12Data.CommandList->IASetIndexBuffer(&IndexBufferView);

        Dx12Data.CommandList->DrawIndexedInstanced(DrawIndexCount, 1, 0, 0, 0);
    }
    else
    {
        Dx12Data.CommandList->DrawInstanced(VertexCount, 1, 0, 0);
    }
}
