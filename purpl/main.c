/// @file main.c
///
/// @brief This file implements the cross-platform entry point.
///
/// @copyright (c) 2024 Randomcode Developers

#include "purpl/purpl.h"

#include "common/common.h"

#include "engine/engine.h"

INT PurplMain(_In_ PCHAR *Arguments, _In_ UINT ArgumentCount)
{
    CmnInitialize(Arguments, ArgumentCount);
    EngInitialize();

    EngMainLoop();

    EngShutdown();
    CmnShutdown();

    return 0;
}
