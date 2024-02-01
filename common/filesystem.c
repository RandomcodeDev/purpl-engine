/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    filesystem.c

Abstract:

    This module implements the common filesystem library.

--*/

#include "common/filesystem.h"

SIZE_T
FsGetFileSize(_In_ PCSTR Path)
{
    FILE *File;
    SIZE_T Size;
    PCSTR FixedPath;

    FixedPath = PlatFixPath(Path);
    LogTrace("Getting size of file %s (%s)", Path, FixedPath);
    File = fopen(FixedPath, "rb");
    if (!File)
    {
        LogWarning("Failed to open file %s (%s): %s", Path, FixedPath,
                   strerror(errno));
        return 0;
    }

    fseek(File, 0, SEEK_END);
    Size = ftell(File);

    CmnFree(FixedPath);
    fclose(File);
    return Size;
}

BOOLEAN
FsCreateDirectory(_In_ PCSTR Path)
{
    LogTrace("Creating directory %s", Path);
    return PlatCreateDirectory(Path);
}

PVOID
FsReadFile(_In_ PCSTR Path, _In_ SIZE_T MaxAmount, _Out_ PSIZE_T ReadAmount,
           _In_ SIZE_T Extra)
/*++

Routine Description:

    This routine reads a file into a buffer which it allocates.

Arguments:

    Path - The path to the file to read.

    MaxAmount - The maximum number of bytes to read, or zero for the whole file.

    ReadAmount - A pointer to a variable that recieves the number of bytes read
from the file.

    Extra - Number of extra bytes to allocate.

Return Value:

    A pointer to a buffer containing the file's contents, or NULL.

--*/
{
    FILE *File;
    PVOID Buffer;
    SIZE_T Size;
    SIZE_T Read;
    PCSTR FixedPath;

    if (!ReadAmount)
    {
        return NULL;
    }

    FixedPath = PlatFixPath(Path);
    LogTrace("Reading up to %zu byte(s) (+%zu) of file %s (%s)", MaxAmount,
             Extra, Path, FixedPath);
    File = fopen(FixedPath, "rb");
    if (!File)
    {
        LogWarning("Failed to open file %s (%s): %s", Path, FixedPath,
                   strerror(errno));
        *ReadAmount = 0;
        return NULL;
    }

    Size = PURPL_MAX(MaxAmount, FsGetFileSize(Path)) + Extra;
    Buffer = CmnAlloc(Size, 1);
    if (!Buffer)
    {
        LogWarning("Failed to allocate data for file %s (%s): %s", Path,
                   FixedPath, strerror(errno));
        *ReadAmount = 0;
        return NULL;
    }

    Read = fread(Buffer, 1, Size, File);
    if (Read != Size - Extra)
    {
        LogWarning("Failed to read file %s (%s): %s", Path, FixedPath,
                   strerror(errno));
        *ReadAmount = 0;
        CmnFree(Buffer);
        return NULL;
    }

    CmnFree(FixedPath);
    *ReadAmount = Read;
    return Buffer;
}

BOOLEAN
FsWriteFile(_In_ PCSTR Path, _In_reads_bytes_(Size) PVOID Data,
            _In_ SIZE_T Size, _In_ BOOLEAN Append)
{
    FILE *File;
    BOOLEAN Success;
    PCSTR FixedPath;

    FixedPath = PlatFixPath(Path);
    LogTrace("%s %zu byte(s) to file %s (%s)", Append ? "Appending" : "Writing",
             Size, Path, FixedPath);
    File = fopen(FixedPath, Append ? "ab" : "wb");
    if (!File)
    {
        LogWarning("Failed to open file %s (%s): %s", Path, FixedPath,
                   strerror(errno));
        return FALSE;
    }
    CmnFree(FixedPath);

    Success = fwrite(Data, 1, Size, File) == Size;

    fclose(File);

    return Success;
}
