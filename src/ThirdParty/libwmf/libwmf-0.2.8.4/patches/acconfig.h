
#ifndef LIBWMF_CONFIG_H
#define LIBWMF_CONFIG_H
@TOP@

/* Define if you have expat.  */
#undef HAVE_EXPAT

/* Define if you have libxml2.  */
#undef HAVE_LIBXML2

/* Define if you have libpng.  */
#undef HAVE_LIBPNG

/* Define if you have libjpeg.  */
#undef HAVE_LIBJPEG

/* defining this will disable egs, fig, svg & foreign layers */
#undef WITHOUT_LAYERS

/* define if you have libgd */
#undef HAVE_GD

/* define if you wish to build the magick device layer. */
#undef ENABLE_MAGICK

/* Define if you have GNU plotutils (libplot) >= 2.4.0.  */
#undef HAVE_LIBPLOT

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

/* define if you have strstr */
#undef HAVE_STRSTR

/* define if you have [_]vsnprintf */
#undef HAVE_VSNPRINTF

/* Define to `_vsnprintf' if necessary */
#undef vsnprintf

/* define if you have [_]snprintf */
#undef HAVE_SNPRINTF

/* Define to `_snprintf' if necessary */
#undef snprintf

/* define if you have vfscanf */
#undef HAVE_VFSCANF

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
#undef size_t

/* Define to `long' if <sys/types.h> does not define. */
#undef off_t

@BOTTOM@

/* If any *printf/*scanf functions are missing, cover them with trio functions */
#define TRIO_REPLACE_STDIO 1

#endif /* ! LIBWMF_CONFIG_H */
