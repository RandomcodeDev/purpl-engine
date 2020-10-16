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
	int logindex;

	/*
	 * Starts the logger instance.
	 * Defined in log.cc
	 */
	logger(int initial_level, const char *fname, ...);

	/*
	 * Opens another log file. Returns an error code on failure.
	 * Defined in log.cc
	 */
	int open(int initial_level, const char *fname, ...);

	/* Retrieve the level of the specified index */
	int get_level(int index);

	/* Set the level of the specified index */
	void set_level(int index, int level);

	/*
	 * Write a message to the a log file. Sets errno on failure.
	 * Defined in log.cc 
	 */
	void write(int index, int level, const char *file, int line, const char *fmt, ...);

	/*
	 * Close a log file. Sets errno on failure. 
	 * Defined in log.cc 
	 */
	void close(int index, const char *msg, ...); 

	/*
	 * Frees the loggers resources and closes the log files.
	 * Defined in log.cc
	 */
	~logger(void);
    
    private:
	/* The log files that are open. */
	FILE *logs[P_MAX_LOGS];
	
	/* The number of log files opened */
	int nlogs;
	
	/*
	 * The log level for each log file. Any message with a level less than or equal to
	 * this will be logged, and everything else will be ignored.
	 */
	int levels[P_MAX_LOGS];
};
}

#endif /* !PURPL_LOG_H */
