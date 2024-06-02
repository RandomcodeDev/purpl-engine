#pragma once

#include "purpl/purpl.h"

#include "common/common.h"

PURPL_MAKE_COMPONENT(struct, POSITION, { vec3 Value; })

PURPL_MAKE_COMPONENT(struct, ROTATION, { vec3 Value; })

PURPL_MAKE_COMPONENT(struct, SCALE, { vec3 Value; })
