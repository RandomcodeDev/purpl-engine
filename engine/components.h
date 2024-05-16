#pragma once

#include "purpl/purpl.h"

#include "common/common.h"

PURPL_MAKE_COMPONENT(struct, POSITION, { vec3 Value; })

PURPL_MAKE_COMPONENT(struct, ROTATION, { versor Value; })
#define ECS_SET_ROTATION(Entity, Angle, X, Y, Z)                                                                       \
    {                                                                                                                  \
        ROTATION R_;                                                                                                   \
        glm_quat(R_.Value, glm_rad(Angle), (X), (Y), (Z));                                                             \
        ecs_set_ptr(EcsGetWorld(), Entity, ROTATION, &R_);                                                             \
    }

PURPL_MAKE_COMPONENT(struct, SCALE, { vec3 Value; })
