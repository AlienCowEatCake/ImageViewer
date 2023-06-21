/* libwmf ("ipa/x/region.h"): library for wmf conversion
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


static void wmf_x_region_frame (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	wmfPen* pen = 0;

	XPoint TL;
	XPoint BR;

	unsigned int i;

	int width;
	int height;

	WMF_DEBUG (API,"wmf_[x_]region_frame");

	if (poly_rect->count == 0) return;

	setdefaultstyle (API);

	pen = WMF_DC_PEN (poly_rect->dc);

	XSetForeground (ddata->display,ddata->gc,get_color (API,WMF_PEN_COLOR (pen)));

	width  = x_width  (API,poly_rect->width );
	height = x_height (API,poly_rect->height);

	if (width  < 1) width  = 1;
	if (height < 1) height = 1;

	for (i = 0; i < poly_rect->count; i++)
	{	TL = x_translate (API,poly_rect->TL[i]);
		BR = x_translate (API,poly_rect->BR[i]);

		if (ddata->window != None)
		{	XFillRectangle (ddata->display,ddata->window,ddata->gc,
			                TL.x-width,TL.y-height,TL.x-(TL.x-width),BR.y-(TL.y-height));
			XFillRectangle (ddata->display,ddata->window,ddata->gc,
			                TL.x-width,BR.y,BR.x-(TL.x-width),BR.y+height-(BR.y));
			XFillRectangle (ddata->display,ddata->window,ddata->gc,
			                TL.x,TL.y-height,BR.x+width-(TL.x),TL.y-(TL.y-height));
			XFillRectangle (ddata->display,ddata->window,ddata->gc,
			                BR.x,TL.y,BR.x+width-(BR.x),BR.y+height-(TL.y));
		}
		if (ddata->pixmap != None)
		{	XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,
			                TL.x-width,TL.y-height,TL.x-(TL.x-width),BR.y-(TL.y-height));
			XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,
			                TL.x-width,BR.y,BR.x-(TL.x-width),BR.y+height-(BR.y));
			XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,
			                TL.x,TL.y-height,BR.x+width-(TL.x),TL.y-(TL.y-height));
			XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,
			                BR.x,TL.y,BR.x+width-(BR.x),BR.y+height-(TL.y));
		}
	}
}

static void wmf_x_region_paint (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint TL;
	XPoint BR;

	unsigned int i;

	WMF_DEBUG (API,"wmf_[x_]region_paint");

	if (poly_rect->count == 0) return;

	if (TO_FILL (poly_rect))
	{	setbrushstyle (API,poly_rect->dc);

		for (i = 0; i < poly_rect->count; i++)
		{	TL = x_translate (API,poly_rect->TL[i]);
			BR = x_translate (API,poly_rect->BR[i]);

			BR.x -= TL.x;
			BR.y -= TL.y;

			if (ddata->window != None)
			{	XFillRectangle (ddata->display,ddata->window,ddata->gc,TL.x,TL.y,BR.x,BR.y);
			}
			if (ddata->pixmap != None)
			{	XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,TL.x,TL.y,BR.x,BR.y);
			}
		}
	}
}

static void wmf_x_region_clip (wmfAPI* API,wmfPolyRectangle_t* poly_rect)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint TL;
	XPoint BR;

	XRectangle* rect;

	unsigned int i;

	WMF_DEBUG (API,"wmf_[x_]region_clip");

	XSetClipMask (ddata->display,ddata->gc,None);

	if (poly_rect->count == 0) return;

	rect = (XRectangle*) wmf_malloc (API,poly_rect->count * sizeof (XRectangle));

	for (i = 0; i < poly_rect->count; i++)
	{	TL = x_translate (API,poly_rect->TL[i]);
		BR = x_translate (API,poly_rect->BR[i]);

		rect[i].x = TL.x;
		rect[i].y = TL.y;
		rect[i].width  = BR.x - TL.x;
		rect[i].height = BR.y - TL.y;
	}

	XSetClipRectangles (ddata->display,ddata->gc,0,0,rect,poly_rect->count,Unsorted);

	wmf_free (API,rect);
}
