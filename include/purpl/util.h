#pragma once

#ifndef PURPL_UTIL_H
#define PURPL_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>

#define getcwd _getcwd /* Damn it Microsoft, nobody cares about ISO compliance as long as we can
			* write portable code. Developers only hate you for not being POSIX compliant!
			*/
#endif /* _WIN32 */

#include "macro.h"
#include "types.h"

namespace purpl
{

/*
 * Formats text, identical to how vprintf would.
 * Defined in util.cc
 */
char *fmt_text_va(const char *fmt, va_list *args);

/*
 * Formats text, how printf would.
 * Defined in util.cc
 */
char *fmt_text(const char *fmt, ...);
}

#endif /* !PURPL_UTIL_H */
