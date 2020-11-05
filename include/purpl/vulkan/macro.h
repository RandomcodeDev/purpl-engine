#pragma once

#ifndef PURPL_VULKAN_MACRO_H
#define PURPL_VULKAN_MACRO_H

#ifndef NDEBUG
#define P_REQUIRED_VULKAN_EXT_COUNT \
	3 /* In the event we need more for a different platform, this will change */
#else
#define P_REQUIRED_VULKAN_EXT_COUNT 2
#endif

#define P_REQUIRED_VULKAN_DEVICE_EXT_COUNT 1

#ifndef NDEBUG
#define P_REQUIRED_VULKAN_LAYER_COUNT 1
#endif

#define P_VULKAN_MAX_FRAMES_IN_FLIGHT 2

#endif
