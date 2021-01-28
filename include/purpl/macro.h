#pragma once

#ifndef PURPL_MACRO_H
#define PURPL_MACRO_H

#include <string.h>

#include "types.h"

/* Straightforward macro for getting the size of an array */
#define P_ARRAYSIZE(arr) (sizeof(arr) / sizeof(arr[0]))

/*
 * Concatenates two values together by left shifting hi's value by the
 * size of type in bits then ORing with lo. Optimized through the use
 * of shifts and ORing instead of addition/multiplication.
 */
#define P_CONCAT(hi, lo, type, target) \
	((target)((hi) << (sizeof(type) << 3)) | (lo))

/* Gets the higher half of a value */
#define P_HIGH(val, target) ((target)((val) >> (sizeof(val) << 3)))

/* Gets the lower half of a value */
#define P_LOW(val, target) ((target)((val) & ((1 << (sizeof(val) << 3)) - 1)))

#ifdef _MSC_VER
#define P_EXPORT __declspec(dllexport)
#else
#define P_EXPORT
#endif

#ifdef _MSC_VER
#pragma warning(disable : 4820)
#pragma warning(disable : 4996)
#pragma warning(disable : 26452)
#pragma warning(disable : 26495)
#pragma warning(disable : 26812)
#endif

/* The base name and extension of the current file. */
#if _WIN32 && \
	_MSC_VER /* MSVC is the only Windows compiler that uses backslashes in __FILE__ */
#define P_FILENAME \
	(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define P_FILENAME \
	(strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define P_MAX_TXT_BUF INT16_MAX

#endif /* !PURPL_MACRO_H */
