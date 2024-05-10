#include "swrast.h"

VOID SwrsDrawModel(_In_ PMODEL Model, _In_ PRENDER_OBJECT_UNIFORM Uniform, _In_ PRENDER_OBJECT_DATA Data)
{
    PMESH Mesh = (PMESH)Model->MeshHandle;

    PVERTEX Projected = NULL;
    for (UINT32 i = 0; i < Mesh->VertexCount; i++)
    {
        VERTEX Vertex = Mesh->Vertices[i];
        SwrsProjectVertex(&Vertex, Uniform->Model);
        stbds_arrpush(Projected, Vertex);
    }

    for (UINT32 i = 0; i < Mesh->IndexCount; i++)
    {
        ivec3 Triangle;
        glm_ivec3_copy(Mesh->Indices[i], Triangle);
        if (Triangle[0] < stbds_arrlenu(Projected) && Triangle[1] < stbds_arrlenu(Projected) &&
            Triangle[2] < stbds_arrlenu(Projected))
        {
            SwrsDrawTriangle(Projected[Triangle[0]], Projected[Triangle[1]], Projected[Triangle[2]], FALSE);
        }
    }
}
