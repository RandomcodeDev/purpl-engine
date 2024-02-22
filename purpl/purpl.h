/// @file purpl/purpl.h
///
/// @brief Global header. Changes will make a lot of stuff rebuild.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "config.h"

#ifndef PURPL_RESOURCE_FILE

#ifdef __cplusplus
#define BEGIN_EXTERN_C                                                                                                 \
    extern "C"                                                                                                         \
    {
#define END_EXTERN_C }
#else
#define BEGIN_EXTERN_C
#define END_EXTERN_C
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#ifdef PURPL_WIN32
#if !defined(__clang__) && __STDC_VERSION__ <= 201100l
#define _Noreturn __declspec(noreturn)
#define _Thread_local __declspec(thread)
#endif

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef __cplusplus
#include <comdef.h>
#endif
#include <ShlObj.h>
#include <shellapi.h>
#include <Uxtheme.h>
#include <windowsx.h>

#ifndef PURPL_GDKX
#include <DbgHelp.h>
#include <Psapi.h>
#include <TlHelp32.h>
#endif

#define strcasecmp _stricmp
#define fseeko64 _fseeki64
#endif

#ifdef PURPL_UNIX
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>

#ifndef PURPL_SWITCH
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <signal.h>
#endif

#include <malloc.h>
#include <unistd.h>

#include "common/wintypes.h"
#endif

#define CGLM_CLIPSPACE_INCLUDE_ALL
#include "cglm/cglm.h"

#include "cJSON.h"

#ifdef PURPL_DISCORD
#include "discord_rpc.h"
#endif

#if defined(PURPL_DEBUG) || defined(PURPL_RELWITHDEBINFO)
#define FLECS_HTTP
#define FLECS_REST
#endif
#include "flecs.h"

#if PURPL_USE_MIMALLOC
#include "mimalloc.h"
#endif

#include "stb/stb_ds.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "stb/stb_sprintf.h"

#ifdef PURPL_VULKAN
#include "volk.h"
#endif

#include "zstd.h"

#ifdef __cplusplus
#ifndef _Noreturn
#define _Noreturn [[noreturn]]
#endif
#ifndef _Thread_local
#define _Thread_local thread_local
#endif
#endif

BEGIN_EXTERN_C

/// @brief Cross-platform entry point
///
/// @param ArgumentCount The number of command line arguments
/// @param Arguments The command line arguments
///
/// @return 0 on success, otherwise does not return
extern INT PurplMain(_In_ PCHAR *Arguments, _In_ UINT ArgumentCount);

#endif

// Game name

#define PURPL_NAME "Purpl"
#define PURPL_EXECUTABLE_NAME "purpl"

/// @brief Organization that made the game
#define PURPL_ORGANIZATION "Randomcode Developers"

// Switch mountpoints

#ifdef PURPL_SWITCH
#define PURPL_SWITCH_MAKE_MOUNTPOINT(Name) Name ":/"
#define PURPL_SWITCH_USERDATA_NAME "user"
#define PURPL_SWITCH_USERDATA_MOUNTPOINT PURPL_SWITCH_MAKE_MOUNTPOINT(PURPL_SWITCH_USERDATA_NAME)
#define PURPL_SWITCH_ROMFS_NAME "romfs"
#define PURPL_SWITCH_ROMFS_MOUNTPOINT PURPL_SWITCH_MAKE_MOUNTPOINT(PURPL_SWITCH_ROMFS_NAME)
#else
#define PURPL_SWITCH_USERDATA_NAME ""
#define PURPL_SWITCH_USERDATA_MOUNTPOINT ""
#define PURPL_SWITCH_ROMFS_NAME ""
#define PURPL_SWITCH_ROMFS_MOUNTPOINT ""
#endif

END_EXTERN_C
