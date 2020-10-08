#pragma once

#ifndef PURPL_MACRO_H
#define PURPL_MACRO_H

#include <string.h>

#include "types.h"

/* Straightforward macros for getting the size of an array or buffer */
#define P_ARRAYSIZE(arr) (sizeof arr / sizeof arr[0])
#define P_BUFSIZE(buf, type) (sizeof buf / sizeof(type))

/*
 * Concatenates two values together by left shifting hi's value by the
 * size of type in bits then ORing with lo. Optimized through the use
 * of shifts and ORing instead of addition/multiplication.
 */
#define P_CONCAT(hi, lo, type, target) \
	((target)((hi) << (sizeof(type) << 3)) | (lo))

#ifdef _MSC_VER
#define P_EXPORT __declspec(dllexport)
#else
#define P_EXPORT
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

/*  */
#ifdef _WIN32
#define P_FILENAME \
	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define P_FILENAME \
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define P_MAX_TXT_BUF INT16_MAX

#endif /* !PURPL_MACRO_H */
