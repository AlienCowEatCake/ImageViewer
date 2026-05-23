/* libwmf ("ipa/plot/device.h"): library for wmf conversion
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


/* This is called by wmf_play() the *first* time the meta file is played
 */
static void wmf_plot_device_open (wmfAPI* API)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	/* plot_t* plot = (plot_t*) ddata->plot_data; */

	WMF_DEBUG (API,"wmf_[plot_]device_open");

	
}

/* This is called by wmf_api_destroy()
 */
static void wmf_plot_device_close (wmfAPI* API)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]device_close");

	/* */

	if (plot)
	{	if (plot->params) pl_deleteplparams (plot->params);
	}
}

/* This is called from the beginning of each play for initial page setup
 */
static void wmf_plot_device_begin (wmfAPI* API)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]device_begin");

	
}

/* This is called from the end of each play for page termination
 */
static void wmf_plot_device_end (wmfAPI* API)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]device_end");

	
}

static plotPoint plot_translate (wmfAPI* API,wmfD_Coord d_pt)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plotPoint g_pt;

	double x;
	double y;

	x = ((double) d_pt.x - (double) ddata->bbox.TL.x);
	x /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);
	x *= (double) ddata->width;

	y = ((double) d_pt.y - (double) ddata->bbox.TL.y);
	y /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);
	y *= (double) ddata->height;

	g_pt.x = (int) floor (x);
	g_pt.y = (int) floor (y);

	return (g_pt);
}

static float plot_width (wmfAPI* API,float wmf_width)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	double width;

	width = (double) wmf_width * (double) ddata->width;
	width /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);

	return ((float) width);
}

static float plot_height (wmfAPI* API,float wmf_height)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	double height;

	height = (double) wmf_height * (double) ddata->height;
	height /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);

	return ((float) height);
}
