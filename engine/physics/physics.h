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

PURPL_MAKE_COMPONENT(struct, PHYSICS_BODY, {
    PHYSICS_HANDLE Handle;
    vec3 Velocity;

})
