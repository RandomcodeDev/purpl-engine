/// @file stb.c
///
/// @brief This file is used for the stb implementations.
///
/// @copyright (c) 2024 Randomcode Developers

#if PURPL_USE_MIMALLOC
#define MALLOC mi_malloc
#define REALLOC mi_realloc
#define FREE mi_free
#else
#define MALLOC malloc
#define REALLOC realloc
#define FREE free
#endif

extern void CmnError(const char *Format, ...);

#define PURPL_ASSERT(Condition)                                                \
    do                                                                         \
    {                                                                          \
        if (!(Condition))                                                      \
            CmnError("Assertion failed: " #Condition);                         \
    } while (0);

#define STB_DS_IMPLEMENTATION 1
#define STBDS_ASSERT PURPL_ASSERT

#define STB_IMAGE_IMPLEMENTATION 1
#define STBI_ASSERT PURPL_ASSERT
#define STBI_MALLOC MALLOC
#define STBI_REALLOC REALLOC
#define STBI_FREE FREE
#define STBI_MEMMOVE memmove

#define STB_IMAGE_WRITE_IMPLEMENTATION 1
#define STBIW_ASSERT PURPL_ASSERT
#define STBIW_MALLOC MALLOC
#define STBIW_REALLOC REALLOC
#define STBIW_FREE FREE
#define STBIW_MEMMOVE memmove

#define STB_SPRINTF_IMPLEMENTATION 1
#define STBSP_ASSERT PURPL_ASSERT
#define STBSP_MALLOC MALLOC
#define STBSP_REALLOC REALLOC
#define STBSP_FREE FREE
#define STBSP_MEMMOVE memmove

#include "purpl/purpl.h"
