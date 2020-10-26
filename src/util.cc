#include "purpl/util.h"

char *purpl::fmt_text_va(const char *fmt, va_list *args)
{
	char *buf;
	size_t len;

	if (!fmt)
		return "";

	if (!args) {
		errno = EINVAL;
		return "";
	}

	len = vsnprintf(
		NULL, 0, fmt,
		*args); /* printf and co. return the number of bytes they _would_ write */
	if (len < 0)
		len = P_MAX_TXT_BUF;

	buf = (char *)calloc(len + 2, sizeof(char));
	if (!buf) {
		errno = ENOMEM;
		return "";
	}
	
	/* Copy the text */
	vsnprintf(buf, len + 1, fmt, *args);

	return buf;
}

char *purpl::fmt_text(const char *fmt, ...)
{
	char *buf;
	va_list args;

	va_start(args, fmt);
	buf = fmt_text_va(fmt, &args);
	va_end(args);

	return buf;
}

char *purpl::read_file_fp(FILE *fp)
{
	char *buf;
	size_t len;

	/* Check for a valid file pointer */
	if (!fp) {
		errno = EINVAL;
		return NULL;
	}

	/* Retrieve the file's length */
	fseek(fp, 0L, SEEK_END);
	len = ftell(fp);
	rewind(fp);

	/* Allocate a buffer for the file contents */
	buf = (char *)calloc(len + 2, sizeof(char));
	if (!buf) {
		errno = ENOMEM;
		return NULL;
	}

	/* Read in the file */
	fread(buf, sizeof(char), len, fp);
	if (!buf) {
		errno = EIO;
		return NULL;
	}

	/* Terminate the buffer's contents */
	buf[len + 1] = '\0';

	return buf;
}

char *purpl::read_file(const char *name)
{
	FILE *fp;

	if (!name) {
		errno = EINVAL;
		return NULL;
	}

	/* Open the file */
	fp = fopen(name, "rb");
	if (!fp) {
		errno = EIO;
		return NULL;
	}

	/* Read the file */
	return read_file_fp(fp);
}
