/*++

Copyright (c) 2024 MobSlicer152

Module Name:

    vk.c

Abstract:

    This module implements the Vulkan backend.

--*/

#include "render.h"

static
VOID
Initialize(
    VOID
    )
{}

static
VOID
BeginFrame(
    VOID
    )
{}

static
VOID
EndFrame(
    VOID
    )
{}

static
VOID
Shutdown(
    VOID
    )
{}

VOID
VkInitializeBackend(
    _Out_ PRENDER_BACKEND Backend
    )
{
    LogDebug("Filling out render backend for Vulkan");

    Backend->Initialize = Initialize;
    Backend->BeginFrame = BeginFrame;
    Backend->EndFrame = EndFrame;
    Backend->Shutdown = Shutdown;
}
