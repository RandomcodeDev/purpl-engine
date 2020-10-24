#include "purpl/log.h"

P_EXPORT purpl::logger::logger(uint *index, uint initial_level,
			       const char *fname, ...)
{
	va_list args;
	char *buf;
	uint i;

	this->nlogs = 1;

	/* Get the file name */
	va_start(args, fname);
	buf = fmt_text_va(fname, &args);
	va_end(args);

	/* Zero our file streams */
	for (i = 0; i < P_MAX_LOGS; i++)
		memset(&this->logs[i], 0, sizeof(FILE *));

	/* Set the level */
	this->levels[this->nlogs - 1] = initial_level;

	/* Open the file, return stdout if that fails */
	this->logs[this->nlogs - 1] = fopen(buf, "wb+");
	if (!this->logs[this->nlogs - 1] || (strcmp(buf, "stdout") == 0))
		this->logs[this->nlogs - 1] = stdout;

	/* Store the index */
	this->logindex = this->nlogs - 1;

	if (index)
		*index = this->logindex;

	/* Free our buffer */
	free(buf);
}

P_EXPORT uint purpl::logger::open(uint initial_level, const char *fname, ...)
{
	va_list args;
	char *buf;

	/* Set the number of logs, will be decreased on failure */
	this->nlogs++;

	va_start(args, fname);
	buf = fmt_text_va(fname, &args);
	va_end(args);

	/* Set the log's level */
	this->levels[this->nlogs - 1] = initial_level;

	/* Open the file */
	this->logs[this->nlogs - 1] = fopen(buf, "wb+");
	if (!this->logs[this->nlogs - 1]) {
		this->levels[this->nlogs - 1] = NULL;
		this->logs[this->nlogs - 1] = NULL;

		/* Free our buffer */
		free(buf);

		this->nlogs--;
		return EIO;
	}

	/* Free our buffer */
	free(buf);

	return this->nlogs - 1;
}

P_EXPORT uint purpl::logger::get_level(uint index)
{
	if (index < P_MAX_LOGS)
		return this->levels[index];
	else
		return 0;
}

P_EXPORT void purpl::logger::set_level(uint index, uint level)
{
	if (index < P_MAX_LOGS)
		this->levels[index] = level;
}

P_EXPORT void purpl::logger::write(uint index, uint level, const char *filename,
				   uint line, const char *fmt, ...)
{
	va_list args;
	char *buf;
	time_t rawtime;
	struct tm *timeinfo;

	/* Allocate a buffer */
	buf = (char *)calloc(P_MAX_TXT_BUF, sizeof(char));
	if (!buf) {
		errno = ENOMEM;
		return;
	}

	/* Get the time */
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	/* Prepend the log level text to the message */
	switch (level) {
	case FATAL:
		sprintf(buf, "[fatal error] ");
		break;
	case ERR:
		sprintf(buf, "[error] ");
		break;
	case WARN:
		sprintf(buf, "[warning] ");
		break;
	default:
	case INFO:
		sprintf(buf, "[info] ");
		break;
	case DEBUG:
		sprintf(buf, "[debug] ");
		break;
	}

	/* Put the time into the buffer next */
	sprintf(buf + strlen(buf), "[%s ", asctime(timeinfo));

	/* Now the file and line number */
	sprintf(buf + strlen(buf) - 2, "] [%s:%d] ", filename, line);

	/* Now for the message */
	va_start(args, fmt);
	sprintf(buf + strlen(buf), "%s\n", fmt_text_va(fmt, &args));
	va_end(args);

	/* Now try to write to the log file, if it exists */
	if (this->logs[index] && this->levels[index] >= level) {
		fprintf(this->logs[index], "%s", buf);
		fflush(this->logs[index]);
	}
}

P_EXPORT void purpl::logger::close(uint index, bool write_goodbye,
				   const char *msg, ...)
{
	va_list args;

	/* Optionally write a message to the log */
	if (write_goodbye) {
		va_start(args, msg);
		this->write(index, P_DEFAULT_LOG_LEVEL, P_FILENAME, __LINE__,
			    "%s", fmt_text_va(msg, &args));
		va_end(args);
	}

	/* Adjust these values to make sure nothing bad happens */
	this->levels[index] = P_DEFAULT_LOG_LEVEL;
	this->nlogs--;

	/* Close the file */
	if (this->logs[index]) {
		fflush(this->logs[index]);
		fclose(this->logs[index]);
	}
}

purpl::logger::~logger(void)
{
	uint i;

	/* Close all the open logs */
	for (i = 0; i < P_MAX_LOGS; i++) {
		if (this->logs[i])

			this->close(
				i, true,
				"This logger is terminating, have a nice day.");
	}
}
