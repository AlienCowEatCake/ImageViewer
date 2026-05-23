/* libwmf ("ipa/fig/region.h"): library for wmf conversion
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


static void wmf_fig_region_frame (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmfBrush* set_brush = 0;

	wmfBrush brush;

	wmfPen* set_pen = 0;

	wmfPen pen;

	wmfDrawRectangle_t draw_rect;

	unsigned int i;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]region_frame");

	if (poly_rect->count == 0) return;

	/* Going to redirect this to wmf_fig_draw_rectangle
	 */
	draw_rect.dc = poly_rect->dc;

	draw_rect.width  = 0;
	draw_rect.height = 0;

	/* brush setup
	 */
	set_brush = WMF_DC_BRUSH (draw_rect.dc);

	brush = (*set_brush);

	WMF_BRUSH_SET_STYLE (&brush,BS_NULL);

	WMF_DC_SET_BRUSH (draw_rect.dc,&brush);

	/* pen setup
	 */
	set_pen = WMF_DC_PEN (draw_rect.dc);

	pen = (*set_pen);

	WMF_PEN_SET_STYLE  (&pen,PS_SOLID);
	WMF_PEN_SET_JOIN   (&pen,PS_JOIN_MITER);
	WMF_PEN_SET_ENDCAP (&pen,PS_ENDCAP_SQUARE);

	WMF_PEN_SET_WIDTH  (&pen,poly_rect->width );
	WMF_PEN_SET_HEIGHT (&pen,poly_rect->height);

	WMF_DC_SET_PEN (draw_rect.dc,&pen);

	for (i = 0; i < poly_rect->count; i++)
	{	draw_rect.TL.x = poly_rect->TL[i].x - poly_rect->width  / 2;
		draw_rect.TL.y = poly_rect->TL[i].y - poly_rect->height / 2;
		draw_rect.BR.x = poly_rect->BR[i].x + poly_rect->width  / 2;
		draw_rect.BR.y = poly_rect->BR[i].y + poly_rect->height / 2;

		wmf_fig_draw_rectangle (API,&draw_rect);
	}

	WMF_DC_SET_BRUSH (draw_rect.dc,set_brush); /* Soon to be redundant ?? */

	WMF_DC_SET_PEN (draw_rect.dc,set_pen); /* Soon to be redundant ?? */
}

static void wmf_fig_region_paint (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmfPen* set_pen = 0;

	wmfPen pen;

	wmfDrawRectangle_t draw_rect;

	unsigned int i;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]region_paint");

	if (poly_rect->count == 0) return;

	if (!TO_FILL (poly_rect)) return;

	/* Going to redirect this to wmf_fig_draw_rectangle
	 */
	draw_rect.dc = poly_rect->dc;

	draw_rect.width  = 0;
	draw_rect.height = 0;

	/* pen setup
	 */
	set_pen = WMF_DC_PEN (draw_rect.dc);

	pen = (*set_pen);

	WMF_PEN_SET_STYLE  (&pen,PS_NULL);

	WMF_DC_SET_PEN (draw_rect.dc,&pen);

	for (i = 0; i < poly_rect->count; i++)
	{	draw_rect.TL = poly_rect->TL[i];
		draw_rect.BR = poly_rect->BR[i];

		wmf_fig_draw_rectangle (API,&draw_rect);
	}

	WMF_DC_SET_PEN (draw_rect.dc,set_pen); /* Soon to be redundant ?? */
}

static void wmf_fig_region_clip (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]region_clip");

	if (out == 0) return;

	wmf_stream_printf (API,out,"# end clip\n");
	wmf_stream_printf (API,out,"# begin clip\n");

	if (poly_rect->count == 0) return;

	/* */
}
