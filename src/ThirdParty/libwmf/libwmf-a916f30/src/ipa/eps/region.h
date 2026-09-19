/* libwmf ("ipa/eps/region.h"): library for wmf conversion
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


static void wmf_eps_region_frame (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	wmfStream* out = ddata->out;

	wmfPen* set_pen = 0;

	wmfPen pen;

	wmfDC dc;

	wmfD_Rect rect;

	unsigned int i;

	float linewidth;
	float stretch;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]region_frame");

	if (out == 0) return;

	if (poly_rect->count == 0) return;

	set_pen = WMF_DC_PEN (poly_rect->dc);

	pen = (*set_pen);

	WMF_PEN_SET_STYLE  (&pen,PS_SOLID);
	WMF_PEN_SET_ENDCAP (&pen,PS_ENDCAP_SQUARE);
	WMF_PEN_SET_JOIN   (&pen,PS_JOIN_MITER);

	WMF_DC_SET_PEN (&dc,&pen);

	linewidth = (float) poly_rect->height;
	stretch = poly_rect->width / poly_rect->height;

	for (i = 0; i < poly_rect->count; i++)
	{	rect.TL.x = poly_rect->TL[i].x - poly_rect->width  / 2;
		rect.TL.y = poly_rect->TL[i].y - poly_rect->height / 2;
		rect.BR.x = poly_rect->BR[i].x + poly_rect->width  / 2;
		rect.BR.y = poly_rect->BR[i].y + poly_rect->height / 2;

		wmf_stream_printf (API,out,"gsave %% wmf_[eps_]region_frame\n");

		wmf_stream_printf (API,out,"%f 1 scale ",stretch);

		wmf_stream_printf (API,out,"newpath %f %f moveto %f %f lineto %f %f lineto %f %f lineto closepath ",
		         rect.TL.x / stretch,rect.TL.y,
		         rect.TL.x / stretch,rect.BR.y,
		         rect.BR.x / stretch,rect.BR.y,
		         rect.BR.x / stretch,rect.TL.y);

		eps_path_stroke (API,&dc,linewidth);

		wmf_stream_printf (API,out,"grestore\n");
	}
}

static void wmf_eps_region_paint (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	unsigned int i;

	wmfStream* out = ddata->out;

	wmfD_Rect bbox;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]region_paint");

	if (out == 0) return;

	if (poly_rect->count == 0) return;

	if (TO_FILL (poly_rect))
	{	for (i = 0; i < poly_rect->count; i++)
		{	bbox.TL = poly_rect->TL[i];
			bbox.BR = poly_rect->BR[i];

			wmf_stream_printf (API,out,"gsave %% wmf_[eps_]region_paint\n");

			wmf_stream_printf (API,out,"newpath %f %f moveto %f %f lineto %f %f lineto %f %f lineto closepath ",
			         poly_rect->TL[i].x,poly_rect->TL[i].y,
			         poly_rect->TL[i].x,poly_rect->BR[i].y,
			         poly_rect->BR[i].x,poly_rect->BR[i].y,
			         poly_rect->BR[i].x,poly_rect->TL[i].y);

			eps_path_fill (API,poly_rect->dc,&bbox);

			wmf_stream_printf (API,out,"grestore\n");
		}
	}
}

static void wmf_eps_region_clip (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	unsigned int i;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]region_clip");

	if (out == 0) return;

	wmf_stream_printf (API,out,"grestore %% end clip\n");
	wmf_stream_printf (API,out,"gsave %% begin clip\n");

	if (poly_rect->count == 0) return;

	wmf_stream_printf (API,out,"[\n");
	for (i = 0; i < poly_rect->count; i++)
	{	wmf_stream_printf (API,out,"%f %f %f %f\n",
		         poly_rect->TL[i].x,
		         poly_rect->TL[i].y,
		         poly_rect->BR[i].x-poly_rect->TL[i].x,
		         poly_rect->BR[i].y-poly_rect->TL[i].y);
	}

	wmf_stream_printf (API,out,"] rectclip\n");
}
