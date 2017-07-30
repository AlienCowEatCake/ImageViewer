/* libwmf ("ipa/eps/draw.h"): library for wmf conversion
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


static void wmf_eps_flood_interior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]flood_interior");

	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
	{	WMF_DEBUG (API,"flood_interior unsupported.");
	}
	else
	{	WMF_ERROR (API,"flood_interior unsupported.");
		API->err = wmf_E_Glitch;
	}
}

static void wmf_eps_flood_exterior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]flood_exterior");

	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
	{	WMF_DEBUG (API,"flood_exterior unsupported.");
	}
	else
	{	WMF_ERROR (API,"flood_exterior unsupported.");
		API->err = wmf_E_Glitch;
	}
}

static void wmf_eps_draw_pixel (wmfAPI* API,wmfDrawPixel_t* draw_pixel)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	float red;
	float green;
	float blue;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_pixel");

	if (out == 0) return;

	red   = (float) ((int) draw_pixel->color.r) / 255;
	green = (float) ((int) draw_pixel->color.g) / 255;
	blue  = (float) ((int) draw_pixel->color.b) / 255;

	wmf_stream_printf (API,out,"newpath %f %f moveto ",draw_pixel->pt.x,draw_pixel->pt.y);
	wmf_stream_printf (API,out,"%f dup neg exch 0 rlineto 0 %f rlineto 0 rlineto ",
	         (float) draw_pixel->pixel_width,
	         (float) draw_pixel->pixel_height);

	wmf_stream_printf (API,out,"closepath %f %f %f setrgbcolor fill\n",red,green,blue);
}

static void wmf_eps_draw_pie (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_pie");

	eps_draw_arc (API,draw_arc,eps_arc_pie);
}

static void wmf_eps_draw_chord (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_chord");

	eps_draw_arc (API,draw_arc,eps_arc_chord);
}

static void wmf_eps_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_arc");

	eps_draw_arc (API,draw_arc,eps_arc_open);
}

static void wmf_eps_draw_ellipse (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_ellipse");
	
	eps_draw_arc (API,draw_arc,eps_arc_ellipse);
}

static void eps_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc,eps_arc_t finish)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	float Ox;
	float Oy;
	float a;
	float b;
	float start = 0;
	float end = 360;
	float linewidth;

	double x_eff;
	double y_eff;
	double stretch;

	wmfStream* out = ddata->out;

	wmfPen* pen = 0;

	wmfD_Rect bbox;

	WMF_DEBUG (API,"~~~~~~~~eps_draw_arc");

	if (out == 0) return;

	bbox.TL = draw_arc->TL;
	bbox.BR = draw_arc->BR;

	Ox = (draw_arc->BR.x + draw_arc->TL.x) / 2; /* origin of ellipse */
	Oy = (draw_arc->BR.y + draw_arc->TL.y) / 2;

	a = (draw_arc->BR.x - draw_arc->TL.x) / 2;  /* axes of ellipse */
	b = (draw_arc->BR.y - draw_arc->TL.y) / 2;

	if (finish != eps_arc_ellipse)
	{	x_eff = (double) draw_arc->start.x;
		y_eff = (double) a * (double) a - x_eff * x_eff;
		y_eff = ((y_eff < 0) ? 0 : sqrt (y_eff));
		if (draw_arc->start.y < 0) y_eff = - y_eff;
		start = (float) (atan2 (y_eff,x_eff) * 180 / PI);

		x_eff = (double) draw_arc->end.x;
		y_eff = (double) a * (double) a - x_eff * x_eff;
		y_eff = ((y_eff < 0) ? 0 : sqrt (y_eff));
		if (draw_arc->end.y < 0) y_eff = - y_eff;
		end = (float) (atan2 (y_eff,x_eff) * 180 / PI);
	}

	if (TO_FILL (draw_arc) && (finish != eps_arc_open))
	{	wmf_stream_printf (API,out,"gsave %% eps_draw_arc\n");

		wmf_stream_printf (API,out,"matrix currentmatrix %f %f translate 1 %f scale ",Ox,Oy,b/a);

		if (finish == eps_arc_ellipse)
		{	wmf_stream_printf (API,out,"0 0 %f 0 360 arc ",a);
		}
		else if (finish == eps_arc_pie)
		{	wmf_stream_printf (API,out,"0 0 %f %f %f arc 0 0 lineto ",a,start,end);
		}
		else if (finish == eps_arc_chord)
		{	wmf_stream_printf (API,out,"0 0 %f %f %f arc ",a,start,end);
		}

		wmf_stream_printf (API,out,"closepath setmatrix ");

		eps_path_fill (API,draw_arc->dc,&bbox);

		wmf_stream_printf (API,out,"grestore\n");
	}
	if (TO_DRAW (draw_arc))
	{	pen = WMF_DC_PEN (draw_arc->dc);

		linewidth = (float) WMF_PEN_HEIGHT (pen);
		stretch = WMF_PEN_WIDTH (pen) / WMF_PEN_HEIGHT (pen);

		wmf_stream_printf (API,out,"gsave %% eps_draw_ellipse\n");

		wmf_stream_printf (API,out,"%f 1 scale ",stretch);

		wmf_stream_printf (API,out,"matrix currentmatrix %f %f translate 1 %f scale ",
		         (float) ((double) Ox / stretch),Oy,b / (float) ((double) a  / stretch));

		if (finish == eps_arc_ellipse)
		{	wmf_stream_printf (API,out,"0 0 %f 0 360 arc closepath ",a);
		}
		else if (finish == eps_arc_open)
		{	wmf_stream_printf (API,out,"0 0 %f %f %f arc ",a,start,end);
		}
		else if (finish == eps_arc_pie)
		{	wmf_stream_printf (API,out,"0 0 %f %f %f arc 0 0 lineto closepath ",a,start,end);
		}
		else if (finish == eps_arc_chord)
		{	wmf_stream_printf (API,out,"0 0 %f %f %f arc closepath ",a,start,end);
		}

		wmf_stream_printf (API,out,"setmatrix ");

		eps_path_stroke (API,draw_arc->dc,linewidth);

		wmf_stream_printf (API,out,"grestore\n");
	}
}

