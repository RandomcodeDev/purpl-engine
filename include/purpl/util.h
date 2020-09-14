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
#define getcwd _getcwd

#define vsnwprintf _vsnwprintf
#define wfopen _wfopen
#endif /* _WIN32 */

#include "macro.h"
#include "types.h"

namespace purpl
{

/*
 * Formats text, identical to how vprintf would.
 * Defined in util.cc
 */
wchar_t *fmt_text_va(const wchar_t *fmt, va_list *args);

/*
 * Formats text, how printf would.
 * Defined in util.cc
 */
wchar_t *fmt_text(const wchar_t *fmt, ...);
}

#endif /* !PURPL_UTIL_H */
