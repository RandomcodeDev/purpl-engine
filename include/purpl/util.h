#pragma once

#ifndef PURPL_UTIL_H
#define PURPL_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>

#ifdef _WIN32
#include <direct.h>
#define _getcwd getcwd
#endif /* _WIN32 */

#include "macro.h"
#include "types.h"

#endif /* !PURPL_UTIL_H */
