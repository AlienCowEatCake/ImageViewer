/* libwmf ("ipa/x/draw.h"): library for wmf conversion
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


static void wmf_x_flood_interior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]flood_interior");

	/* TODO */
}

static void wmf_x_flood_exterior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]flood_exterior");

	/* TODO */
}

static void wmf_x_draw_pixel (wmfAPI* API,wmfDrawPixel_t* draw_pixel)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint pt;

	WMF_DEBUG (API,"wmf_[x_]draw_pixel");

	pt = x_translate (API,draw_pixel->pt);

	setdefaultstyle (API);

	XSetForeground (ddata->display,ddata->gc,get_color (API,&(draw_pixel->color)));

	if (ddata->window != None)
	{	XDrawPoint (ddata->display,ddata->window,ddata->gc,pt.x,pt.y);
	}
	if (ddata->pixmap != None)
	{	XDrawPoint (ddata->display,ddata->pixmap,ddata->gc,pt.x,pt.y);
	}
}

static void wmf_x_draw_pie (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]draw_pie");

	x_draw_arc (API,draw_arc,x_arc_pie);
}

static void wmf_x_draw_chord (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]draw_chord");

	x_draw_arc (API,draw_arc,x_arc_chord);
}

static void wmf_x_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]draw_arc");

	x_draw_arc (API,draw_arc,x_arc_open);
}

static void wmf_x_draw_ellipse (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]draw_ellipse");

	x_draw_arc (API,draw_arc,x_arc_ellipse);
}

static void x_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc,x_arc_t finish) /* TODO !! */
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint TL;
	XPoint BR;

	XPoint s_pt;
	XPoint e_pt;

	int Ox;
	int Oy;
	int w;
	int h;
	int start = 0;
	int end = 64 * 360;

	WMF_DEBUG (API,"~~~~~~~~x_draw_arc");

	TL = x_translate (API,draw_arc->TL);
	BR = x_translate (API,draw_arc->BR);

	Ox = (BR.x + TL.x) / 2; /* origin of ellipse */
	Oy = (BR.y + TL.y) / 2;

	w = (BR.x - TL.x);  /* axes of ellipse */
	h = (BR.y - TL.y);

	if (finish != x_arc_ellipse)
	{	start = (int) (atan2 (draw_arc->end.y  ,draw_arc->end.x  ) * 64 * 180 / PI);
		end   = (int) (atan2 (draw_arc->start.y,draw_arc->start.x) * 64 * 180 / PI);

		start = (64 * 360) - start;
		end   = (64 * 360) - end;

		while (start < 0)
		{	start += 64 * 360;
			end   += 64 * 360;
		}
		while (end <= start) end += 64 * 360;

		end -= start;

		if (finish != x_arc_open)
		{	s_pt = x_translate (API,draw_arc->start);
			s_pt.x += Ox;
			s_pt.y += Oy;
			e_pt = x_translate (API,draw_arc->end);
			e_pt.x += Ox;
			e_pt.y += Oy;
		}
	}

	if (TO_FILL (draw_arc) && (finish != x_arc_open))
	{	setbrushstyle (API,draw_arc->dc);

		if (finish == x_arc_ellipse)
		{	XSetArcMode (ddata->display,ddata->gc,ArcChord);
		}
		else if (finish == x_arc_pie)
		{	XSetArcMode (ddata->display,ddata->gc,ArcPieSlice);
		}
		else if (finish == x_arc_chord)
		{	XSetArcMode (ddata->display,ddata->gc,ArcChord);
		}

		if (ddata->window != None)
		{	XFillArc (ddata->display,ddata->window,ddata->gc,TL.x,TL.y,w,h,start,end);
		}
		if (ddata->pixmap != None)
		{	XFillArc (ddata->display,ddata->pixmap,ddata->gc,TL.x,TL.y,w,h,start,end);
		}
	}
	if (TO_DRAW (draw_arc))
	{	setlinestyle (API,draw_arc->dc);

		if (ddata->window != None)
		{	XDrawArc (ddata->display,ddata->window,ddata->gc,TL.x,TL.y,w,h,start,end);
		}
		if (ddata->pixmap != None)
		{	XDrawArc (ddata->display,ddata->pixmap,ddata->gc,TL.x,TL.y,w,h,start,end);
		}

		if (finish == x_arc_ellipse)
		{	/* Do nothing */
		}
		else if (finish == x_arc_open)
		{	/* Do nothing */
		}
		else if (finish == x_arc_pie)
		{	if (ddata->window != None)
			{	XDrawLine (ddata->display,ddata->window,ddata->gc,e_pt.x,e_pt.y,Ox,Oy);
				XDrawLine (ddata->display,ddata->window,ddata->gc,Ox,Oy,s_pt.x,s_pt.y);
			}
			if (ddata->pixmap != None)
			{	XDrawLine (ddata->display,ddata->pixmap,ddata->gc,e_pt.x,e_pt.y,Ox,Oy);
				XDrawLine (ddata->display,ddata->pixmap,ddata->gc,Ox,Oy,s_pt.x,s_pt.y);
			}
		}
		else if (finish == x_arc_chord)
		{	if (ddata->window != None)
			{	XDrawLine (ddata->display,ddata->window,ddata->gc,e_pt.x,e_pt.y,s_pt.x,s_pt.y);
			}
			if (ddata->pixmap != None)
			{	XDrawLine (ddata->display,ddata->pixmap,ddata->gc,e_pt.x,e_pt.y,s_pt.x,s_pt.y);
			}
		}
	}
}

