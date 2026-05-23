/* libwmf ("ipa/plot.h"): library for wmf conversion
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


#ifndef WMFIPA_PLOT_H
#define WMFIPA_PLOT_H

#include <plot.h>

typedef struct _plotPoint plotPoint;
typedef struct _plot_t    plot_t;

static plotPoint plot_translate (wmfAPI*,wmfD_Coord);

static float plot_width (wmfAPI*,float);
static float plot_height (wmfAPI*,float);

struct _plotPoint
{	int x;
	int y;
};

struct _plot_t
{	plPlotter* plotter;
	plPlotterParams* params;
};

#endif /* ! WMFIPA_PLOT_H */
