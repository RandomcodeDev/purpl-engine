#pragma once

#ifndef PURPL_VULKAN_BUFFER_H
#define PURPL_VULKAN_BUFFER_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <cglm/call.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

namespace purpl
{
/* A structure to hold information about a vertex */
struct vert_info {
	vec4 pos;
	vec4 color;
};

/*
 * Gets the binding description for a vertex input buffer.
 * Defined in buffer.cc
 */
VkVertexInputBindingDescription get_vert_input_binding_desc(struct vert_info vert);

/*
 * Gets the attribute descriptions for a vertex input buffer.
 * Defined in buffer.cc
 */
VkVertexInputAttributeDescription *get_vert_input_attrib_descs(struct vert_info vert);
}

#endif /* !PURPL_VULKAN_BUFFER_H */
