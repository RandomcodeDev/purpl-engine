/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    filesystem.h

Abstract:

    This module contains definitions for the common filesystem library.

--*/

#pragma once

#include "purpl/purpl.h"

#include "platform/platform.h"

#include "common.h"
#include "log.h"

//
// Get the size of a file
//

extern
SIZE_T
FsGetFileSize(
    _In_ PCSTR Path
    );

//
// Create a directory
//

BOOLEAN
FsCreateDirectory(
    _In_ PCSTR Path
    );

//
// Read a file
//

extern
PVOID
FsReadFile(
    _In_ PCSTR Path,
    _In_ SIZE_T MaxAmount,
    _Out_ PSIZE_T ReadAmount,
    _In_ SIZE_T Extra
    );

//
// Write a file
//

extern
BOOLEAN
FsWriteFile(
    _In_ PCSTR Path,
    _In_reads_bytes_(Size) PVOID Data,
    _In_ SIZE_T Size,
    _In_ BOOLEAN Append
    );
