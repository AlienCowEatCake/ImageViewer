/* libwmf ("ipa/plot/draw.h"): library for wmf conversion
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


static void wmf_plot_flood_interior (wmfAPI* API,wmfFlood_t* flood)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]flood_interior");

	
}

static void wmf_plot_flood_exterior (wmfAPI* API,wmfFlood_t* flood)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]flood_exterior");

	
}

static void wmf_plot_draw_pixel (wmfAPI* API,wmfDrawPixel_t* draw_pixel)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_pixel");

	
}

static void wmf_plot_draw_pie (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_pie");

	
}

static void wmf_plot_draw_chord (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_chord");

	
}

static void wmf_plot_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_arc");

	
}

static void wmf_plot_draw_ellipse (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_ellipse");

	
}

static void wmf_plot_draw_line (wmfAPI* API,wmfDrawLine_t* draw_line)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_line");

	
}

static void wmf_plot_poly_line (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]poly_line");

	
}

static void wmf_plot_draw_polygon (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_polygon");

	
}

static void wmf_plot_draw_rectangle (wmfAPI* API,wmfDrawRectangle_t* draw_rectangle)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_rectangle");

	
}
