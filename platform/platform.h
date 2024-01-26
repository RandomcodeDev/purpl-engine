/*++

Copyright (c) 2023 Randomcode Developers

Module Name:

    platform.h

Abstract:

    This module contains the platform abstraction API.

--*/

#pragma once

#include "purpl/purpl.h"

//
// Defined if the platform has a relevant location other than standard output
// to send log output to
//

#if defined PURPL_WIN32 || defined PURPL_SWITCH
#define PURPL_HAVE_PLATPRINT 1
#endif

//
// Initialize the platform layer
//

extern
VOID
PlatInitialize(
    VOID
    );

//
// Clean up platform-specific resources
//

extern
VOID
PlatShutdown(
    VOID
    );

//
// Get a stack backtrace string
//

extern
PCSTR
PlatCaptureStackBackTrace(
    _In_ SIZE_T FramesToSkip,
    _In_ SIZE_T MaxFrames
    );

//
// Display an error message and terminate the process
//

extern
_Noreturn
VOID
PlatError(
    _In_ PCSTR Message
    );

//
// Get a description of the operating system
//

extern
PCSTR
PlatGetDescription(
    VOID
    );

//
// Get the return address
//

extern
PVOID
PlatGetReturnAddress(
    VOID
    );

//
// Get the user data directory (AppData, XDG_DATA_HOME, ~/.local, etc)
//

extern
PCSTR
PlatGetUserDataDirectory(
    VOID
    );

//
// Get the current time in milliseconds
//

extern
UINT64
PlatGetMilliseconds(
    VOID
    );

//
// Create a directory
//

extern
BOOLEAN
PlatCreateDirectory(
    _In_ PCSTR Path
    );

//
// Output text in a platform specific way like OutputDebugString
//

#ifdef PURPL_HAVE_PLATPRINT
extern
VOID
PlatPrint(
    _In_ PCSTR Text
    );
#endif

//
// Fix a path (if necessary)
//

extern
PCHAR
PlatFixPath(
    _In_ PCSTR Path
    );
