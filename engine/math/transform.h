/// @file mathutil.h
///
/// @brief This file defines math utilities
///
/// @copyright (c) 2024 Randomcode Developers

#pragma once

#include "common/common.h"

/// @brief Create a transformation matrix
///
/// @param[in] Position The translation to apply (default is 0, 0, 0)
/// @param[in] Rotation The rotation to apply (default is 0, 0, 0, 0)
/// @param[in] Scale The scaling to apply (default is 1, 1, 1)
/// @param[out] Transform The transformation matrix to fill
static inline VOID MthCreateTransformMatrix(_In_opt_ CONST vec3 Position, _In_opt_ CONST versor Rotation,
                                            _In_opt_ CONST vec3 Scale, _Out_ mat4 Transform)
{
    vec3 DefaultPosition = {0.0f, 0.0f, 0.0f};
    versor DefaultRotation;
    glm_quat_identity(DefaultRotation);
    vec3 DefaultScale = {1.0f, 1.0f, 1.0f};

    glm_mat4_identity(Transform);
    glm_translate(Transform, (PFLOAT)(Position ? Position : DefaultPosition));
    glm_scale(Transform, (PFLOAT)(Scale ? Scale : DefaultScale));
    glm_quat_rotate(Transform, (PFLOAT)(Rotation ? Rotation : DefaultRotation), Transform);
}

/// @brief Rotate a quaternion around its axis
///
/// @param[in,out] Quaternion The quaternion to rotate
/// @param[in] Angle The number of degrees to rotate by
static inline VOID MthRotateQuaternion(_Inout_ versor Quaternion, _In_ FLOAT Angle)
{
    vec3 Axis;
    glm_quat_axis(Quaternion, Axis);
    versor Rotation;
    glm_quatv(Rotation, glm_rad(Angle), Axis);
    glm_quat_mul(Quaternion, Rotation, Quaternion);
}
