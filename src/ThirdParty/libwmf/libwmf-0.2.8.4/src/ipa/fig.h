/* libwmf ("ipa/fig.h"): library for wmf conversion
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


#ifndef WMFIPA_FIG_H
#define WMFIPA_FIG_H

/*
 * FIG : Facility for Interactive Generation of figures
 * Copyright (c) 1985-1988 by Supoj Sutanthavibul
 * Parts Copyright (c) 1989-1998 by Brian V. Smith
 * Parts Copyright (c) 1991 by Paul King
 *
 * Any party obtaining a copy of these files is granted, free of charge, a
 * full and unrestricted irrevocable, world-wide, paid up, royalty-free,
 * nonexclusive right and license to deal in this software and
 * documentation files (the "Software"), including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons who receive
 * copies from any such party to do so, with the only requirement being
 * that this copyright notice remain intact.
 */

/* FIG format  object codes */

#define	O_COLOR_DEF	0
#define	O_ELLIPSE	1
#define	O_POLYLINE	2
#define	O_SPLINE	3
#define	O_TEXT		4
#define	O_ARC		5
#define	O_COMPOUND	6
#define	O_END_COMPOUND	-O_COMPOUND
#define	O_ALL_OBJECT	99

/* Subtypes for O_POLYLINE */

#define	T_POLYLINE	1
#define	T_BOX		2
#define	T_POLYGON	3
#define	T_ARC_BOX	4
#define	T_PICTURE	5

/* line types */

#define	SOLID_LINE			0
#define	DASH_LINE			1
#define	DOTTED_LINE			2
#define	DASH_DOT_LINE		3
#define	DASH_2_DOTS_LINE	4
#define	DASH_3_DOTS_LINE	5

/* Join styles */

#define	JOIN_MITER	0
#define	JOIN_ROUND	1
#define	JOIN_BEVEL	2

/* cap styles */

#define	CAP_BUTT	0
#define	CAP_ROUND	1
#define	CAP_PROJECT	2

/* number of patterns */

#define NUMPATTERNS	22

/* FIG license applies above */

typedef struct _fig_t    fig_t;
typedef struct _figPoint figPoint;
typedef struct _figDC    figDC;

struct _fig_t
{	void* equiv[NUMPATTERNS];
};

struct _figPoint
{	int x;
	int y;
};

struct _figDC
{	int pen_style;

	int thickness;

	int area_fill;

	int line_style;
	int join_style;
	int cap_style;

	int radius;

	int forward_arrow;
	int backward_arrow;

	int pen_color;
	int fill_color;

	float style_val;
};

/* fig/bmp.h
 */
static void fig_bmp_add (wmfAPI*,void*);
static int  fig_bmp_pattern (wmfAPI*,void*);
static void fig_bmp_remove (wmfAPI*,void*);

/* fig/color.h
 */
typedef struct _fig_color_t fig_color_t;

struct _fig_color_t
{	float r;
	float g;
	float b;
};

static void fig_std_colors (wmfAPI*);

static void fig_color_to_file (wmfAPI*,wmfStream*);

/* fig/device.h
 */
static figPoint fig_translate (wmfAPI*,wmfD_Coord);

static int fig_width  (wmfAPI*,float);
static int fig_height (wmfAPI*,float);

/* fig/draw.h
 */
static void fig_draw_arc (wmfAPI*,wmfDrawArc_t*,int);

/* fig/font.h
 */
typedef struct _PS_to_FIG PS_to_FIG;

struct _PS_to_FIG
{	char* PS_FontName;

	int FIG_FontNumber;
};

/* fig.c
 */
static int  fig_brushstyle (wmfAPI*,wmfDC*);
static int  fig_linestyle  (wmfAPI*,wmfDC*);
static int  fig_joinstyle  (wmfAPI*,wmfDC*);
static int  fig_capstyle   (wmfAPI*,wmfDC*);

static void fig_set_style  (wmfAPI*,wmfDC*,figDC*);

#endif /* ! WMFIPA_FIG_H */