static void wmf_x_draw_line (wmfAPI* API,wmfDrawLine_t* draw_line)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint from;
	XPoint to;

	WMF_DEBUG (API,"wmf_[x_]draw_line");

	if (TO_DRAW (draw_line))
	{	setlinestyle (API,draw_line->dc);

		from = x_translate (API,draw_line->from);
		to   = x_translate (API,draw_line->to  );

		if (ddata->window != None)
		{	XDrawLine (ddata->display,ddata->window,ddata->gc,from.x,from.y,to.x,to.y);
		}
		if (ddata->pixmap != None)
		{	XDrawLine (ddata->display,ddata->pixmap,ddata->gc,from.x,from.y,to.x,to.y);
		}
	}
}

static void wmf_x_poly_line (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint* pt;

	U16 i;

	WMF_DEBUG (API,"wmf_[x_]poly_line");

	if (poly_line->count < 2) return;

	pt = (XPoint*) wmf_malloc (API,poly_line->count * sizeof (XPoint));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	for (i = 0; i < poly_line->count; i++)
	{	pt[i] = x_translate (API,poly_line->pt[i]);
	}

	if (TO_DRAW (poly_line))
	{	setlinestyle (API,poly_line->dc);

		if (ddata->window != None)
		{	XDrawLines (ddata->display,ddata->window,ddata->gc,
			            pt,poly_line->count,CoordModeOrigin);
		}
		if (ddata->pixmap != None)
		{	XDrawLines (ddata->display,ddata->pixmap,ddata->gc,
			            pt,poly_line->count,CoordModeOrigin);
		}
	}

	wmf_free (API,pt);
}

static void wmf_x_draw_polygon (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint* pt;

	U16 i;

	WMF_DEBUG (API,"wmf_[x_]draw_polygon");

	if (poly_line->count < 2) return;

	pt = (XPoint*) wmf_malloc (API,(poly_line->count + 1) * sizeof (XPoint));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	for (i = 0; i < poly_line->count; i++)
	{	pt[i] = x_translate (API,poly_line->pt[i]);
	}
	pt[poly_line->count] = pt[0];

	if (TO_FILL (poly_line))
	{	setbrushstyle (API,poly_line->dc);

		if (ddata->window != None)
		{	XFillPolygon (ddata->display,ddata->window,ddata->gc,
			              pt,poly_line->count+1,Complex,CoordModeOrigin);
		}
		if (ddata->pixmap != None)
		{	XFillPolygon (ddata->display,ddata->pixmap,ddata->gc,
			              pt,poly_line->count+1,Complex,CoordModeOrigin);
		}
	}
	if (TO_DRAW (poly_line))
	{	setlinestyle (API,poly_line->dc);

		if (ddata->window != None)
		{	XDrawLines (ddata->display,ddata->window,ddata->gc,
			            pt,poly_line->count+1,CoordModeOrigin);
		}
		if (ddata->pixmap != None)
		{	XDrawLines (ddata->display,ddata->pixmap,ddata->gc,
			            pt,poly_line->count+1,CoordModeOrigin);
		}
	}

	wmf_free (API,pt);
}

static void wmf_x_draw_rectangle (wmfAPI* API,wmfDrawRectangle_t* draw_rectangle)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint TL;
	XPoint BR;

	WMF_DEBUG (API,"wmf_[x_]draw_rectangle");

	TL = x_translate (API,draw_rectangle->TL);
	BR = x_translate (API,draw_rectangle->BR);

	BR.x -= TL.x;
	BR.y -= TL.y;

	if (TO_FILL (draw_rectangle))
	{	setbrushstyle (API,draw_rectangle->dc);

		if (ddata->window != None)
		{	XFillRectangle (ddata->display,ddata->window,ddata->gc,TL.x,TL.y,BR.x,BR.y);
		}
		if (ddata->pixmap != None)
		{	XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,TL.x,TL.y,BR.x,BR.y);
		}
	}
	if (TO_DRAW (draw_rectangle))
	{	setlinestyle (API,draw_rectangle->dc);

		if (ddata->window != None)
		{	XDrawRectangle (ddata->display,ddata->window,ddata->gc,TL.x,TL.y,BR.x,BR.y);
		}
		if (ddata->pixmap != None)
		{	XDrawRectangle (ddata->display,ddata->pixmap,ddata->gc,TL.x,TL.y,BR.x,BR.y);
		}
	}
}
