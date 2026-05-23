/* libwmf ("ipa/eps.h"): library for wmf conversion
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


#ifndef WMFIPA_EPS_H
#define WMFIPA_EPS_H

typedef enum
{	eps_arc_ellipse = 0,
	eps_arc_open,
	eps_arc_pie,
	eps_arc_chord
} eps_arc_t;

static void eps_draw_arc (wmfAPI*,wmfDrawArc_t*,eps_arc_t);

static void eps_path_fill (wmfAPI*,wmfDC*,wmfD_Rect*);
static void eps_path_stroke (wmfAPI*,wmfDC*,float);

static char* WMF_PS_DEFS = "";

static char* WMF_PS_HEAD = "\
save\n\
";

static char* WMF_PS_TAIL = "\
restore\n\
";

#endif /* ! WMFIPA_EPS_H */
