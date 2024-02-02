/// @file filesystem.h
///
/// @brief This module contains definitions for the common filesystem library.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "platform/platform.h"

#include "alloc.h"
#include "common.h"
#include "log.h"

/// @brief Gets the size of a file
/// @param Path The path to the file
/// @return The size of the file in bytes
extern SIZE_T FsGetFileSize(_In_ PCSTR Path);

/// @brief Creates a directory
/// @param Path The path to the directory
/// @return Whether the creation succeeded
BOOLEAN
FsCreateDirectory(_In_ PCSTR Path);

/// @brief This routine reads a file into a buffer which it allocates.
///
/// @param Path       The path to the file to read.
/// @param MaxAmount  The maximum number of bytes to read, or zero for the whole
/// file.
/// @param ReadAmount A pointer to a variable that receives the number of bytes
/// read
///                   from the file.
/// @param Extra      Number of extra bytes to allocate.
///
/// @return A pointer to a buffer containing the file's contents, or NULL.
extern PVOID FsReadFile(_In_ PCSTR Path, _In_ SIZE_T MaxAmount,
                        _Out_ PSIZE_T ReadAmount, _In_ SIZE_T Extra);

/// @brief Write to a file
/// @param Path The path to the file
/// @param Data The data to write to the file
/// @param Size The size of the data
/// @param Append Whether to append to the file or overwrite it (either way, it will be created)
/// @return Whether the write succeded
extern BOOLEAN FsWriteFile(_In_ PCSTR Path, _In_reads_bytes_(Size) PVOID Data,
                           _In_ SIZE_T Size, _In_ BOOLEAN Append);
