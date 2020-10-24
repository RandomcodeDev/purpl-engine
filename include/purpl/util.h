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

#define getcwd _getcwd /* Note: This comment was originally rude to Microsoft, but what purpose does that serve? <sarcastic comment here :)>
			* Dear Microsoft, nobody cares about ISO compliance as long as we can
			* write portable code. Developers only hate you for not being POSIX compliant!
			*/
#endif /* _WIN32 */

#include "macro.h"
#include "types.h"

namespace purpl
{
/* The buffers returned by these functions last until you free them, and if you don't you get a memory leak. */

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
