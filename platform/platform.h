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

/// @brief Initialize the platform layer
extern VOID PlatInitialize(VOID);

/// @brief Clean up platform-specific resources
extern VOID PlatShutdown(VOID);

/// @brief Gets a stack trace in a static buffer.
///
/// @param FramesToSkip The number of stack frames to skip.
/// @param MaxFrames    The maximum number of frames to get.
///
/// @return The address of a static buffer containing a string with
///         the formatted stack trace.
extern PCSTR PlatCaptureStackBackTrace(_In_ SIZE_T FramesToSkip,
                                       _In_ SIZE_T MaxFrames);

/// @brief Displays an error (and potentially gives the option to trigger a breakpoint), and exits the process
///
/// @param Message The error message.
_Noreturn extern VOID PlatError(_In_ PCSTR Message);

/// @brief Retrieves a string with information about the system version.
///
/// @return A static buffer containing the system description.
extern PCSTR PlatGetDescription(VOID);

/// @brief Gets the return address of the calling function
/// @return The return address of the calling function
extern PVOID PlatGetReturnAddress(VOID);

/// @brief Get the user data directory (AppData, XDG_DATA_HOME, ~/.local, etc)
///
/// @return The user data directory in a static buffer.
extern PCSTR PlatGetUserDataDirectory(VOID);

/// @brief Gets the number of milliseconds passed since an arbitrary point in time.
///
/// @return A number of milliseconds.
extern UINT64 PlatGetMilliseconds(VOID);

/// @brief Recursively creates a directory.
///
/// @param Path The path of the directory to create.
///
/// @return Whether the directory could be created.
extern BOOLEAN PlatCreateDirectory(_In_ PCSTR Path);

/// @brief Output text in a platform specific way like OutputDebugString
///
/// @param Text The text to output
#ifdef PURPL_HAVE_PLATPRINT
extern VOID PlatPrint(_In_ PCSTR Text);
#endif

/// @brief Fix a path if necessary
/// @param Path The path to fix
/// @return The fixed path in a new buffer
extern PCHAR PlatFixPath(_In_ PCSTR Path);
