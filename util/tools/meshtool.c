/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    texturetool.c

Abstract:

    This file implements a rudimentary tool for working with meshes.

--*/

#include "purpl/purpl.h"

#include "common/common.h"

#include "util/mesh.h"

#include "assimp/cimport.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

_Noreturn
VOID
Usage(
    VOID
    )
/*++

Routine Description:

    Prints instructions for using the program and exits.

Arguments:

    None.

Return Value:

    Does not return.

--*/
{
    printf("Usage:\n"
           "\n"
           "\tto <input model> <output Purpl mesh>\t\t\t- Create a mesh\n"
           "\tfrom <input Purpl mesh> <output OBJ mesh>\t\t- Convert a mesh to an OBJ model\n"
           );
    exit(EINVAL);
}

VOID
ProcessNode(
    _In_ const struct aiNode* Node,
    _In_ const struct aiScene* Scene,
    _Out_ struct aiMesh** Meshes,
    _Inout_ PUINT32 CurrentMesh
    )
/*++

Routine Description:

    Recursively processes the meshes of the nodes in a scene.

Arguments:

    Node - The node to process.

    Scene - The scene to use meshes from.

    Meshes - The array to store processed meshes in.

Return Value:

    None.

--*/
{
    for ( UINT32 i = 0; i < Node->mNumMeshes; i++ )
    {
        Meshes[*CurrentMesh] = Scene->mMeshes[Node->mMeshes[i]];
        (*CurrentMesh)++;
    }

    for ( UINT32 i = 0; i < Node->mNumChildren; i++ )
    {
        ProcessNode(
            Node->mChildren[i],
            Scene,
            Meshes,
            CurrentMesh
            );
    }
}

PCHAR
GetMaterialName(
    _In_ const struct aiScene* Scene,
    _In_ struct aiMesh* Mesh
    )
{
    struct aiMaterial* Material;
    struct aiString Name;

    Material = Scene->mMaterials[Mesh->mMaterialIndex];
    aiGetMaterialString(
        Material,
        AI_MATKEY_NAME,
        &Name
        );

    return CmnFormatString(
        "%s",
        Name.data
        );
}

PMESH
ConvertMesh(
    _In_ const struct aiScene* Scene,
    _In_ struct aiMesh* Mesh
    )
/*++

Routine Description:

    Convert a mesh into the engine format.

Arguments:

    Scene - The scene the mesh is from.

    Mesh - The mesh to convert.

Return Value:

    NULL or the converted mesh.

--*/
{
    PVERTEX Vertices;
    ivec3* Indices;
    PMESH OutMesh;
    SIZE_T i;
    PCHAR MaterialName;

    MaterialName = GetMaterialName(
        Scene,
        Mesh
        );

    printf("Converting mesh %s with %u vertices and %u faces using material %s\n", Mesh->mName.data, Mesh->mNumVertices, Mesh->mNumFaces, MaterialName);

    Indices = NULL;

    Vertices = CmnAlloc(
        Mesh->mNumVertices,
        sizeof(VERTEX)
        );
    if ( !Vertices )
    {
        fprintf(stderr, "Failed to allocate %u vertices: %s\n", Mesh->mNumVertices, strerror(errno));
        goto Error;
    }

    for ( i = 0; i < Mesh->mNumVertices; i++ )
    {
//        printf("%f %f %f\n", Mesh->mVertices[i].x, Mesh->mVertices[i].y, Mesh->mVertices[i].z);

        Vertices[i].Position[0] = Mesh->mVertices[i].x;
        Vertices[i].Position[1] = Mesh->mVertices[i].y;
        Vertices[i].Position[2] = Mesh->mVertices[i].z;

        if ( Mesh->mColors[0] )
        {
            Vertices[i].Colour[0] = Mesh->mColors[0][i].r;
            Vertices[i].Colour[1] = Mesh->mColors[0][i].g;
            Vertices[i].Colour[2] = Mesh->mColors[0][i].b;
            Vertices[i].Colour[3] = Mesh->mColors[0][i].a;
        }
        else
        {
            Vertices[i].Colour[0] = 1.0f;
            Vertices[i].Colour[1] = 1.0f;
            Vertices[i].Colour[2] = 1.0f;
            Vertices[i].Colour[3] = 1.0f;
        }

        if ( Mesh->mNormals )
        {
            Vertices[i].Normal[0] = Mesh->mNormals[i].x;
            Vertices[i].Normal[1] = Mesh->mNormals[i].y;
            Vertices[i].Normal[2] = Mesh->mNormals[i].z;
        }

        if ( Mesh->mTextureCoords[0] )
        {
            Vertices[i].TextureCoordinate[0] = Mesh->mTextureCoords[0][i].x;
            Vertices[i].TextureCoordinate[1] = Mesh->mTextureCoords[0][i].y;
        }
    }

    Indices = CmnAlloc(
        Mesh->mNumFaces,
        sizeof(ivec3)
        );
    if ( !Indices )
    {
        fprintf(stderr, "Failed to allocate %u indices: %s\n", Mesh->mNumFaces, strerror(errno));
        goto Error;
    }

    for ( i = 0; i < Mesh->mNumFaces; i++ )
    {
        // Always 3 because of triangulate
        Indices[i][0] = Mesh->mFaces[i].mIndices[0];
        Indices[i][1] = Mesh->mFaces[i].mIndices[1];
        Indices[i][2] = Mesh->mFaces[i].mIndices[2];
    }

    OutMesh = CreateMesh(
        MaterialName,
        Vertices,
        Mesh->mNumVertices,
        Indices,
        Mesh->mNumFaces
        );
    if ( !OutMesh )
    {
        fprintf(stderr, "Failed to create mesh\n");
        goto Error;
    }

    return OutMesh;
Error:
    if ( MaterialName )
    {
        CmnFree(MaterialName);
    }
    if ( Vertices )
    {
        CmnFree(Vertices);
    }
    if ( Indices )
    {
        CmnFree(Indices);
    }
    return NULL;
}

