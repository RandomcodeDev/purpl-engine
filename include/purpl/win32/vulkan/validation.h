#pragma once

#ifndef PURPL_WIN32_VULKAN_VALIDATION_H
#define PURPL_WIN32_VULKAN_VALIDATION_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

#include <vulkan/vulkan.h>

/* We have this macro so that this can be changed by the end user */
#define P_VULKAN_VALIDATION_LAYERS       \
	{                                \
		"VK_LAYER_KHRONOS_layer" \
	}

namespace purpl
{
/* The validation layers to use for debugging */
const char enabled_layers[][] = P_VULKAN_VALIDATION_LAYERS;

/*
 * Checks the support for the validation layers requested.
 * Defined in validation.cc
 */
bool check_validation_layer_support(void);
}

#endif /* !PURPL_WIN32_VULKAN_VALIDATION_H */
