#include "purpl/util.h"

char *purpl::fmt_text_va(const char *fmt, va_list *args)
{
	char *buf;
	size_t len;

	if (!fmt)
		return "";
	
	if (!args) {
		errno = EINVAL;
		return NULL;
	}

	len = vsnprintf(NULL, 0, fmt, *args); /* printf and co., return the number of bytes they _would_ write */
	if (len < 0)
		len = P_MAX_TXT_BUF;

	buf = (char *)calloc(len + 2, sizeof(char));
	if (!buf) {
		errno = ENOMEM;
		return NULL;
	}

	/* calloc technically already does this */
	memset(buf, '\0', sizeof(char));
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
