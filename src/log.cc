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

void P_EXPORT purpl::logger::write(int index, int level, const wchar_t *fmt, ...)
{
	
}
