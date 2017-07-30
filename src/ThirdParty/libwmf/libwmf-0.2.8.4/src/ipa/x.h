/* libwmf ("ipa/x.h"): library for wmf conversion
   Copyright (C) 2000 - various; see CREDITS, ChangeLog, and sources

   The libwmf Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The libwmf Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the libwmf Library; see the file COPYING.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */


#ifndef WMFIPA_X_H
#define WMFIPA_X_H

/* ROP code to GC function conversion
 */
static int Our_XROPfunction[16] =
{	GXclear,        /* R2_BLACK */
	GXnor,          /* R2_NOTMERGEPEN */
	GXandInverted,  /* R2_MASKNOTPEN */
	GXcopyInverted, /* R2_NOTCOPYPEN */
	GXandReverse,   /* R2_MASKPENNOT */
	GXinvert,       /* R2_NOT */
	GXxor,          /* R2_XORPEN */
	GXnand,         /* R2_NOTMASKPEN */
	GXand,          /* R2_MASKPEN */
	GXequiv,        /* R2_NOTXORPEN */
	GXnoop,         /* R2_NOP */
	GXorInverted,   /* R2_MERGENOTPEN */
	GXcopy,         /* R2_COPYPEN */
	GXorReverse,    /* R2_MERGEPENNOT */
	GXor,           /* R2_MERGEPEN */
	GXset           /* R2_WHITE */
};

#define NB_HATCH_STYLES  6

static unsigned char HatchBrushes[NB_HATCH_STYLES + 1][8] =
{	{ 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00 }, /* HS_HORIZONTAL */
	{ 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08 }, /* HS_VERTICAL   */
	{ 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 }, /* HS_FDIAGONAL  */
	{ 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 }, /* HS_BDIAGONAL  */
	{ 0x08, 0x08, 0x08, 0xff, 0x08, 0x08, 0x08, 0x08 }, /* HS_CROSS      */
	{ 0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81 }, /* HS_DIAGCROSS  */
	{ 0xee, 0xbb, 0xee, 0xbb, 0xee, 0xbb, 0xee, 0xbb }  /* Hack for DKGRAY */
};

static char PEN_dash[]       = { 5,3 };         /* -----   -----   -----  */
static char PEN_dot[]        = { 2,2 };         /* --  --  --  --  --  -- */
static char PEN_dashdot[]    = { 4,3,2,3 };     /* ----   --   ----   --  */
static char PEN_dashdotdot[] = { 4,2,2,2,2,2 }; /* ----  --  --  ----  -- */
static char PEN_alternate[]  = { 1,1 };         /* FIXME */

static void setbrushstyle (wmfAPI*,wmfDC*);
static void setlinestyle (wmfAPI*,wmfDC*);
static void setdefaultstyle (wmfAPI*);

/* device.h
 */
static XPoint x_translate (wmfAPI*,wmfD_Coord);
static XPoint x_translate_ft64 (wmfAPI*,wmfD_Coord,FT_Vector*);

static float x_width (wmfAPI*,float);
static float x_height (wmfAPI*,float);

/* color.h
 */
static void setup_color (wmfAPI*);

static unsigned long get_color (wmfAPI*,wmfRGB*);

/* draw.h
 */
typedef enum
{	x_arc_ellipse = 0,
	x_arc_open,
	x_arc_pie,
	x_arc_chord
} x_arc_t;

static void x_draw_arc (wmfAPI*,wmfDrawArc_t*,x_arc_t);

/* font.h
 */
static void x_draw_ftbitmap (wmfAPI*,FT_Bitmap*,XPoint,wmfRGB*,wmfRGB*);

#endif /* ! WMFIPA_X_H */
