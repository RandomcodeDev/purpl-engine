#pragma once

#ifndef PURPL_LOG_H
#define PURPL_LOG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "macro.h"
#include "types.h"
#include "util.h"

#define FATAL 0
#define ERR 1
#define WARN 2
#define INFO 3
#define DEBUG 4

#define P_MAX_LOGS 32

#ifndef NDEBUG
#define P_DEFAULT_LOG_LEVEL DEBUG
#else
#define P_DEFAULT_LOG_LEVEL WARN
#endif

namespace purpl
{
class P_EXPORT logger {
    public:
	/* The initial log file */
	uint logindex;

	/* Starts the logger instance. */
	logger(uint *index, uint initial_level, const char *fname, ...);

	/* Opens another log file. Returns an error code on failure. */
	uint open(uint initial_level, const char *fname, ...);

	/* Retrieve the level of the specified index */
	uint get_level(uint index);

	/* Set the level of the specified index */
	void set_level(uint index, uint level);

	/* Write a message to the a log file. Sets errno on failure. */
	void write(uint index, uint level, const char *file, uint line,
		   const char *fmt, ...);

	/* Close a log file. Sets errno on failure. */
	void close(uint index, bool write_goodbye, const char *msg, ...);

	/* Frees the logger's resources and closes the log files. */
	~logger(void);

    private:
	/* The log files that are open. */
	FILE *logs[P_MAX_LOGS];

	/* The number of log files opened */
	uint nlogs;

	/*
	 * The log level for each log file. Any message with a level less than or equal to
	 * this will be logged, and everything else will be ignored.
	 */
	uint levels[P_MAX_LOGS];
};
}

#endif /* !PURPL_LOG_H */
