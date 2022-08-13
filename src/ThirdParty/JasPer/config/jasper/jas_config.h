#ifndef JAS_CONFIG_H
#define JAS_CONFIG_H

#if 0
#define _POSIX_C_SOURCE 200809L
#endif

#include <jasper/jas_compiler.h>
#include <jasper/jas_dll.h>

/* This preprocessor symbol identifies the version of JasPer. */

/*!
@brief
A null-terminated string containing the JasPer library version
*/
#define	JAS_VERSION "3.0.6"

/*!
@brief
The JasPer library major version number.
*/
#define JAS_VERSION_MAJOR 3

/*!
@brief
The JasPer library minor version number.
*/
#define JAS_VERSION_MINOR 0

/*!
@brief
The JasPer library patch version number.
*/
#define JAS_VERSION_PATCH 6

/*
The version of the C standard against which JasPer was built.
*/
#define JAS_STDC_VERSION (__STDC_VERSION__ - 0)

/* #undef JAS_ENABLE_32BIT */

#define JAS_SIZEOF_INT 4
#define JAS_SIZEOF_LONG 4
#define JAS_SIZEOF_LLONG 4
#define JAS_SIZEOF_SIZE_T 4
#define JAS_SIZEOF_SSIZE_T 4

#define JAS_HAVE_FCNTL_H
#if defined(_MSC_VER)
#define JAS_HAVE_IO_H
/* #undef JAS_HAVE_UNISTD_H */
/* #undef JAS_HAVE_SYS_TIME_H */
#else
/* #undef JAS_HAVE_IO_H */
#define JAS_HAVE_UNISTD_H
#define JAS_HAVE_SYS_TIME_H
#endif
#define JAS_HAVE_SYS_TYPES_H
/* #undef JAS_HAVE_SSIZE_T */
/* #undef JAS_HAVE_MAX_ALIGN_T */

#if 0
/* #undef JAS_HAVE_UINTMAX_T */
/* #undef JAS_HAVE_INTMAX_T */
#endif

/* #undef JAS_HAVE_MKOSTEMP */
#if defined(_MSC_VER)
/* #undef JAS_HAVE_GETTIMEOFDAY */
/* #undef JAS_HAVE_GETRUSAGE */
#else
#define JAS_HAVE_GETTIMEOFDAY
#define JAS_HAVE_GETRUSAGE
#endif
/* #undef JAS_HAVE_NANOSLEEP */

/* #undef JAS_HAVE_GL_GLUT_H */
/* #undef JAS_HAVE_GLUT_GLUT_H */
/* #undef JAS_HAVE_GLUT_H */

#define JAS_INCLUDE_PNM_CODEC
#define JAS_INCLUDE_BMP_CODEC
#define JAS_INCLUDE_RAS_CODEC
#define JAS_INCLUDE_JP2_CODEC
#define JAS_INCLUDE_JPC_CODEC
#if defined(HAS_LIBJPEG)
#define JAS_INCLUDE_JPG_CODEC
#endif
#if defined(HAS_LIBHEIF)
#define JAS_INCLUDE_HEIC_CODEC
#endif
#define JAS_INCLUDE_PGX_CODEC
#define JAS_INCLUDE_MIF_CODEC
#define JAS_ENABLE_DANGEROUS_INTERNAL_TESTING_MODE

#define JAS_ENABLE_PNM_CODEC 1
#define JAS_ENABLE_BMP_CODEC 1
#define JAS_ENABLE_RAS_CODEC 1
#define JAS_ENABLE_JP2_CODEC 1
#define JAS_ENABLE_JPC_CODEC 1
#if defined(HAS_LIBJPEG)
#define JAS_ENABLE_JPG_CODEC 1
#else
#define JAS_ENABLE_JPG_CODEC 0
#endif
#if defined(HAS_LIBHEIF)
#define JAS_ENABLE_HEIC_CODEC 1
#else
#define JAS_ENABLE_HEIC_CODEC 0
#endif
#define JAS_ENABLE_PGX_CODEC 1
#define JAS_ENABLE_MIF_CODEC 1

#define JAS_DEFAULT_MAX_MEM_USAGE 1073741824

/* #undef JAS_THREADS */
/* #undef JAS_THREADS_C11 */
/* #undef JAS_THREADS_PTHREAD */
/* #undef JAS_THREADS_WIN32 */

/* #undef JAS_HAVE_THREAD_LOCAL */

/* #undef JAS_ENABLE_NON_THREAD_SAFE_DEBUGGING */

#ifdef _MSC_VER
/* #undef JAS_HAVE_SNPRINTF */
#else
#define JAS_HAVE_SNPRINTF
#endif

#if !defined(JAS_DEC_DEFAULT_MAX_SAMPLES)
#define JAS_DEC_DEFAULT_MAX_SAMPLES (64 * ((size_t) 1048576))
#endif

#if (JAS_DEFAULT_MAX_MEM_USAGE == 0)
#define JAS_DEFAULT_MAX_MEM_USAGE (1024ULL * 1024ULL * 1024ULL)
#endif

#if (__STDC_VERSION__ - 0 < JAS_STDC_VERSION)
#warning "Your code is being built against an older version of the C standard than JasPer was.  Although this is supported, this may require some extra preprocessor defines when building."
#endif

#endif
