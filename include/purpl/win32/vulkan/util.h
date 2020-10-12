#pragma once

#ifndef PURPL_WIN32_VULKAN_UTIL_H
#define PURPL_WIN32_VULKAN_UTIL_H

#include <stdio.h> /* For debugging */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <vulkan/vulkan.h>

#include "purpl/macro.h"

#include "macro.h"

namespace purpl
{
/*
 * Copied from GLFW, specifically vulkan.c, line 165. GLFW authors, if you don't like
 * this use of your code I'll take it out and write my own version, just let me know.
 * Defined in util.cc
 */
const char *get_vulkan_err_str(VkResult result);

/*
 * Gets all the available Vulkan extensions, and how many there are.
 * In the event of an error, count or errno will hold the error code and NULL will be returned.
 * Defined in util.cc
 */
VkExtensionProperties *get_vulkan_exts(uint *count);

/*
 * Checks if the required extensions are present (VK_KHR_surface and VK_KHR_win32_surface)
 * If they aren't, returns NULL.
 * Defined in util.cc
 */
char **check_required_exts_avail(void);
}

#endif
