/* libwmf (ipa/plot.c): library for wmf conversion
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


#ifdef HAVE_CONFIG_H
#include "wmfconfig.h"
#endif /* HAVE_CONFIG_H */

#include <math.h>

#include "wmfdefs.h"

#ifdef HAVE_LIBPLOT

#include "libwmf/plot.h"

static void wmf_plot_device_open (wmfAPI*);
static void wmf_plot_device_close (wmfAPI*);
static void wmf_plot_device_begin (wmfAPI*);
static void wmf_plot_device_end (wmfAPI*);
static void wmf_plot_flood_interior (wmfAPI*,wmfFlood_t*);
static void wmf_plot_flood_exterior (wmfAPI*,wmfFlood_t*);
static void wmf_plot_draw_pixel (wmfAPI*,wmfDrawPixel_t*);
static void wmf_plot_draw_pie (wmfAPI*,wmfDrawArc_t*);
static void wmf_plot_draw_chord (wmfAPI*,wmfDrawArc_t*);
static void wmf_plot_draw_arc (wmfAPI*,wmfDrawArc_t*);
static void wmf_plot_draw_ellipse (wmfAPI*,wmfDrawArc_t*);
static void wmf_plot_draw_line (wmfAPI*,wmfDrawLine_t*);
static void wmf_plot_poly_line (wmfAPI*,wmfPolyLine_t*);
static void wmf_plot_draw_polygon (wmfAPI*,wmfPolyLine_t*);
static void wmf_plot_draw_rectangle (wmfAPI*,wmfDrawRectangle_t*);
static void wmf_plot_rop_draw (wmfAPI*,wmfROP_Draw_t*);
static void wmf_plot_bmp_draw (wmfAPI*,wmfBMP_Draw_t*);
static void wmf_plot_bmp_read (wmfAPI*,wmfBMP_Read_t*);
static void wmf_plot_bmp_free (wmfAPI*,wmfBMP*);
static void wmf_plot_draw_text (wmfAPI*,wmfDrawText_t*);
static void wmf_plot_udata_init (wmfAPI*,wmfUserData_t*);
static void wmf_plot_udata_copy (wmfAPI*,wmfUserData_t*);
static void wmf_plot_udata_set (wmfAPI*,wmfUserData_t*);
static void wmf_plot_udata_free (wmfAPI*,wmfUserData_t*);
static void wmf_plot_region_frame (wmfAPI*,wmfPolyRectangle_t*);
static void wmf_plot_region_paint (wmfAPI*,wmfPolyRectangle_t*);
static void wmf_plot_region_clip (wmfAPI*,wmfPolyRectangle_t*);

#include "ipa/plot.h"
#include "ipa/plot/bmp.h"
#include "ipa/plot/device.h"
#include "ipa/plot/draw.h"
#include "ipa/plot/region.h"

#endif /* HAVE_LIBPLOT */

void wmf_plot_function (wmfAPI* API)
{
#ifdef HAVE_LIBPLOT
	wmf_plot_t* ddata = 0;

	wmfFunctionReference* FR = (wmfFunctionReference*) API->function_reference;

	plot_t* plot = 0;

	if ((API->flags & API_STANDARD_INTERFACE) == 0)
	{	WMF_ERROR (API,"Can't use this device layer with 'lite' interface!");
		API->err = wmf_E_DeviceError;
		return;
	}

/* IPA function reference links
 */
	FR->device_open    = wmf_plot_device_open;
	FR->device_close   = wmf_plot_device_close;
	FR->device_begin   = wmf_plot_device_begin;
	FR->device_end     = wmf_plot_device_end;
	FR->flood_interior = wmf_plot_flood_interior;
	FR->flood_exterior = wmf_plot_flood_exterior;
	FR->draw_pixel     = wmf_plot_draw_pixel;
	FR->draw_pie       = wmf_plot_draw_pie;
	FR->draw_chord     = wmf_plot_draw_chord;
	FR->draw_arc       = wmf_plot_draw_arc;
	FR->draw_ellipse   = wmf_plot_draw_ellipse;
	FR->draw_line      = wmf_plot_draw_line;
	FR->poly_line      = wmf_plot_poly_line;
	FR->draw_polygon   = wmf_plot_draw_polygon;
	FR->draw_rectangle = wmf_plot_draw_rectangle;
	FR->rop_draw       = wmf_plot_rop_draw;
	FR->bmp_draw       = wmf_plot_bmp_draw;
	FR->bmp_read       = wmf_plot_bmp_read;
	FR->bmp_free       = wmf_plot_bmp_free;
	FR->draw_text      = wmf_plot_draw_text;
	FR->udata_init     = wmf_plot_udata_init;
	FR->udata_copy     = wmf_plot_udata_copy;
	FR->udata_set      = wmf_plot_udata_set;
	FR->udata_free     = wmf_plot_udata_free;
	FR->region_frame   = wmf_plot_region_frame;
	FR->region_paint   = wmf_plot_region_paint;
	FR->region_clip    = wmf_plot_region_clip;

/* Allocate device data structure
 */
	ddata = (wmf_plot_t*) wmf_malloc (API,sizeof (wmf_plot_t));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	API->device_data = (void*) ddata;

/* Device data defaults
 */
	ddata->type = wmf_plot_generic;

	ddata->plot_data = (plot_t*) wmf_malloc (API,sizeof (plot_t));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	plot = (plot_t*) ddata->plot_data;

	plot->plotter = 0;
	plot->params = pl_newplparams ();

	if (plot->params == 0)
	{	API->err = wmf_E_DeviceError;
		return;
	}

	ddata->file = 0;

	ddata->width  = 0;
	ddata->height = 0;

	ddata->bbox.TL.x = 0;
	ddata->bbox.TL.y = 0;
	ddata->bbox.BR.x = 0;
	ddata->bbox.BR.y = 0;

	ddata->flags = 0;
#else /* HAVE_LIBPLOT */
	API->device_data = 0;

	API->err = wmf_E_DeviceError;
#endif /* HAVE_LIBPLOT */
}
#ifdef HAVE_LIBPLOT
static void wmf_plot_draw_text (wmfAPI* API,wmfDrawText_t* draw_text)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]draw_text");

	
}

static void wmf_plot_udata_init (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]udata_init");

	
}

static void wmf_plot_udata_copy (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]udata_copy");

	
}

static void wmf_plot_udata_set (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]udata_set");

	
}

static void wmf_plot_udata_free (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]udata_free");

	
}
#endif /* HAVE_LIBPLOT */
