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
#include <io.h>

#include <windows.h>

#define getcwd \
	_getcwd /*
		 * Note: This comment was originally rude to Microsoft,
		 * but what purpose does that serve? <sarcastic comment here :)>
		 * Dear Microsoft, nobody cares about ISO compliance as long as we can
		 * write portable code. Developers only hate you for not being POSIX compliant!
		 */
#elif __linux__
#include <unistd.h>
#include <sys/mman.h>
#endif /* _WIN32 */

#include "macro.h"
#include "types.h"

namespace purpl
{
/*
 * The buffers returned by these functions last until you free them,
 * and if you don't you get a memory leak.
 */

/* Formats text, identical to how vprintf would. */
char P_EXPORT *fmt_text_va(const char *fmt, va_list *args);

/* Formats text, how printf would. */
char P_EXPORT *fmt_text(const char *fmt, ...);

/*
 * These are wrappers for system calls that map and unmap files
 * to and from the program's virtual memory. Just use/edit the buffer and then unmap the file.
 */

/*
 * Uses the appropriate system call based on what operating system the
 * engine is compiled for. Remember to use unmap_file and not free, because
 * that won't end well :). This returns an int because platform specifics.
 * Maybe there should be a function to get error text.
 */
int P_EXPORT map_file(FILE *fp, size_t *len_ret, char **buf);

/*
 * Unmaps a file using the right system call.
 * Use this to get rid of the buffers for map/read_file*
 * if you enable mapping the file.
 */
int P_EXPORT unmap_file(char *buf, size_t len);

/*
 * Seeing as the buffers returned by these functions if you set map to true
 * are not buffers but pages allocated by a system call, use unmap_file.
 * Note that writing into the file is both not necessary and can possibly
 * cause errors in this case. Just edit the buffer and unmap it.
 */

/*
 * Loads/maps the contents of a file into a buffer. len_ret is optional. if map is false
 */
char P_EXPORT *load_file_fp(FILE *fp, bool map, size_t *len_ret);

/*
 * Loads/maps the contents of a file into a buffer. len_ret is optional if map is false.
 */
char P_EXPORT *read_file(const char *name, bool map, size_t *len_ret);
}

#endif /* !PURPL_UTIL_H */
