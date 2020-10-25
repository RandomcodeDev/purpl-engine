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

namespace purpl
{
VkPipeline create_graphics_pipeline();
}

#endif /* !PURPL_VULKAN_PIPELINE_H */
