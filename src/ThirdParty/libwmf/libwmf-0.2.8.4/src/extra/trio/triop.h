/*************************************************************************
 *
 * $Id: triop.h,v 1.1 2001/06/07 08:23:02 fjfranklin Exp $
 *
 * Copyright (C) 2000 Bjorn Reese and Daniel Stenberg.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE AUTHORS AND
 * CONTRIBUTORS ACCEPT NO RESPONSIBILITY IN ANY CONCEIVABLE MANNER.
 *
 ************************************************************************
 *
 * Private functions, types, etc. used for callback functions.
 *
 * The ref pointer is an opaque type and should remain as such.
 * Private data must only be accessible through the getter and
 * setter functions.
 *
 ************************************************************************/

#ifndef TRIO_TRIOP_H
#define TRIO_TRIOP_H

#if defined(__STDC__) && (__STDC_VERSION__ >= 199901L)
# define TRIO_C99
#endif
#define TRIO_BSD
#define TRIO_GNU
#define TRIO_MISC
#define TRIO_UNIX98
#define TRIO_EXTENSION
#define TRIO_ERRORS


typedef int (*trio_callback_t)(void *ref);

void *trio_register(trio_callback_t callback, const char *name);
void trio_unregister(void *handle);

const char *trio_get_format(void *ref);
void *trio_get_argument(void *ref);

/* Modifiers */
int  trio_get_width(void *ref);
void trio_set_width(void *ref, int width);
int  trio_get_precision(void *ref);
void trio_set_precision(void *ref, int precision);
int  trio_get_base(void *ref);
void trio_set_base(void *ref, int base);
int  trio_get_padding(void *ref);
void trio_set_padding(void *ref, int is_padding);
int  trio_get_short(void *ref); /* h */
void trio_set_shortshort(void *ref, int is_shortshort);
int  trio_get_shortshort(void *ref); /* hh */
void trio_set_short(void *ref, int is_short);
int  trio_get_long(void *ref); /* l */
void trio_set_long(void *ref, int is_long);
int  trio_get_longlong(void *ref); /* ll */
void trio_set_longlong(void *ref, int is_longlong);
int  trio_get_longdouble(void *ref); /* L */
void trio_set_longdouble(void *ref, int is_longdouble);
int  trio_get_alternative(void *ref); /* # */
void trio_set_alternative(void *ref, int is_alternative);
int  trio_get_alignment(void *ref); /* - */
void trio_set_alignment(void *ref, int is_leftaligned);
int  trio_get_spacing(void *ref); /* (space) */
void trio_set_spacing(void *ref, int is_space);
int  trio_get_sign(void *ref); /* + */
void trio_set_sign(void *ref, int is_showsign);
int  trio_get_quote(void *ref); /* ' */
void trio_set_quote(void *ref, int is_quote);
int  trio_get_upper(void *ref);
void trio_set_upper(void *ref, int is_upper);
#if defined(TRIO_C99)
int  trio_get_largest(void *ref); /* j */
void trio_set_largest(void *ref, int is_largest);
int  trio_get_ptrdiff(void *ref); /* t */
void trio_set_ptrdiff(void *ref, int is_ptrdiff);
int  trio_get_size(void *ref); /* z / Z */
void trio_set_size(void *ref, int is_size);
#endif

/* Printing */
int trio_print_ref(void *ref, const char *format, ...);
int trio_vprint_ref(void *ref, const char *format, va_list args);
int trio_printv_ref(void *ref, const char *format, void **args);

void trio_print_int(void *ref, int number);
void trio_print_uint(void *ref, unsigned int number);
/*  void trio_print_long(void *ref, long number); */
/*  void trio_print_ulong(void *ref, unsigned long number); */
void trio_print_double(void *ref, double number);
void trio_print_string(void *ref, char *string);
void trio_print_pointer(void *ref, void *pointer);

#endif /* TRIO_TRIOP_H */
