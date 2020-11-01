#include "purpl/util.h"

char *purpl::fmt_text_va(const char *fmt, va_list *args)
{
	va_list tmpargs;
	char *buf;
	size_t len;

	if (!fmt)
		return "";

	if (!args) {
		errno = EINVAL;
		return "";
	}

	va_copy(tmpargs, *args);

	len = vsnprintf(
		NULL, 0, fmt,
		tmpargs); /* printf and co. return the number of bytes they _would_ write */
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

char *purpl::read_file_fp(FILE *fp, uint *len_ret)
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

	if (len_ret)
		*len_ret = len + 1;

	return buf;
}

char *purpl::read_file(const char *name, uint *len_ret)
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
	return read_file_fp(fp, len_ret);
}
