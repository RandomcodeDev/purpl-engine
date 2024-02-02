/// @file main.c
///
/// @brief This module implements the cross-platform entry point.
///
/// @copyright (c) 2024 Randomcode Developers

#include "purpl/purpl.h"

#include "common/common.h"

#include "engine/engine.h"

INT PurplMain(_In_ INT ArgumentCount, _In_ PCHAR *Arguments)
{
    UNREFERENCED_PARAMETER(ArgumentCount);
    UNREFERENCED_PARAMETER(Arguments);

    CmnInitialize();
    EngInitialize();

    EngMainLoop();

    EngShutdown();
    CmnShutdown();

    return 0;
}
