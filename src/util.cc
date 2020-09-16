#include <purpl/util.h>

wchar_t *purpl::fmt_text_va(const wchar_t *fmt, va_list *args)
{
	wchar_t *buf;
	int len;
	
	if (!args) {
		errno = EINVAL;
		return NULL;
	}

	len = vsnwprintf(NULL, 0, fmt, *args); /* snprintf returns the number of bytes it _would_ write */
	if (len < 0)
		len = P_MAX_TXT_BUF;

	buf = (wchar_t *)calloc(len + 1, sizeof(wchar_t));
	if (!buf) {
		errno = ENOMEM;
		return NULL;
	}

	vsnwprintf(buf, len + 1, fmt, *args);

	return buf;
}

wchar_t *purpl::fmt_text(const wchar_t *fmt, ...)
{
	wchar_t *buf;
	va_list args;

	va_start(args, fmt);
	buf = fmt_text_va(fmt, &args);
	va_end(args);

	return buf;
}
