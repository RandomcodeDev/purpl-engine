#include <purpl/log.h>

P_EXPORT purpl::logger::logger(int *index, int initial_level, const wchar_t *fname, ...)
{
	va_list args;
	wchar_t *buf;
	
	this->nlogs = 1;

	/* Get the file name */
	va_start(args, fname);
	buf = fmt_text_va(fname, &args);
	va_end(args);

	/* Set the level */
	this->levels[this->nlogs - 1] = initial_level;

	/* Open the file, return stdout if that fails */
	this->logs[this->nlogs - 1] = wfopen(buf, L"wb+");
	if (!this->logs[this->nlogs - 1])
		this->logs[this->nlogs - 1] = stdout;

	/* Return the index (through a parameter */
	*index = this->nlogs - 1;
}

int P_EXPORT purpl::logger::open(int initial_level, const wchar_t *fname, ...)
{
	va_list args;
	wchar_t *buf;

	/* Set the number of logs, will be decreased on failure */
	this->nlogs++;

	va_start(args, fname);
	buf = fmt_text_va(fname, &args);
	va_end(args);

	/* Set the log's level */
	this->levels[this->nlogs - 1] = initial_level;

	/* Open the file */
	this->logs[this->nlogs - 1] = wfopen(buf, L"wb+");
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
	this->levels[index] = level;
}

void P_EXPORT purpl::logger::write(int index, int level, const wchar_t *file, int line, const wchar_t *fmt, ...)
{
	va_list args;
	wchar_t *buf;
	time_t rawtime;
	struct tm *timeinfo;

	buf = (wchar_t *)calloc(P_MAX_TXT_BUF, sizeof(wchar_t));
	if (!buf) {
		errno = ENOMEM;
		return;
	}

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	
	switch (level) {
	case FATAL:
		swprintf(buf, L"[fatal error] ");
		break;
	case ERR:
		swprintf(buf, L"[error] ");
		break;
	case WARN:
		swprintf(buf, L"[warning] ");
		break;
	default:
	case INFO:
		swprintf(buf, L"[info] ");
		break;
	case DEBUG:
		swprintf(buf, L"[debug] ");
		break;
	}

	swprintf(buf + wcslen(buf), L"[%s ", wasctime(timeinfo));

#ifdef _WIN32
	swprintf(buf + wcslen(buf) - 2, L"] [%s:%d] ",
		 file,
		 line);
#else
	swprintf(buf + wcslen(buf), L" [%s:%d] ", file, line);
#endif

	va_start(args, fmt);
	swprintf(buf + wcslen(buf), L"%s\n", fmt_text_va(fmt, &args));
	va_end(args);

	if (this->logs[index] && this->levels[index] >= level)
		fwprintf(this->logs[index], L"%s", buf);
}

void purpl::logger::close(int index, const wchar_t *msg, ...)
{
	va_list args;

	va_start(args, msg);
	this->write(index, P_DEFAULT_LOG_LEVEL, P_FILENAME, __LINE__, L"%s", fmt_text_va(msg, &args));
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
			this->close(i, L"This logger is terminating, have a nice day.");
	}
}
