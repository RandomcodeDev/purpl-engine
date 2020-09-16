#pragma once

#ifndef PURPL_MACRO_H
#define PURPL_MACRO_H

#include <string.h>
#include <wchar.h>

#include "types.h"

#define P_ARRAYSIZE(arr) (sizeof arr / sizeof arr[0])
#define P_BUFSIZE(buf, type) (sizeof buf / sizeof(type))

#define P_CONCAT(hi, lo, type, target) ((target)hi << sizeof(type) | lo)

#ifdef _MSC_VER
#define P_EXPORT __declspec(dllexport)
#else
#define P_EXPORT
#endif

#ifdef _WIN32
#define P_FILENAME \
	(wcsrchr(__FILEW__, L'\\') ? wcsrchr(__FILEW__, L'\\') + 1 : __FILEW__)
#else
#define P_FILENAME \
	(wcsrchr(__FILEW__, L'/') ? wcsrchr(__FILEW__, L'/') + 1 : __FILEW__)
#endif

#define P_MAX_TXT_BUF INT16_MAX

#endif /* !PURPL_MACRO_H */
