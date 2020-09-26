#pragma once

#ifndef PURPL_WIN32_VULKAN_INST_H
#define PURPL_WIN32_VULKAN_INST_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <purpl/macro.h>

#include <vulkan/vulkan.h>

namespace purpl {
class P_EXPORT win32_vulkan_inst {
public:
	/*
	 * Sets up the instance.
	 * Defined in inst.cc
	 */
	win32_vulkan_inst();

private:
	VkInstance inst;
	VkApplicationInfo app_info;
};
}

#endif
