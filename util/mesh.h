/// @file mesh.h
///
/// @brief This module defines the mesh API.
///
/// @copyright (c) 2024 Randomcode Developers


#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/filesystem.h"
#include "common/log.h"

// Mesh file magic bytes

#define MESH_MAGIC "PMSH"
#define MESH_MAGIC_NUMBER 'HSMP'
#define MESH_MAGIC_LENGTH 4

// Mesh format version
#define MESH_FORMAT_VERSION 5

// Vertex
typedef struct VERTEX
{
    vec3 Position;
    vec4 Colour;
    vec2 TextureCoordinate;
    vec3 Normal;
} VERTEX, *PVERTEX;

//
// Mesh
//

typedef struct MESH
{
    union {
        CHAR MagicBytes[4];
        UINT32 Magic;
    };

    UINT32 Version;

    CHAR Material[32];

    UINT64 VertexCount;
    UINT64 IndexCount;

    // Set to FALSE if you use arrays
    BOOLEAN DataSeparate;
    PVERTEX Vertices;
    ivec3* Indices;
} MESH, *PMESH;

//
// Mesh file header size
//

#define MESH_HEADER_SIZE offsetof(MESH, DataSeparate)

//
// Check a mesh's validity
//

#define ValidateMesh(Mesh) \
    ((Mesh) && \
    memcmp( \
        (Mesh)->MagicBytes, \
        MESH_MAGIC, \
        MESH_MAGIC_LENGTH \
        ) == 0 && \
    (Mesh)->Version == MESH_FORMAT_VERSION)

//
// Create a mesh
//

extern
PMESH
CreateMesh(
    _In_ PCSTR Material,
    _In_reads_(VertexCount * sizeof(VERTEX)) PVERTEX Vertices,
    _In_ SIZE_T VertexCount,
    _In_reads_(IndexCount * sizeof(ivec3)) ivec3* Indices,
    _In_ SIZE_T IndexCount
    );

//
// Load a mesh
//

extern
PMESH
LoadMesh(
    _In_ PCSTR Path
    );

//
// Write a mesh
//

extern
BOOLEAN
WriteMesh(
    _In_ PCSTR Path,
    _In_ PMESH Mesh
    );
