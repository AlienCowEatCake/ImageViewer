/* wmfconfig.h.  Generated from wmfconfig.h.in by configure.  */
/* wmfconfig.h.in.  Generated from configure.ac by autoheader.  */

#ifndef LIBWMF_CONFIG_H
#define LIBWMF_CONFIG_H

/* Define if you have expat.  */
#if defined (HAS_LIBEXPAT) && !defined (HAVE_EXPAT)
#define HAVE_EXPAT 1
#endif

/* Define if you have libxml2.  */
/* #undef HAVE_LIBXML2 */

/* Define if you have libpng.  */
#if defined (HAS_LIBPNG) && !defined (HAVE_LIBPNG)
#define HAVE_LIBPNG 1
#endif

/* Define if you have libjpeg.  */
#if defined (HAS_LIBJPEG) && !defined (HAVE_LIBJPEG)
#define HAVE_LIBJPEG 1
#endif

/* defining this will disable egs, fig, svg & foreign layers */
/* #undef WITHOUT_LAYERS */

/* define if you have libgd */
#define HAVE_GD 1

/* define if you wish to build the magick device layer. */
/* #undef ENABLE_MAGICK */

/* Define if you have GNU plotutils (libplot) >= 2.4.0.  */
/* #undef HAVE_LIBPLOT */

/* Define if you have the <unistd.h> header file.  */
#if !defined (_WIN32) && !defined (HAVE_UNISTD_H)
#define HAVE_UNISTD_H 1
#endif

/* define if you have strstr */
#define HAVE_STRSTR 1

/* define if you have [_]vsnprintf */
#define HAVE_VSNPRINTF 1

/* Define to `_vsnprintf' if necessary */
/* #undef vsnprintf */

/* define if you have [_]snprintf */
#define HAVE_SNPRINTF 1

/* Define to `_snprintf' if necessary */
/* #undef snprintf */

/* define if you have vfscanf */
#define HAVE_VFSCANF 1

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define to `long' if <sys/types.h> does not define. */
/* #undef off_t */


/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `fprintf' function. */
#define HAVE_FPRINTF 1

/* Define to 1 if you have the `fscanf' function. */
#define HAVE_FSCANF 1

/* Define to 1 if you have the <inttypes.h> header file. */
/* #undef HAVE_INTTYPES_H */

/* Define to 1 if you have the <memory.h> header file. */
/* #undef HAVE_MEMORY_H */

/* Define to 1 if you have the `printf' function. */
#define HAVE_PRINTF 1

/* Define to 1 if you have the `scanf' function. */
#define HAVE_SCANF 1

/* Define to 1 if you have the `snprintf' function. */
#define HAVE_SNPRINTF 1

/* Define to 1 if you have the `sprintf' function. */
#define HAVE_SPRINTF 1

/* Define to 1 if you have the `sscanf' function. */
#define HAVE_SSCANF 1

/* Define to 1 if you have the <stdint.h> header file. */
/* #undef HAVE_STDINT_H */

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
/* #undef HAVE_STRINGS_H */

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the `strstr' function. */
#define HAVE_STRSTR 1

/* Define to 1 if you have the <sys/stat.h> header file. */
/* #undef HAVE_SYS_STAT_H */

/* Define to 1 if you have the <sys/types.h> header file. */
/* #undef HAVE_SYS_TYPES_H */

/* Define to 1 if you have the <time.h> header file. */
#define HAVE_TIME_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#if !defined (_WIN32) && !defined (HAVE_UNISTD_H)
#define HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `vfprintf' function. */
#define HAVE_VFPRINTF 1

/* Define to 1 if you have the `vfscanf' function. */
#define HAVE_VFSCANF 1

/* Define to 1 if you have the `vprintf' function. */
#define HAVE_VPRINTF 1

/* Define to 1 if you have the `vscanf' function. */
#define HAVE_VSCANF 1

/* Define to 1 if you have the `vsnprintf' function. */
#define HAVE_VSNPRINTF 1

/* Define to 1 if you have the `vsprintf' function. */
#define HAVE_VSPRINTF 1

/* Define to 1 if you have the `vsscanf' function. */
#define HAVE_VSSCANF 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ""

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME ""

/* Define to the full name and version of this package. */
#define PACKAGE_STRING ""

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME ""

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION ""

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Define to 1 if the X Window System is missing or not being used. */
#define X_DISPLAY_MISSING 1

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `long int' if <sys/types.h> does not define. */
/* #undef off_t */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* If any *printf / *scanf functions are missing, cover them with trio functions */
/* #undef TRIO_REPLACE_STDIO */

/* custom fonts provider */
#include "fontsprovider/fontsprovider.h"
#define WMF_FONTDIR         ProvideWmfFontdir()
#define WMF_GS_FONTDIR      ProvideWmfGsFontdir()
#define WMF_SYS_FONTMAP     ProvideWmfSysFontmap()
#define WMF_XTRA_FONTMAP    ProvideWmfXtraFontmap()
#define WMF_GS_FONTMAP      ProvideWmfGsFontmap()

#endif /* ! LIBWMF_CONFIG_H */