static void wmf_eps_draw_line (wmfAPI* API,wmfDrawLine_t* draw_line)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	float linewidth;

	double stretch;

	wmfStream* out = ddata->out;

	wmfPen* pen = 0;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_line");

	if (out == 0) return;

	if (TO_DRAW (draw_line))
	{	pen = WMF_DC_PEN (draw_line->dc);

		linewidth = (float) WMF_PEN_HEIGHT (pen);
		stretch = WMF_PEN_WIDTH (pen) / WMF_PEN_HEIGHT (pen);

		wmf_stream_printf (API,out,"gsave %% wmf_[eps_]draw_line\n");

		wmf_stream_printf (API,out,"%f 1 scale ",stretch);

		wmf_stream_printf (API,out,"newpath %f %f moveto %f %f lineto ",
		         (float) ((double) draw_line->from.x / stretch),draw_line->from.y,
		         (float) ((double) draw_line->to.x   / stretch),draw_line->to.y  );

		eps_path_stroke (API,draw_line->dc,linewidth);

		wmf_stream_printf (API,out,"grestore\n");
	}
}

static void wmf_eps_poly_line (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	U16 i;
	U16 sub_length;
	U16 sub_count;

	float linewidth;

	double stretch;

	wmfPolyLine_t sub_line;

	wmfStream* out = ddata->out;

	wmfPen* pen = 0;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]poly_line");

	if (out == 0) return;

	if (poly_line->count > 500)
	{	sub_length = poly_line->count / (1 + poly_line->count / 500);
		sub_count = 0;

		sub_line.dc = poly_line->dc;
		sub_line.pt = poly_line->pt;

		while (poly_line->count > sub_count + 1)
		{	sub_line.count = MIN (sub_length,poly_line->count - sub_count);

			wmf_eps_poly_line (API,&sub_line);

			sub_line.pt += sub_line.count - 1;
			sub_count += sub_line.count - 1;
		}
	}
	else if ((poly_line->count > 1) && TO_DRAW (poly_line))
	{	pen = WMF_DC_PEN (poly_line->dc);

		linewidth = (float) WMF_PEN_HEIGHT (pen);
		stretch = WMF_PEN_WIDTH (pen) / WMF_PEN_HEIGHT (pen);

		wmf_stream_printf (API,out,"gsave %% wmf_[eps_]poly_line\n");

		wmf_stream_printf (API,out,"%f 1 scale\n",stretch);

		for (i = 0; i < poly_line->count; i++)
		{	wmf_stream_printf (API,out,"%f %f\n",
			         (float) ((double) poly_line->pt[poly_line->count-1-i].x / stretch),
			         poly_line->pt[poly_line->count-1-i].y);
		}

		wmf_stream_printf (API,out,"newpath moveto 2 1 %u { pop lineto } for ",(unsigned int) poly_line->count);

		eps_path_stroke (API,poly_line->dc,linewidth);

		wmf_stream_printf (API,out,"grestore\n");
	}
}

