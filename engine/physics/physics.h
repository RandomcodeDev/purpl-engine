/// @file physics.h
///
/// @brief This file declares the physics API.
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

#include "common/common.h"
#include "common/configvar.h"
#include "common/log.h"

/// @brief Backend thing
typedef UINT64 PHYSICS_HANDLE;

PURPL_MAKE_COMPONENT(struct, PHYSICS_BODY, { PHYSICS_HANDLE Handle; })

/// @brief Physics backend
PURPL_MAKE_TAG(struct, PHYSICS_BACKEND, {
    VOID (*Initialize)(VOID);
    VOID (*Update)(_In_ FLOAT Delta);
    VOID (*Shutdown)(VOID);

    //VOID (*CreateBody)(_Out_ PPHYSICS_BODY Body, _In_ )
})
