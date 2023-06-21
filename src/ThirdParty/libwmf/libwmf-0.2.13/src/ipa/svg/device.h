/* libwmf ("ipa/svg/device.h"): library for wmf conversion
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
static void wmf_svg_device_open (wmfAPI* API)
{	/* wmf_svg_t* ddata = WMF_SVG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]device_open");

	
}

/* This is called by wmf_api_destroy()
 */
static void wmf_svg_device_close (wmfAPI* API)
{	/* wmf_svg_t* ddata = WMF_SVG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]device_close");

	
}

/* This is called from the beginning of each play for initial page setup
 */
static void wmf_svg_device_begin (wmfAPI* API)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]device_begin");

	if (out == 0) return;

	if ((out->reset (out->context)) && ((API->flags & WMF_OPT_IGNORE_NONFATAL) == 0))
	{	WMF_ERROR (API,"unable to reset output stream!");
		API->err = wmf_E_DeviceError;
		return;
	}

	if ((ddata->bbox.BR.x <= ddata->bbox.TL.x) || (ddata->bbox.BR.y <= ddata->bbox.TL.y))
	{	WMF_ERROR (API,"~~~~~~~~wmf_[svg_]device_begin: bounding box has null or negative size!");
		API->err = wmf_E_Glitch;
		return;
	}

	if ((ddata->width == 0) || (ddata->height == 0))
	{	ddata->width  = (unsigned int) ceil (ddata->bbox.BR.x - ddata->bbox.TL.x);
		ddata->height = (unsigned int) ceil (ddata->bbox.BR.y - ddata->bbox.TL.y);
	}

	wmf_stream_printf (API,out,"<?xml version=\"1.0\" standalone=\"no\"?>\n");
	wmf_stream_printf (API,out,"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20001102//EN\"\n");
	wmf_stream_printf (API,out,"\"http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd\">\n");

	wmf_stream_printf (API,out,"<svg width=\"%u\" height=\"%u\"\n",ddata->width,ddata->height);
	wmf_stream_printf (API,out,"\txmlns:sodipodi=\"http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd\">\n");

	if (ddata->Description)
	{	wmf_stream_printf (API,out,"<desc>%s</desc>\n",ddata->Description);
	}
}

/* This is called from the end of each play for page termination
 */
static void wmf_svg_device_end (wmfAPI* API)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]device_end");

	if (out == 0) return;

	wmf_stream_printf (API,out,"</svg>\n");
}

static svgPoint svg_translate (wmfAPI* API,wmfD_Coord d_pt)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	svgPoint g_pt;

	double x;
	double y;

	x = ((double) d_pt.x - (double) ddata->bbox.TL.x);
	x /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);
	x *= (double) ddata->width;

	y = ((double) d_pt.y - (double) ddata->bbox.TL.y);
	y /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);
	y *= (double) ddata->height;

	g_pt.x = (float) x;
	g_pt.y = (float) y;

	return (g_pt);
}

static float svg_width (wmfAPI* API,float wmf_width)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	double width;

	width = (double) wmf_width * (double) ddata->width;
	width /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);

	return ((float) width);
}

static float svg_height (wmfAPI* API,float wmf_height)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	double height;

	height = (double) wmf_height * (double) ddata->height;
	height /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);

	return ((float) height);
}
