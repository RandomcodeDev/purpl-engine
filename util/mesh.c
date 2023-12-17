/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    mesh.c

Abstract:

    This module implements the mesh file API.

--*/

#include "mesh.h"

PMESH
CreateMesh(
    _In_ PCSTR Material,
    _In_reads_(VertexCount * sizeof(VERTEX)) PVERTEX Vertices,
    _In_ SIZE_T VertexCount,
    _In_reads_(IndexCount * sizeof(ivec3)) ivec3* Indices,
    _In_ SIZE_T IndexCount
    )
/*++

Routine Description:

    Creates a mesh from the data given.

Arguments:

    Material - The name of the material to use.

    Vertices - The vertices to make the mesh out of.

    VertexCount - The number of vertices supplied.

    Indices - The indices to make the mesh's triangles out of.

    IndexCount - The number of indices supplied.

Return Value:

    NULL - The mesh could not be created.

    Non-NULL - The mesh was created successfully.

--*/
{
    PMESH Mesh;

    if ( !Material || !Vertices || !VertexCount || (!Indices && IndexCount) )
    {
        LogError("Cannot create invalid mesh");
        return NULL;
    }

    Mesh = PURPL_ALLOC(
        1,
        sizeof(MESH)
        );
    if ( !Mesh )
    {
        LogError("Could not allocate mesh");
        return NULL;
    }

    Mesh->Magic = MESH_MAGIC_NUMBER;
    Mesh->Version = MESH_FORMAT_VERSION;
    strncpy(
        Mesh->Material,
        Material,
        PURPL_ARRAYSIZE(Mesh->Material) - 1
        );
    Mesh->Vertices = Vertices;
    Mesh->VertexCount = VertexCount;
    Mesh->Indices = Indices;
    Mesh->IndexCount = IndexCount;
    Mesh->DataSeparate = TRUE;

    return Mesh;
}

PMESH
LoadMesh(
    _In_ PCSTR Path
    )
/*++

Routine Description:

    Loads a mesh.

Arguments:

    Path - The path to load the mesh from.

Return Value:

    NULL - The mesh could not be loaded.

    Non-NULL - The mesh was loaded successfully.

--*/
{
    PMESH Mesh;
    SIZE_T Size;

    LogInfo("Loading mesh %s", Path);

    Size = 0;
    Mesh = FsReadFile(
        Path,
        0,
        &Size,
        sizeof(MESH) - MESH_HEADER_SIZE
        );
    if ( !ValidateMesh(Mesh) )
    {
        LogError("Mesh %s is invalid", Path);
        return NULL;
    }

    if ( Size < MESH_HEADER_SIZE )
    {
        LogError("Mesh is %zu bytes but should be %zu bytes", Size, MESH_HEADER_SIZE);
        PURPL_FREE(Mesh);
        return NULL;
    }

    memmove(
        (PBYTE)Mesh + sizeof(MESH),
        (PBYTE)Mesh + MESH_HEADER_SIZE,
        Size - sizeof(MESH)
        );
    Mesh->Vertices = (PVERTEX)((PBYTE)Mesh + sizeof(MESH));
    Mesh->Indices = (ivec3*)(Mesh->Vertices + Mesh->VertexCount);
    Mesh->DataSeparate = FALSE;
    return Mesh;
}

BOOLEAN
WriteMesh(
    _In_ PCSTR Path,
    _In_ PMESH Mesh
    )
/*++

Routine Description:

    Writes a mesh to a file.

Arguments:

    Path - The path to write the mesh to.

    Mesh - The mesh to write.

Return Value:

    TRUE - The mesh was written successfully.

    FALSE - The mesh could not be written.

--*/
{
    if ( !Path || !Mesh )
    {
        return FALSE;
    }

    LogInfo("Writing mesh to %s", Path);

    if ( !FsWriteFile(
            Path,
            Mesh,
            MESH_HEADER_SIZE,
            FALSE
            ) )
    {
        LogError("Could not write mesh header to %s", Path);
        return FALSE;
    }

    if ( !FsWriteFile(
            Path,
            Mesh->Vertices,
            Mesh->VertexCount * sizeof(VERTEX),
            TRUE
            ) )
    {
        LogError("Could not write vertex data to %s", Path);
        return FALSE;
    }

    if ( !FsWriteFile(
            Path,
            Mesh->Indices,
            Mesh->IndexCount * sizeof(ivec3),
            TRUE
            ) )
    {
        LogError("Could not write index data to %s", Path);
        return FALSE;
    }

    return TRUE;
}