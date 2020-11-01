#pragma once

#ifndef PURPL_VULKAN_SPIRV_H
#define PURPL_VULKAN_SPIRV_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
/*
 * Creates a shader module from the contents of a SPIR-V binary.
 * Defined in spirv.cc
 */
VkShaderModule create_shader_module(VkDevice device, const char *spirv, uint len);
}
#endif /* !PURPL_VULKAN_SPIRV_H */
