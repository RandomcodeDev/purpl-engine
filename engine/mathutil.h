/// @file mathutil.h
///
/// @brief This file defines math utilities
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "purpl/purpl.h"

/// @brief Create a transformation matrix
///
/// @param[in] Position The translation to apply (default is 0, 0, 0)
/// @param[in] Rotation The rotation to apply (default is 0, 0, 0, 0)
/// @param[in] Scale The scaling to apply (default is 1, 1, 1)
/// @param[out] Transform The transformation matrix to fill
extern VOID MthCreateTransformMatrix(_In_opt_ vec3 Position, _In_opt_ vec4 Rotation, _In_opt_ vec3 Scale,
                                     _Out_ mat4 Transform);
