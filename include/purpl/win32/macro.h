#pragma once

#ifndef PURPL_WIN32_MACRO_H
#define PURPL_WIN32_MACRO_H

#define P_WIN32_WINDOW_TEXT_MAX 90

/* Tell Win32 not to use Unicode functions, for simplicity. If Unicode-dependent translations are ever needed, I guess we'll have to rewrite a lot. */
#ifdef UNICODE
#undef UNICODE
#endif /* !UNICODE */

#endif /* !PURPL_WIN32_MACRO_H */
