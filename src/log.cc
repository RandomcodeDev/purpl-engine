#include <purpl/log.h>

P_EXPORT purpl::logger::logger(int *index, int initial_level, const char *fname, ...)
{
	va_list args;
	char *buf;
	
	this->nlogs = 1;

	/* Get the file name */
	va_start(args, fname);
	buf = fmt_text_va(fname, &args);
	va_end(args);

	/* Set the level */
	this->levels[this->nlogs - 1] = initial_level;

	/* Open the file, return stdout if that fails */
	this->logs[this->nlogs - 1] = fopen(buf, "wb+");
	if (!this->logs[this->nlogs - 1] || (strcmp(buf, "stdout") == 0))
		this->logs[this->nlogs - 1] = stdout;

	/* Return the index (through a parameter */
	*index = this->nlogs - 1;
}

int P_EXPORT purpl::logger::open(int initial_level, const char *fname, ...)
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
		
		this->nlogs--;
		return EIO;
	}

	return this->nlogs - 1;
}

int P_EXPORT purpl::logger::get_level(int index)
{
	return this->levels[index];
}

void P_EXPORT purpl::logger::set_level(int index, int level)
{
	if (index < P_MAX_LOGS)
		this->levels[index] = level;
}

void P_EXPORT purpl::logger::write(int index, int level, const char *filename, int line, const char *fmt, ...)
{
	va_list args;
	char *buf;
	time_t rawtime;
	struct tm *timeinfo;

	buf = (char *)calloc(P_MAX_TXT_BUF, sizeof(char));
	if (!buf) {
		errno = ENOMEM;
		return;
	}

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
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

	sprintf(buf + strlen(buf), "[%s ", asctime(timeinfo));

	sprintf(buf + strlen(buf) - 2, "] [%s:%d] ",
		 filename,
		 line);

	va_start(args, fmt);
	sprintf(buf + strlen(buf), "%s\n", fmt_text_va(fmt, &args));
	va_end(args);

	if (this->logs[index] && this->levels[index] >= level) {
		fprintf(this->logs[index], "%s", buf);
		fflush(this->logs[index]);
	}
}

void purpl::logger::close(int index, const char *msg, ...)
{
	va_list args;

	va_start(args, msg);
	this->write(index, P_DEFAULT_LOG_LEVEL, P_FILENAME, __LINE__, "%s", fmt_text_va(msg, &args));
	va_end(args);

	if (this->logs[index])
		fclose(this->logs[index]);

	this->levels[index] = P_DEFAULT_LOG_LEVEL;

	this->nlogs--;
}

purpl::logger::~logger(void)
{
	int i;

	for (i = 0; i < P_MAX_LOGS; i++) {
		if (this->logs[i])
			this->close(i, "This logger is terminating, have a nice day.");
	}
}
