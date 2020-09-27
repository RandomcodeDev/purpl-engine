#pragma once

#ifndef PURPL_WIN32_VULKAN_INST_H
#define PURPL_WIN32_VULKAN_INST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"

#include "macro.h"
#include "util.h"

namespace purpl {
class P_EXPORT win32_vulkan_inst {
public:
	/*
	 * Sets up the instance.
	 * Defined in inst.cc
	 */
	win32_vulkan_inst(void);

	/*
	 * Frees the various resources used by the instance;
	 * Defined in inst.cc
	 */
	~win32_vulkan_inst(void);

private:
	VkInstance inst;
	VkApplicationInfo app_info;
};
}

#endif
