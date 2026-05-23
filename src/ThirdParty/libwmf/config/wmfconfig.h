/* wmfconfig.h.  Generated from wmfconfig.h.in by configure.  */
/* wmfconfig.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Use expat as libwmf_xml */
#if defined (HAS_LIBEXPAT) && !defined (HAVE_EXPAT)
#define HAVE_EXPAT 1
#endif

/* Define to 1 if you have the 'fprintf' function. */
#define HAVE_FPRINTF 1

/* Define to 1 if you have the 'fscanf' function. */
#define HAVE_FSCANF 1

/* Library gd is available */
#define HAVE_GD 1

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Library libjpeg is available */
#if defined (HAS_LIBJPEG) && !defined (HAVE_LIBJPEG)
#define HAVE_LIBJPEG 1
#endif

/* Library libplot is available */
/* #undef HAVE_LIBPLOT */

/* Library libpng is available */
#if defined (HAS_LIBPNG) && !defined (HAVE_LIBPNG)
#define HAVE_LIBPNG 1
#endif

/* Use libxml2 as libwmf_xml */
/* #undef HAVE_LIBXML2 */

/* Define to 1 if you have the 'printf' function. */
#define HAVE_PRINTF 1

/* Define to 1 if you have the 'scanf' function. */
#define HAVE_SCANF 1

/* Function _snprintf is available */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the 'sprintf' function. */
#define HAVE_SPRINTF 1

/* Define to 1 if you have the 'sscanf' function. */
#define HAVE_SSCANF 1

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdio.h> header file. */
#define HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the 'strstr' function. */
#define HAVE_STRSTR 1

/* Build against system libgd */
/* #undef HAVE_SYS_GD */

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/types.h> header file. */
/* #undef HAVE_SYS_TYPES_H */

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Header unistd.h is available */
#if !defined (_WIN32) && !defined (HAVE_UNISTD_H)
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the 'vfprintf' function. */
#define HAVE_VFPRINTF 1

/* Function vfscanf is available */
#define HAVE_VFSCANF 1

/* Define to 1 if you have the 'vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the 'vscanf' function. */
#define HAVE_VSCANF 1

/* Function vsnprintf is available */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the 'vsprintf' function. */
#define HAVE_VSPRINTF 1

/* Define to 1 if you have the 'vsscanf' function. */
#define HAVE_VSSCANF 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ""

/* Name of package */
#define PACKAGE "libwmf"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "libwmf"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libwmf 0.2.15"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libwmf"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "0.2.15"

/* Define to 1 if all of the C89 standard headers exist (not just the ones
   required in a freestanding environment). This macro is provided for
   backward compatibility; new code need not use it. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION "0.2.15"

/* Don't use layers */
/* #undef WITHOUT_LAYERS */

/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1

/* Define to empty if 'const' does not conform to ANSI C. */
/* #undef const */

/* Define to 'long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define as 'unsigned int' if <stddef.h> doesn't define. */
/* #undef size_t */

/* Use _snprintf instead of snprintf */
/* #undef snprintf */

/* Use _vsnprintf instead of vsnprintf */
/* #undef vsnprintf */

/* custom fonts provider */
#include "fontsprovider/fontsprovider.h"
#define WMF_FONTDIR         ProvideWmfFontdir()
#define WMF_GS_FONTDIR      ProvideWmfGsFontdir()
#define WMF_SYS_FONTMAP     ProvideWmfSysFontmap()
#define WMF_XTRA_FONTMAP    ProvideWmfXtraFontmap()
#define WMF_GS_FONTMAP      ProvideWmfGsFontmap()