INT
ConvertTo(
    _In_ PCSTR Source,
    _In_ PCSTR Destination
    )
/*++

Routine Description:

    Converts 3D models to Purpl meshes.

Arguments:

    Source - The mesh to convert.

    Destination - The Purpl mesh to write to.

Return Value:

    0 on success or an appropriate errno code.

--*/
{
    const struct aiScene* Scene;
    struct aiMesh** Meshes;
    UINT32 CurrentMesh;
    UINT32 i;
    PMESH Mesh;
    PCHAR BaseName;
    PCHAR Extension;
    PCSTR OutputName;

    printf("Converting model %s to Purpl mesh %s\n", Source, Destination);

    BaseName = CmnFormatString("%s", Destination);
    Extension = NULL;
    if ( strrchr(
             BaseName,
             '.'
             ) )
    {
        Extension = strrchr(
            BaseName,
            '.'
            ) + 1;
        if ( *Extension != 0 )
        {
            Extension[-1] = 0;
        }
    }

    Scene = aiImportFile(
        Source,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals |
            aiProcess_OptimizeMeshes | aiProcess_ConvertToLeftHanded
        );
    if ( !Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
         !Scene->mRootNode )
    {
        fprintf(stderr, "Failed to load mesh %s: %s\n", Source,
                aiGetErrorString());
        return errno;
    }

    Meshes = CmnAlloc(
        Scene->mNumMeshes,
        sizeof(struct aiMesh*)
        );
    if ( !Meshes )
    {
        fprintf(stderr, "Failed to allocate array of %u meshes: %s\n", Scene->mNumMeshes, strerror(errno));
        return errno;
    }

    CurrentMesh = 0;
    ProcessNode(
        Scene->mRootNode,
        Scene,
        Meshes,
        &CurrentMesh
        );

    for ( i = 0; i < 1; i++ ) //Scene->mNumMeshes; i++ )
    {
        OutputName = CmnFormatTempString("%s%s%s", BaseName, Extension ? "." : "", Extension);
        Mesh = ConvertMesh(
            Scene,
            Meshes[i]
            );
        printf("Writing mesh %s\n", OutputName);
        if ( !WriteMesh(
             OutputName,
             Mesh
             ) )
        {
            fprintf(stderr, "Failed to write mesh %s\n", OutputName);
            return errno;
        }
    }

    CmnFree(BaseName);

    return 0;
}

INT
ConvertFrom(
    _In_ PCSTR Source,
    _In_ PCSTR Destination
    )
/*++

Routine Description:

    Converts Purpl meshes to OBJ.

Arguments:

    Source - The Purpl mesh to convert.

    Destination - The OBJ file to write to.

Return Value:

    0 on success or an appropriate errno code.

--*/
{
    UNREFERENCED_PARAMETER(Source);
    UNREFERENCED_PARAMETER(Destination);

    return 0;
}

//
// Tool mode
//

typedef enum MESHTOOL_MODE
{
    MeshToolModeNone,
    MeshToolModeConvertTo,
    MeshToolModeConvertFrom,
    MeshToolModeCount
} MESHTOOL_MODE, *PMESHTOOL_MODE;

//
// Functions for each mode
//

typedef INT
(*PFNMESHTOOL_OPERATION)(
    _In_ PCSTR Source,
    _In_ PCSTR Destination
    );
PFNMESHTOOL_OPERATION Operations[MeshToolModeCount] = {
    (PFNMESHTOOL_OPERATION)Usage,
    ConvertTo,
    ConvertFrom,
};

INT
main(
    INT argc,
    PCHAR* argv
    )
/*++

Routine Description:

    Processes arguments and calls the requested function.

Arguments:

    argc - Number of arguments.

    argv - Array of arguments.

Return Value:

    EXIT_SUCCESS - Success.

    errno value - Failure.

--*/
{
    MESHTOOL_MODE Mode;
    INT Result;

    printf("Purpl Mesh Tool v" PURPL_VERSION_STRING " (supports mesh format v" PURPL_STRINGIZE_EXPAND(MESH_FORMAT_VERSION) ") on %s\n\n", PlatGetDescription());

    CmnInitialize();

    if ( argc < 4 )
    {
        Mode = MeshToolModeNone;
    }
    else if ( strcmp(
                argv[1],
                "to"
                ) == 0 && argc > 3 )
    {
        Mode = MeshToolModeConvertTo;
    }
    else if ( strcmp(
                 argv[1],
                 "from"
                 ) == 0 && argc > 3 )
    {
        Mode = MeshToolModeConvertFrom;
    }
    else
    {
        Mode = MeshToolModeNone;
    }

    Result = Operations[Mode](
        argv[2],
        argv[3]
        );

    CmnShutdown();

    return Result;
}
