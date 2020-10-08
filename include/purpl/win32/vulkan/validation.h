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

namespace purpl
{
/* The validation layers to use for debugging */
const char enabled_layers[][VK_MAX_EXTENSION_NAME_SIZE] = { "VK_LAYER_KHRONOS_layer" };

/*
 * Checks the support for the validation layers requested.
 * Defined in validation.cc
 */
bool check_validation_layer_support(void);
}

#endif /* !PURPL_WIN32_VULKAN_VALIDATION_H */
