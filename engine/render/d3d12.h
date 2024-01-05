/*++

Copyright (c) 2023 MobSlicer152

Module Name:

    render.h

Abstract:

    This module declares the DirectX 12 backend.

--*/

#pragma once

#include "purpl/purpl.h"

#include "common/log.h"

typedef struct RENDER_BACKEND RENDER_BACKEND, *PRENDER_BACKEND;

extern
VOID
D3d12SetupBackend(
    _In_ PRENDER_BACKEND Backend
    );