static void wmf_eps_draw_polygon (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	U16 i;

	float linewidth;

	double stretch;

	wmfStream* out = ddata->out;

	wmfPen* pen = 0;

	wmfD_Rect bbox;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_polygon");

	if (out == 0) return;

	if (poly_line->count > 500)
	{	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
		{	WMF_DEBUG (API,"Too many points on polygon!");
		}
		else
		{	WMF_ERROR (API,"Too many points on polygon!");
			API->err = wmf_E_Glitch;
		}
	}
	else if (poly_line->count > 2)
	{	if (TO_FILL (poly_line))
		{	bbox.TL.x = poly_line->pt[0].x;
			bbox.TL.y = poly_line->pt[0].y;
			bbox.BR.x = poly_line->pt[0].x;
			bbox.BR.y = poly_line->pt[0].y;

			wmf_stream_printf (API,out,"gsave %% wmf_[eps_]draw_polygon\n");

			for (i = 0; i < poly_line->count; i++)
			{	wmf_stream_printf (API,out,"%f %f\n",poly_line->pt[i].x,poly_line->pt[i].y);

				if (bbox.TL.x > poly_line->pt[i].x) bbox.TL.x = poly_line->pt[i].x;
				if (bbox.TL.y > poly_line->pt[i].y) bbox.TL.y = poly_line->pt[i].y;
				if (bbox.BR.x < poly_line->pt[i].x) bbox.BR.x = poly_line->pt[i].x;
				if (bbox.BR.y < poly_line->pt[i].y) bbox.BR.y = poly_line->pt[i].y;
			}

			wmf_stream_printf (API,out,"newpath moveto 2 1 %u { pop lineto } for closepath ",
			         (unsigned int) poly_line->count);

			eps_path_fill (API,poly_line->dc,&bbox);

			wmf_stream_printf (API,out,"grestore\n");
		}
		if (TO_DRAW (poly_line))
		{	pen = WMF_DC_PEN (poly_line->dc);

			linewidth = (float) WMF_PEN_HEIGHT (pen);
			stretch = WMF_PEN_WIDTH (pen) / WMF_PEN_HEIGHT (pen);

			wmf_stream_printf (API,out,"gsave %% wmf_[eps_]draw_polygon\n");

			wmf_stream_printf (API,out,"%f 1 scale\n",stretch);

			for (i = 0; i < poly_line->count; i++)
			{	wmf_stream_printf (API,out,"%f %f\n",
				         (float) ((double) poly_line->pt[poly_line->count-1-i].x / stretch),
				         poly_line->pt[poly_line->count-1-i].y);
			}

			wmf_stream_printf (API,out,"newpath moveto 2 1 %u { pop lineto } for closepath ",
			         (unsigned int) poly_line->count);

			eps_path_stroke (API,poly_line->dc,linewidth);

			wmf_stream_printf (API,out,"grestore\n");
		}
	}
}

static void wmf_eps_draw_rectangle (wmfAPI* API,wmfDrawRectangle_t* draw_rect)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	float linewidth;

	double stretch;

	wmfStream* out = ddata->out;

	wmfPen* pen = 0;

	wmfD_Rect bbox;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_rectangle");

	if (out == 0) return;

	if (TO_FILL (draw_rect))
	{	bbox.TL = draw_rect->TL;
		bbox.BR = draw_rect->BR;

		wmf_stream_printf (API,out,"gsave %% wmf_[eps_]draw_rectangle\n");

		wmf_stream_printf (API,out,"newpath %f %f moveto %f %f lineto %f %f lineto %f %f lineto closepath ",
		         draw_rect->TL.x,draw_rect->TL.y,
		         draw_rect->TL.x,draw_rect->BR.y,
		         draw_rect->BR.x,draw_rect->BR.y,
		         draw_rect->BR.x,draw_rect->TL.y);

		eps_path_fill (API,draw_rect->dc,&bbox);

		wmf_stream_printf (API,out,"grestore\n");
	}
	if (TO_DRAW (draw_rect))
	{	pen = WMF_DC_PEN (draw_rect->dc);

		linewidth = (float) WMF_PEN_HEIGHT (pen);
		stretch = WMF_PEN_WIDTH (pen) / WMF_PEN_HEIGHT (pen);

		wmf_stream_printf (API,out,"gsave %% wmf_[eps_]draw_rectangle\n");

		wmf_stream_printf (API,out,"%f 1 scale ",stretch);

		wmf_stream_printf (API,out,"newpath %f %f moveto %f %f lineto %f %f lineto %f %f lineto closepath ",
		         (float) ((double) draw_rect->TL.x / stretch),draw_rect->TL.y,
		         (float) ((double) draw_rect->TL.x / stretch),draw_rect->BR.y,
		         (float) ((double) draw_rect->BR.x / stretch),draw_rect->BR.y,
		         (float) ((double) draw_rect->BR.x / stretch),draw_rect->TL.y);

		eps_path_stroke (API,draw_rect->dc,linewidth);

		wmf_stream_printf (API,out,"grestore\n");
	}
}
