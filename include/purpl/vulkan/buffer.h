#pragma once

#ifndef PURPL_VULKAN_BUFFER_H
#define PURPL_VULKAN_BUFFER_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <cglm/types.h>

#include "purpl/macro.h"

#include <vulkan/vulkan.h>

#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
/* A structure to hold information about a vertex */
struct vert_info {
	vec4 pos;
	vec4 colour;
};

/*
 * Gets the binding description for a vertex input buffer.
 * Defined in buffer.cc
 */
VkVertexInputBindingDescription get_vert_input_binding_desc(void);

/*
 * Gets the attribute descriptions for a vertex input buffer.
 * Defined in buffer.cc
 */
VkVertexInputAttributeDescription *get_vert_input_attrib_descs(void);
}

#endif /* !PURPL_VULKAN_BUFFER_H */
