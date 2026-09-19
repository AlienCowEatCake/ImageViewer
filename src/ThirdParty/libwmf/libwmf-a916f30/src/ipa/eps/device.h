/* libwmf ("ipa/eps/device.h"): library for wmf conversion
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
static void wmf_eps_device_open (wmfAPI* API)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]device_open");

	
}

/* This is called by wmf_api_destroy()
 */
static void wmf_eps_device_close (wmfAPI* API)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]device_close");

	
}

/* This is called from the beginning of each play for initial page setup
 */
static void wmf_eps_device_begin (wmfAPI* API)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	wmfStream* out = ddata->out;

	float Ox;
	float Oy;

	float xScale;
	float yScale;

	time_t t;
	
	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]device_begin");

	if (out == 0) return;

	if ((out->reset (out->context)) && ((API->flags & WMF_OPT_IGNORE_NONFATAL) == 0))
	{	WMF_ERROR (API,"unable to reset output stream!");
		API->err = wmf_E_DeviceError;
		return;
	}

	if ((ddata->bbox.BR.x <= ddata->bbox.TL.x) || (ddata->bbox.BR.y <= ddata->bbox.TL.y))
	{	WMF_ERROR (API,"~~~~~~~~wmf_[eps_]device_begin: bounding box has null or negative size!");
		API->err = wmf_E_Glitch;
		return;
	}

	if ((ddata->eps_width == 0) || (ddata->eps_height == 0))
	{	ddata->eps_width  = (unsigned int) ceil (ddata->bbox.BR.x - ddata->bbox.TL.x);
		ddata->eps_height = (unsigned int) ceil (ddata->bbox.BR.y - ddata->bbox.TL.y);
	}

	if (ddata->flags & WMF_EPS_STYLE_PS) /* Output in full postscript style */
	{	wmf_stream_printf (API,out,"%%!PS-Adobe-2.0\n");
		wmf_stream_printf (API,out,"%%%%BoundingBox: ");
		wmf_stream_printf (API,out," 0 0 %u %u\n",ddata->page_width,ddata->page_height);

		if (ddata->Title)
		{	wmf_stream_printf (API,out,"%%%%Title: ");
			wmf_stream_printf (API,out,"%s\n",ddata->Title);
		}
		if (ddata->Creator)
		{	wmf_stream_printf (API,out,"%%%%Creator: ");
			wmf_stream_printf (API,out,"%s\n",ddata->Creator);
		}
		if (ddata->Date)
		{	wmf_stream_printf (API,out,"%%%%Date: ");
			wmf_stream_printf (API,out,"%s\n",ddata->Date);
		}
		else 
		{	t = time (0);
			wmf_stream_printf (API,out,"%%%%Date: ");
			wmf_stream_printf (API,out,"%s\n",ctime (&t));
		}
		if (ddata->For)
		{	wmf_stream_printf (API,out,"%%%%For: ");
			wmf_stream_printf (API,out,"%s\n",ddata->For);
		}

		wmf_stream_printf (API,out,"%%%%Pages: 1\n");
		wmf_stream_printf (API,out,"%%%%PageOrder: Ascend\n");
		wmf_stream_printf (API,out,"%%%%EndComments\n");

		wmf_stream_printf (API,out,"%%%%BeginProlog\n");

		out->sputs (WMF_PS_DEFS,out->context);

		wmf_stream_printf (API,out,"%%%%EndProlog\n");

		wmf_stream_printf (API,out,"%%%%BeginSetup\n");
		wmf_stream_printf (API,out,"%%%%EndSetup\n");

		wmf_stream_printf (API,out,"%%Page: 1 1\n");

		if (ddata->flags & WMF_EPS_LANDSCAPE) wmf_stream_printf (API,out,"%%%%PageOrientation: Landscape\n");
		else                                  wmf_stream_printf (API,out,"%%%%PageOrientation: Portrait\n" );

		wmf_stream_printf (API,out,"%%%%BeginPageSetup\n");

		out->sputs (WMF_PS_HEAD,out->context);

		wmf_stream_printf (API,out,"gsave\n");

		if (ddata->flags & WMF_EPS_LANDSCAPE)
		{	wmf_stream_printf (API,out,"%u 0 translate\n",ddata->page_width);
			wmf_stream_printf (API,out,"90 rotate\n");
		}

		wmf_stream_printf (API,out,"%%%%EndPageSetup\n\n");

		wmf_stream_printf (API,out,"%d %d translate\n",ddata->eps_x,ddata->eps_y);
	}
	else                   /* Output in basic encapsulated postscript style */
	{	wmf_stream_printf (API,out,"%%!PS-Adobe-2.0 EPSF-2.0\n");
		wmf_stream_printf (API,out,"%%%%BoundingBox: ");
		wmf_stream_printf (API,out," 0 0 %d %d\n",ddata->eps_width,ddata->eps_height);

		out->sputs (WMF_PS_DEFS,out->context);
		out->sputs (WMF_PS_HEAD,out->context);

		wmf_stream_printf (API,out,"gsave\n");
	}

	wmf_stream_printf (API,out,"0 %d translate\n",ddata->eps_height);

	wmf_stream_printf (API,out,"1 -1 scale\n");

	Ox = - ((float) ddata->eps_width  * ddata->bbox.TL.x) / (ddata->bbox.BR.x - ddata->bbox.TL.x);
	Oy = - ((float) ddata->eps_height * ddata->bbox.TL.y) / (ddata->bbox.BR.y - ddata->bbox.TL.y);

	wmf_stream_printf (API,out,"%f %f translate\n",Ox,Oy);

	xScale = (float) ddata->eps_width  / (ddata->bbox.BR.x - ddata->bbox.TL.x);
	yScale = (float) ddata->eps_height / (ddata->bbox.BR.y - ddata->bbox.TL.y);

	wmf_stream_printf (API,out,"%f %f scale\n",xScale,yScale);

	wmf_stream_printf (API,out,"gsave %% begin clip\n");
}

/* This is called from the end of each play for page termination
 */
static void wmf_eps_device_end (wmfAPI* API)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]device_end");

	if (out == 0) return;

	wmf_stream_printf (API,out,"grestore %% end clip\n");

	if (ddata->flags & WMF_EPS_STYLE_PS) /* Output in full postscript style */
	{	wmf_stream_printf (API,out,"%%%%PageTrailer\n");

		wmf_stream_printf (API,out,"grestore\n");

		out->sputs (WMF_PS_TAIL,out->context);

		wmf_stream_printf (API,out,"showpage\n");

		wmf_stream_printf (API,out,"%%%%Trailer\n");
		wmf_stream_printf (API,out,"%%%%EOF\n");
	}
	else                   /* Output in basic encapsulated postscript style */
	{	wmf_stream_printf (API,out,"grestore\n");

		out->sputs (WMF_PS_TAIL,out->context);

		wmf_stream_printf (API,out,"showpage\n");
	}
}
