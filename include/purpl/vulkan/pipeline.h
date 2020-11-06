#pragma once

#ifndef PURPL_VULKAN_PIPELINE_H
#define PURPL_VULKAN_PIPELINE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"
#include "purpl/types.h"
#include "purpl/util.h"

#include "spirv.h"

namespace purpl
{
/*
 * Creates a render pass.
 * Defined in pipeline.cc
 */
VkRenderPass create_render_pass(VkDevice device, VkFormat image_format);

/*
 * Creates a graphics pipeline.
 * Defined in pipeline.cc
 */
VkPipeline create_graphics_pipeline(VkDevice device, VkExtent2D viewport_extent,
				    VkRenderPass render_pass,
				    const char *vert_shader_path,
				    const char *frag_shader_path,
				    VkPipelineLayout *pipeline_layout);
}

#endif /* !PURPL_VULKAN_PIPELINE_H */
