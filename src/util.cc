#include "purpl/util.h"

/*
 * Because we use these functions here, say
 * that the implementations should be put here
 */
#define STB_SPRINTF_IMPLEMENTATION
#include <stb_sprintf.h>

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

	len = stbsp_vsnprintf(
		NULL, 0, fmt,
		tmpargs); /* printf and co. return the number of bytes they _would_ write */
	if (len < 0)
		len = P_MAX_TXT_BUF; /* This is a failsafe because this function is used everywhere */

	buf = (char *)calloc(len + 2, sizeof(char));
	if (!buf) {
		errno = ENOMEM;
		return "";
	}

	/* Copy the text */
	stbsp_vsnprintf(buf, len + 1, fmt, *args);

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

int purpl::map_file(FILE *fp, size_t *len_ret, char **buf)
{
	char *mapped;
	int fd;
	size_t len;

	/* Fail if we don't have the stuff we need */
	if (!fp || !len_ret || !buf)
		return EINVAL;

	/* Get the file length */
	fseek(fp, SEEK_END, 0);
	len = ftell(fp);
	rewind(fp);

	/*
	 * For the mapping process, we need a file descriptor, which is, somehow,
	 * both platform independent and always necessary, which was unexpected.
	 */
	fd = fileno(fp);

	/*
	 * This next bit deals with the fact that this involves a system call type of thing.
	 * The process is to get the length and a file descriptor/handle from fp,
	 * then do the system call to map the file here.
	 */
#ifdef _WIN32
	/* The variables that we need for the Win32 API are here */
	HANDLE file;
	HANDLE mapping;

	/* Get a Win32 file handle from the file descriptor we got earlier */
	file = (HANDLE)_get_osfhandle(fd);
	if (!file)
		return EIO;

	/* Create the mapping for the file */
	mapping = CreateFileMappingA(file, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (GetLastError())
		return EIO;

	/* Map the file view */
	mapped = (char *)MapViewOfFile(mapping, FILE_MAP_WRITE, 0, 0, len);
#elif __linux__
	/* This is way simpler than Win32 */
	mapped = (char *)mmap(NULL, len, PROT_WRITE, MAP_SHARED, fd, 0);
	if (!mapped)
		return EIO;
#endif

	/* Pass out the starting addres of the mapped pages and the file length */
	*buf = mapped;
	*len_ret = len;

	return 0;
}

int purpl::unmap_file(char *buf, size_t len)
{
#ifdef _WIN32
	UnmapViewOfFile(buf);
	return (int)GetLastError();
#elif __linux__
	return munmap(buf, len);
#endif
}

char *purpl::load_file_fp(FILE *fp, bool map, size_t *len_ret)
{
	char *buf;
	size_t len = 0;

	/* Check for a valid file pointer */
	if (!fp) {
		errno = EINVAL;
		return NULL;
	}

	if (map) {
		/* In this case, the caller has to save the length for unmapping the file */
		if (!len_ret) {
			errno = EINVAL;
			return NULL;
		}

		/* Map the file */
		map_file(fp, len_ret, &buf);
	}
	
	/* This is also a fallback */
	if (!map || !buf) {
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
	}

	if (!buf) {
		errno = EIO;
		return NULL;
	}

	/* Terminate the buffer's contents */
	buf[len + 1] = '\0';
	
	if (len_ret)
		*len_ret = len;
	
	return buf;
}

char *purpl::load_file(const char *name, bool map, size_t *len_ret)
{
	FILE *fp;

	if (!name) {
		errno = EINVAL;
		return NULL;
	}

	/* Open the file */
	fp = fopen(name, "rb+");
	if (!fp) {
		errno = EIO;
		return NULL;
	}

	/* Read the file */
	return load_file_fp(fp, map, len_ret);
}
