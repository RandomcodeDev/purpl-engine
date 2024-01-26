/*++

Copyright (c) 2024 Randomcode Developers

Module Name:

    purpl/purpl/purpl.h

Abstract:

    Global header. Changes will make a lot of stuff rebuild.

--*/

#pragma once

#define PURPL_VERBOSE 1
#define PURPL_USE_MIMALLOC 1

#ifndef PURPL_RESOURCE_FILE

#ifdef __cplusplus
#define BEGIN_EXTERN_C extern "C" {
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
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#ifdef __cplusplus
#define _Noreturn
#define _Thread_local thread_local
#endif

#ifdef PURPL_WIN32
#if __STDC_VERSION__ <= 201100l
#define _Noreturn __declspec(noreturn)
#define _Thread_local __declspec(thread)
#endif

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>

#include <shellapi.h>
#include <ShlObj.h>
#include <uxtheme.h>

#ifndef PURPL_GDKX
#include <dbghelp.h>
#include <psapi.h>
#include <tlhelp32.h>
#endif
#endif

#ifdef PURPL_UNIX
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/utsname.h>

#ifndef PURPL_SWITCH
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#endif
#include <unistd.h>

#include "common/wintypes.h"
#endif

#define CGLM_CLIPSPACE_INCLUDE_ALL
#include "cglm/cglm.h"

#include "cJSON.h"

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

BEGIN_EXTERN_C

//
// Cross platform entry point
//

INT
PurplMain(
    _In_ INT ArgumentCount,
    _In_ PCHAR* Arguments
    );

#endif

//
// Game name
//

#define GAME_NAME "Purpl"
#define GAME_EXECUTABLE_NAME "purpl"

//
// Organization that made the game
//

#define GAME_ORGANIZATION "Randomcode Developers"

//
// Game version
//

#include "config.h"

//
// Switch mountpoints
//

#ifdef PURPL_SWITCH
#define SWITCH_MAKE_MOUNTPOINT(Name) Name ":/"
#define SWITCH_USERDATA_NAME "user"
#define SWITCH_USERDATA_MOUNTPOINT SWITCH_MAKE_MOUNTPOINT(SWITCH_USERDATA_NAME)
#define SWITCH_ROMFS_NAME "romfs"
#define SWITCH_ROMFS_MOUNTPOINT SWITCH_MAKE_MOUNTPOINT(SWITCH_ROMFS_NAME)
#else
#define SWITCH_USERDATA_NAME ""
#define SWITCH_USERDATA_MOUNTPOINT ""
#define SWITCH_ROMFS_NAME ""
#define SWITCH_ROMFS_MOUNTPOINT ""
#endif

END_EXTERN_C
