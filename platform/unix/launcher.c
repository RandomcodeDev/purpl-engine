/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    launcher.c

Abstract:

    This module implements the entrypoint for Unix-like operating systems.

--*/

#include "purpl/purpl.h"

#include "common/common.h"

INT
main(
    INT argc,
    PCHAR* argv   
    )
/*++

Routine:

    This routine is the entrypoint for Unix-like systems.

Arguments:

    argc - The number of arguments.

    argv - Array of command line arguments.

Return Value:

    An appropriate errno code.

--*/
{
    INT Result;

    Result = PurplMain(
        argc,
        argv
        );

    return Result;
}
