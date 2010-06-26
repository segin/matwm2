/*
 * $Header: /cvsroot/overdose/overdose/src/snprintf.h,v 1.1 2006/11/11 09:43:57 lesleyb Exp $
 */
#ifndef _PORTABLE_SNPRINTF_H_
#define _PORTABLE_SNPRINTF_H_

#define PORTABLE_SNPRINTF_VERSION_MAJOR 2
#define PORTABLE_SNPRINTF_VERSION_MINOR 2

#ifdef HAVE_SNPRINTF
#include <stdio.h>
#else
int snprintf(char *, size_t, const char *, /*args*/ ...);
int vsnprintf(char *, size_t, const char *, va_list);
#endif

#if defined(HAVE_SNPRINTF) && defined(PREFER_PORTABLE_SNPRINTF)
int portable_snprintf(char *str, size_t str_m, const char *fmt, /*args*/ ...);
int portable_vsnprintf(char *str, size_t str_m, const char *fmt, va_list ap);
#define snprintf  portable_snprintf
#define vsnprintf portable_vsnprintf
#endif

typedef int    gint;
typedef char   gchar;
gint vasprintf (gchar **string, gchar const *format, va_list args);
int vasnprintf(char **ptr, size_t str_m, const char *fmt, va_list ap);

int asprintf  (char **ptr, const char *fmt, /*args*/ ...);
/* gint vasprintf (gchar **string, gchar const *format, va_list args); */
int asnprintf (char **ptr, size_t str_m, const char *fmt, /*args*/ ...);
/* int vasnprintf(char **ptr, size_t str_m, const char *fmt, va_list ap); */

#endif
