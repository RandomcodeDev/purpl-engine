#include "opengl.h"

VOID GlCreateModel(_In_z_ PCSTR Name, _Inout_ PMODEL Model, _In_ PMESH Mesh)
{
    POPENGL_MODEL_DATA ModelData = CmnAllocType(1, OPENGL_MODEL_DATA);
    if (!ModelData)
    {
        CmnError("Failed to allocate memory for model data: %s", strerror(errno));
    }

    glGenVertexArrays(1, &ModelData->VertexArray);
    glBindVertexArray(ModelData->VertexArray);

    glGenBuffers(1, &ModelData->VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, ModelData->VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, Mesh->VertexCount * sizeof(VERTEX), Mesh->Vertices, GL_STATIC_DRAW);
    glObjectLabel(GL_BUFFER, ModelData->VertexBuffer, 13, "Vertex buffer");

    ModelData->ElementCount = (UINT32)Mesh->IndexCount;

    glGenBuffers(1, &ModelData->IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ModelData->IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ModelData->ElementCount * sizeof(ivec3), Mesh->Indices, GL_STATIC_DRAW);
    glObjectLabel(GL_BUFFER, ModelData->IndexBuffer, 12, "Index buffer");

    glBindTexture(GL_TEXTURE_2D, (UINT32)Model->Material->TextureHandle);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (PVOID)offsetof(VERTEX, Position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (PVOID)offsetof(VERTEX, Colour));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (PVOID)offsetof(VERTEX, TextureCoordinate));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), (PVOID)offsetof(VERTEX, Normal));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);

    Model->MeshHandle = (RENDER_HANDLE)ModelData;
}

VOID GlDrawModel(_In_ PMODEL Model, _In_ PRENDER_OBJECT_UNIFORM Uniform, _In_ PRENDER_OBJECT_DATA Data)
{
    UNREFERENCED_PARAMETER(Data);

    POPENGL_MODEL_DATA ModelData = (POPENGL_MODEL_DATA)Model->MeshHandle;

    GlWriteUniformBuffer(GlData.UniformBuffer, PURPL_ALIGN(GlData.UniformBufferAlignment, sizeof(RENDER_SCENE_UNIFORM)),
                         Uniform, sizeof(RENDER_OBJECT_UNIFORM));

    glBindBufferRange(GL_UNIFORM_BUFFER, RENDER_SHADER_SCENE_UBO_REGISTER, GlData.UniformBuffer, 0, sizeof(RENDER_SCENE_UNIFORM));
    glBindBufferRange(GL_UNIFORM_BUFFER, RENDER_SHADER_OBJECT_UBO_REGISTER, GlData.UniformBuffer,
                      PURPL_ALIGN(GlData.UniformBufferAlignment, sizeof(RENDER_SCENE_UNIFORM)),
                      sizeof(RENDER_OBJECT_UNIFORM));

    glUseProgram((UINT32)Model->Material->ShaderHandle);

    glBindVertexArray(ModelData->VertexArray);
    glDrawElements(GL_TRIANGLES, ModelData->ElementCount * 3, GL_UNSIGNED_INT, NULL);
    glBindVertexArray(0);
}

VOID GlDestroyModel(_Inout_ PMODEL Model)
{
    POPENGL_MODEL_DATA ModelData = (POPENGL_MODEL_DATA)Model->MeshHandle;
    if (!ModelData)
    {
        return;
    }
    glDeleteVertexArrays(1, &ModelData->VertexArray);
    glDeleteBuffers(1, &ModelData->IndexBuffer);
    glDeleteBuffers(1, &ModelData->VertexBuffer);
    CmnFree(Model->MeshHandle);
}
