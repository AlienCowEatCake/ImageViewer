/* libwmf ("ipa/fig/device.h"): library for wmf conversion
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
static void wmf_fig_device_open (wmfAPI* API)
{	
/*	wmf_fig_t* ddata = WMF_FIG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]device_open");

	
}

/* This is called by wmf_api_destroy()
 */
static void wmf_fig_device_close (wmfAPI* API)
{
/*	wmf_fig_t* ddata = WMF_FIG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]device_close");

	
}

/* This is called from the beginning of each play for initial page setup
 */
static void wmf_fig_device_begin (wmfAPI* API)
{	
	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	unsigned int page_width;
	unsigned int page_height;

	float def_width;
	float def_height;

	float ratio_def;
	float ratio_page;

	time_t t;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]device_begin");

	if (out == 0) return;

	if ((out->reset (out->context)) && ((API->flags & WMF_OPT_IGNORE_NONFATAL) == 0))
	{	WMF_ERROR (API,"unable to reset output stream!");
		API->err = wmf_E_DeviceError;
		return;
	}

	if ((ddata->bbox.BR.x <= ddata->bbox.TL.x) || (ddata->bbox.BR.y <= ddata->bbox.TL.y))
	{	WMF_ERROR (API,"bounding box has null or negative size!");
		API->err = wmf_E_Glitch;
		return;
	}

	if (ddata->dpi == 0)
	{	WMF_ERROR (API,"Glitch! dpi = 0?");
		API->err = wmf_E_Glitch;
		return;
	}

	if ((ddata->fig_width == 0) || (ddata->fig_height == 0))
	{	wmf_size (API,&def_width,&def_height);

		if (ddata->flags & WMF_FIG_LANDSCAPE)
		{	page_width  = wmf_ipa_page_height (API,ddata->format);
			page_height = wmf_ipa_page_width  (API,ddata->format);
		}
		else
		{	page_width  = wmf_ipa_page_width  (API,ddata->format);
			page_height = wmf_ipa_page_height (API,ddata->format);
		}

		if (ERR (API))
		{	WMF_DEBUG (API,"bailing...");
			return;
		}

		ddata->fig_width  = (unsigned int) ceil (def_width );
		ddata->fig_height = (unsigned int) ceil (def_height);

		if ((ddata->flags & WMF_FIG_NO_MARGINS) == 0)
		{	ddata->fig_x = 72; /* 1 inch margin */
			ddata->fig_y = 72;

			page_width  -= 144;
			page_height -= 144;
		}

		if ((ddata->fig_width > page_width) || (ddata->fig_height > page_height) || WMF_FIG_MAXPECT)
		{	ratio_def = def_height / def_width;
			ratio_page = (float) page_height / (float) page_width;

			if (ratio_def > ratio_page)
			{	ddata->fig_height = page_height;
				ddata->fig_width  = (unsigned int) ((float) page_height / ratio_def);
			}
			else
			{	ddata->fig_width  = page_width;
				ddata->fig_height = (unsigned int) ((float) page_width  * ratio_def);
			}
		}

		ddata->fig_x += ((int) page_width  - (int) ddata->fig_width ) / 2;
		ddata->fig_y += ((int) page_height - (int) ddata->fig_height) / 2;

		ddata->fig_x = (ddata->fig_x * ddata->dpi) / 72;
		ddata->fig_y = (ddata->fig_y * ddata->dpi) / 72;

		ddata->fig_width  = (ddata->fig_width  * ddata->dpi) / 72;
		ddata->fig_height = (ddata->fig_height * ddata->dpi) / 72;
	}

	ddata->depth = 999; /* This overrides user settings ?? */
	
	wmf_stream_printf (API,out,"#FIG 3.2\n");
	if (ddata->flags & WMF_FIG_LANDSCAPE)
	{	wmf_stream_printf (API,out,"Landscape\n");
	}
	else
	{	wmf_stream_printf (API,out,"Portrait\n");
	}
	wmf_stream_printf (API,out,"Center\n");
	wmf_stream_printf (API,out,"Metric\n");
	wmf_stream_printf (API,out,"%s\n",wmf_ipa_page_format (API,ddata->format));
	wmf_stream_printf (API,out,"100.0\n");
	wmf_stream_printf (API,out,"Single\n");
	wmf_stream_printf (API,out,"-2\n");
	wmf_stream_printf (API,out,"%u 2\n",ddata->dpi);

	if (ddata->Title)
	{	wmf_stream_printf (API,out,"# Title: ");
		wmf_stream_printf (API,out,"%s\n",ddata->Title);
	}
	if (ddata->Creator)
	{	wmf_stream_printf (API,out,"# Creator: ");
		wmf_stream_printf (API,out,"%s\n",ddata->Creator);
	}
	if (ddata->Date)
	{	wmf_stream_printf (API,out,"# Date: ");
		wmf_stream_printf (API,out,"%s\n",ddata->Date);
	}
	else 
	{	t = time (0);
		wmf_stream_printf (API,out,"# Date: ");
		wmf_stream_printf (API,out,"%s\n",ctime (&t));
	}
	if (ddata->For)
	{	wmf_stream_printf (API,out,"# For: ");
		wmf_stream_printf (API,out,"%s\n",ddata->For);
	}

	/* Write colour pseudo-objects to file
	 */
	fig_color_to_file (API,out);
}

/* This is called from the end of each play for page termination
 */
static void wmf_fig_device_end (wmfAPI* API)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]device_end");

	if (out == 0) return;
	
	wmf_stream_printf (API,out,"# end device_end\n");
}

/* translation & scaling functions
 */
static figPoint fig_translate (wmfAPI* API,wmfD_Coord d_pt)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	figPoint fig_pt;

	double x;
	double y;

	x = ((double) d_pt.x - (double) ddata->bbox.TL.x);
	x /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);
	x *= (double) ddata->fig_width;

	y = ((double) d_pt.y - (double) ddata->bbox.TL.y);
	y /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);
	y *= (double) ddata->fig_height;

	fig_pt.x = (int) ddata->fig_x + (int) floor (x);
	fig_pt.y = (int) ddata->fig_y + (int) floor (y);

	return (fig_pt);
}

static int fig_width (wmfAPI* API,float wmf_width)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	double width;

	width = (double) wmf_width * (double) ddata->fig_width;
	width /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);

	return ((int) ceil (ABS (width)));
}

static int fig_height (wmfAPI* API,float wmf_height)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	double height;

	height = (double) wmf_height * (double) ddata->fig_height;
	height /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);

	return ((int) ceil (ABS (height)));
}
