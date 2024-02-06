/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    texturetool.c

Abstract:

    This file implements a rudimentary tool for working with meshes.

--*/

#include "purpl/purpl.h"

#include "common/alloc.h"
#include "common/common.h"

#include "util/mesh.h"

#include "ufbx/ufbx.h"

_Noreturn VOID Usage(VOID)
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
           "\tfrom <input Purpl mesh> <output OBJ mesh>\t\t- Convert a mesh to "
           "an OBJ model\n");
    exit(EINVAL);
}

INT ConvertTo(_In_ PCSTR Source, _In_ PCSTR Destination)
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
    return 0;
}

INT ConvertFrom(_In_ PCSTR Source, _In_ PCSTR Destination)
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

typedef INT (*PFNMESHTOOL_OPERATION)(_In_ PCSTR Source, _In_ PCSTR Destination);
PFNMESHTOOL_OPERATION Operations[MeshToolModeCount] = {
    (PFNMESHTOOL_OPERATION)Usage,
    ConvertTo,
    ConvertFrom,
};

INT main(INT argc, PCHAR *argv)
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

    printf("Purpl Mesh Tool v" GAME_VERSION_STRING
           " (supports mesh format v" PURPL_STRINGIZE_EXPAND(
               MESH_FORMAT_VERSION) ") on %s\n\n",
           PlatGetDescription());

    CmnInitialize();

    if (argc < 4)
    {
        Mode = MeshToolModeNone;
    }
    else if (strcmp(argv[1], "to") == 0 && argc > 3)
    {
        Mode = MeshToolModeConvertTo;
    }
    else if (strcmp(argv[1], "from") == 0 && argc > 3)
    {
        Mode = MeshToolModeConvertFrom;
    }
    else
    {
        Mode = MeshToolModeNone;
    }

    Result = Operations[Mode](argv[2], argv[3]);

    CmnShutdown();

    return Result;
}
