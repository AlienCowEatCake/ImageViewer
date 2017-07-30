/* libwmf ("ipa/fig/draw.h"): library for wmf conversion
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


static void wmf_fig_flood_interior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_fig_t* ddata = WMF_FIG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]flood_interior");

	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
	{	WMF_DEBUG (API,"flood_interior unsupported.");
	}
	else
	{	WMF_ERROR (API,"flood_interior unsupported.");
		API->err = wmf_E_Glitch;
	}
}

static void wmf_fig_flood_exterior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_fig_t* ddata = WMF_FIG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]flood_exterior");

	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
	{	WMF_DEBUG (API,"flood_exterior unsupported.");
	}
	else
	{	WMF_ERROR (API,"flood_exterior unsupported.");
		API->err = wmf_E_Glitch;
	}
}

static void wmf_fig_draw_pixel (wmfAPI* API,wmfDrawPixel_t* draw_pixel)
{	wmfBrush* set_brush = 0;

	wmfBrush brush;

	wmfPen* set_pen = 0;

	wmfPen pen;

	wmfDrawRectangle_t draw_rect;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_pixel");

	/* Going to redirect this to wmf_fig_draw_rectangle
	 */
	draw_rect.dc = draw_pixel->dc;

	draw_rect.width  = 0;
	draw_rect.height = 0;

	/* brush setup
	 */
	set_brush = WMF_DC_BRUSH (draw_rect.dc);

	brush = (*set_brush);

	WMF_BRUSH_SET_STYLE (&brush,BS_SOLID);

	WMF_BRUSH_SET_COLOR (&brush,&(draw_pixel->color));

	WMF_DC_SET_BRUSH (draw_rect.dc,&brush);

	/* pen setup
	 */
	set_pen = WMF_DC_PEN (draw_rect.dc);

	pen = (*set_pen);

	WMF_PEN_SET_STYLE (&pen,PS_NULL);

	WMF_DC_SET_PEN (draw_rect.dc,&pen);

	draw_rect.TL.x = (float)  draw_pixel->pt.x;
	draw_rect.TL.y = (float)  draw_pixel->pt.y;
	draw_rect.BR.x = (float) (draw_pixel->pt.x + draw_pixel->pixel_width );
	draw_rect.BR.y = (float) (draw_pixel->pt.y + draw_pixel->pixel_height);

	wmf_fig_draw_rectangle (API,&draw_rect);

	WMF_DC_SET_BRUSH (draw_rect.dc,set_brush); /* Soon to be redundant ?? */

	WMF_DC_SET_PEN (draw_rect.dc,set_pen); /* Soon to be redundant ?? */
}

static void wmf_fig_draw_pie (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;
	
	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_pie");

	wmf_stream_printf (API,out,"# wmf_[fig_]draw_pie\n");
	
	fig_draw_arc (API,draw_arc,0);
	
	wmf_stream_printf (API,out,"# draw_pie not impl\n");
}

static void wmf_fig_draw_chord (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;
	
	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_chord");

	wmf_stream_printf (API,out,"# draw_chord not impl\n");
}

static void wmf_fig_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;
	
	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_arc");

	wmf_stream_printf (API,out,"# wmf_[fig_]draw_arc\n");
	
	fig_draw_arc(API, draw_arc, 1);
	
	wmf_stream_printf (API,out,"# end draw_arc\n");
}

static void fig_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc,int sub_type)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	figDC fig;

	figPoint TL;
	figPoint BR;

	int direction;

	int Ox;
	int Oy;

	float start_x;
	float start_y;
	float end_x;
	float end_y;
	float mid_x;
	float mid_y;

	float rad;
	float start;
	float end;
	float mid;
	
	WMF_DEBUG (API,"~~~~~~~~fig_draw_arc");

	wmf_stream_printf (API,out,"# fig_draw_arc\n");

	fig_set_style (API,draw_arc->dc,&fig);

	ddata->depth -= ddata->ddec;
	
	direction = 0;

	start_x = (float) draw_arc->start.x;
	start_y = (float) draw_arc->start.y;
	end_x   = (float) draw_arc->end.x;
	end_y   = (float) draw_arc->end.y;

	TL = fig_translate (API,draw_arc->TL);
	BR = fig_translate (API,draw_arc->BR);

	/* origin: */
	Ox = (BR.x + TL.x + 1) / 2; 
	Oy = (BR.y + TL.y + 1) / 2;

	/* long axis */
	rad = ((float) (BR.x - TL.x)) / 2;

	/*
	start_y = sqrt(rad * rad - start_x * start_x);
	if (draw_arc->start.y < 0) start_y = - start_y;
	end_y   = sqrt(rad * rad - end_x * end_x);
	if (draw_arc->end.y < 0) end_y = - end_y;
	*/

	start = atan2 (start_y,start_x);
	end   = atan2 (  end_y,  end_x);

	/* rad = sqrt(start_x*start_x + start_y*start_y); */

	/* Construct middle point: */
	if (end < start) end += M_2PI;
	mid = 0.5 * (start + end);

	mid_x = Ox + (rad * cos (mid));
	mid_y = Oy + (rad * sin (mid));

	/* Shift to origin: */
	start_x = Ox + start_x;
	start_y = Oy + start_y;
	end_x = Ox + end_x;
	end_y = Oy + end_y;
	
	wmf_stream_printf (API,out,
		"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %f %f %d %d %d %d %d %d\n", 
		O_ARC,
		sub_type,
		fig.line_style,
		fig.thickness,
		fig.pen_color,
		fig.fill_color,
		ddata->depth,
		fig.pen_style,
		fig.area_fill,
		fig.style_val,
		fig.cap_style,
		direction,
		fig.forward_arrow,
		fig.backward_arrow,
		(float) Ox,
		(float) Oy, 
		(int) start_x,
		(int) start_y,
		(int) mid_x,
		(int) mid_y,
		(int) end_x,
		(int) end_y);
}

static void wmf_fig_draw_ellipse (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	figDC fig;

	figPoint TL;
	figPoint BR;

	float angle;

	int sub_type;
	int direction;

	int Ox;
	int Oy;
	int a;
	int b;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_ellipse");

	wmf_stream_printf (API,out,"# wmf_[fig_]draw_ellipse\n");

	fig_set_style (API,draw_arc->dc,&fig);

	sub_type = 1;

	ddata->depth -= ddata->ddec;

	direction = 1;

	angle = 0.0;

	TL = fig_translate (API,draw_arc->TL);
	BR = fig_translate (API,draw_arc->BR);

	/* origin of ellipse */
	Ox = (BR.x + TL.x + 1) / 2; 
	Oy = (BR.y + TL.y + 1) / 2;

	/* axes of ellipse */
	a = (BR.x - TL.x + 1) / 2;
	b = (BR.y - TL.y + 1) / 2;

	wmf_stream_printf (API,out,
		"%d %d %d %d %d %d %d %d %d %f %d %f %d %d %d %d %d %d %d %d\n", 
		O_ELLIPSE,
		sub_type,
		fig.line_style,
		fig.thickness,
		fig.pen_color,
		fig.fill_color,
		ddata->depth,
		fig.pen_style,
		fig.area_fill,
		fig.style_val,
		direction,
		angle,
		Ox,
		Oy,
		a,
		b,
		Ox,
		Oy,
		(Ox + a),
		(Oy + b));

	wmf_stream_printf (API,out,"# end draw_ellipse\n");
}

static void wmf_fig_draw_line (wmfAPI* API,wmfDrawLine_t* draw_line)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	figDC fig;

	figPoint from;
	figPoint to;

	int npoints;
	
	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_line");

	if (out == 0) return;

	if (TO_DRAW (draw_line))
	{	wmf_stream_printf (API,out,"# wmf_[fig_]draw_line\n");

		fig_set_style (API,draw_line->dc,&fig);

		ddata->depth -= ddata->ddec;

		npoints = 2;

		wmf_stream_printf (API,out,
			"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %d %d\n",
			O_POLYLINE,
			T_POLYLINE,
			fig.line_style,
			fig.thickness,
			fig.pen_color,
			fig.fill_color,
			ddata->depth,
			fig.pen_style,
			fig.area_fill,
			fig.style_val,
			fig.join_style,
			fig.cap_style,
			fig.radius,
			fig.forward_arrow,
			fig.backward_arrow,
			npoints);

		from = fig_translate (API,draw_line->from);
		to   = fig_translate (API,draw_line->to  );

		wmf_stream_printf (API,out,
			"%d %d\n%d %d\n",
			from.x,
			from.y,
			to.x,
			to.y);

		wmf_stream_printf (API,out,"# end draw_line\n");
	}
}

static void wmf_fig_poly_line (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	wmfPolyLine_t sub_line;

	figDC fig;

	figPoint pt;

	U16 i;
	U16 sub_length;
	U16 sub_count;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]poly_line");

	if (out == 0) return;

	if (poly_line->count > 500)
	{	sub_length = poly_line->count / (1 + poly_line->count / 500);
		sub_count = 0;

		sub_line.dc = poly_line->dc;
		sub_line.pt = poly_line->pt;

		while (poly_line->count > sub_count + 1)
		{	sub_line.count = MIN (sub_length,poly_line->count - sub_count);

			wmf_fig_poly_line (API,&sub_line);
			sub_line.pt += sub_line.count - 1;
			sub_count += sub_line.count - 1;
		}
	}
	else if ((poly_line->count > 1) && TO_DRAW (poly_line))
	{	fig_set_style (API,poly_line->dc,&fig); /* fjf - the logic may have change here ?? */

		fig.area_fill = -1; /* override fill settings */

		ddata->depth -= ddata->ddec;

		wmf_stream_printf (API,out,"# wmf_[fig_]poly_line\n");
		
		wmf_stream_printf (API,out,
			"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %d %d\n",
			O_POLYLINE,
			T_POLYLINE,
			fig.line_style,
			fig.thickness,
			fig.pen_color,
			fig.fill_color,
			ddata->depth,
			fig.pen_style,
			fig.area_fill,
			fig.style_val,
			fig.join_style,
			fig.cap_style,
			fig.radius,
			fig.forward_arrow,
			fig.backward_arrow,
			poly_line->count);


		/* Output co-ordinate pairs: */
		for (i = 0; i < poly_line->count; i++)
		{	pt = fig_translate (API,poly_line->pt[poly_line->count-1-i]);

			wmf_stream_printf (API,out,"%d %d\n",pt.x,pt.y);
		}

		wmf_stream_printf (API,out,"# end poly_line\n");
	}
}

static void wmf_fig_draw_polygon (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	figDC fig;

	figPoint pt;

	U16 i;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_polygon");

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
	{	fig_set_style (API,poly_line->dc,&fig);

		ddata->depth -= ddata->ddec;

		if (TO_FILL (poly_line))
		{	wmf_stream_printf (API,out,"# wmf_[fig_]draw_polygon\n");

			wmf_stream_printf (API,out,
				"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %d %d\n",
				O_POLYLINE,
				T_POLYGON,
				fig.line_style,
				fig.thickness,
				fig.pen_color,
				fig.fill_color,
				ddata->depth,
				fig.pen_style,
				fig.area_fill,
				fig.style_val,
				fig.join_style,
				fig.cap_style,
				fig.radius,
				fig.forward_arrow,
				fig.backward_arrow,
				poly_line->count + 1);

			for (i = 0; i < poly_line->count; i++)
			{	pt = fig_translate (API,poly_line->pt[i]);

				wmf_stream_printf (API,out,"%d %d\n",pt.x,pt.y);
			}

			pt = fig_translate (API,poly_line->pt[0]);

			wmf_stream_printf (API,out,"%d %d\n",pt.x,pt.y);

			wmf_stream_printf (API,out,"# end draw_polygon\n");
		}
		if (TO_DRAW (poly_line))
		{	fig.area_fill = -1;

			fig.thickness++;
			
			wmf_stream_printf (API,out,"# wmf_[fig_]draw_polygon\n");

			wmf_stream_printf (API,out,
				"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %d %d\n",
				O_POLYLINE,
				T_POLYGON,
				fig.line_style,
				fig.thickness,
				fig.pen_color,
				fig.fill_color,
				ddata->depth,
				fig.pen_style,
				fig.area_fill,
				fig.style_val,
				fig.join_style,
				fig.cap_style,
				fig.radius,
				fig.forward_arrow,
				fig.backward_arrow,
				poly_line->count + 1);

			for (i = 0; i < poly_line->count; i++)
			{	pt = fig_translate (API,poly_line->pt[poly_line->count-1-i]);

				wmf_stream_printf (API,out,"%d %d\n",pt.x,pt.y);
			}

			pt = fig_translate (API,poly_line->pt[poly_line->count-1]);

			wmf_stream_printf (API,out,"%d %d\n",pt.x,pt.y);

			wmf_stream_printf (API,out,"# end draw_polygon\n");
		}
	}
}

static void wmf_fig_draw_rectangle (wmfAPI* API,wmfDrawRectangle_t* draw_rect)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	figDC fig;

	figPoint TL;
	figPoint BR;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_rectangle");

	if (out == 0) return;

	fig_set_style (API,draw_rect->dc,&fig);

	ddata->depth -= ddata->ddec;

	TL = fig_translate (API,draw_rect->TL);
	BR = fig_translate (API,draw_rect->BR);

	if (TO_FILL (draw_rect))
	{	wmf_stream_printf (API,out,"# wmf_[fig_]draw_rectangle\n");

		wmf_stream_printf (API,out,
			"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %d %d\n",
			O_POLYLINE,
			T_BOX,
			fig.line_style,
			fig.thickness,
			fig.pen_color,
			fig.fill_color,
			ddata->depth,
			fig.pen_style,
			fig.area_fill,
			fig.style_val,
			fig.join_style,
			fig.cap_style,
			fig.radius,
			fig.forward_arrow,
			fig.backward_arrow,
			5);

		wmf_stream_printf (API,out,"%d %d\n%d %d\n%d %d\n%d %d\n%d %d\n",
			TL.x,TL.y,
			TL.x,BR.y,
			BR.x,BR.y,
			BR.x,TL.y,
			TL.x,TL.y);

		wmf_stream_printf (API,out,"# end draw_rectangle\n");
	}
	if (TO_DRAW (draw_rect))
	{	fig.area_fill = -1;

		fig.thickness++;
		
		wmf_stream_printf (API,out,"# wmf_[fig_]draw_rectangle\n");

		wmf_stream_printf (API,out,
			"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %d %d\n",
			O_POLYLINE,
			T_BOX,
			fig.line_style,
			fig.thickness,
			fig.pen_color,
			fig.fill_color,
			ddata->depth,
			fig.pen_style,
			fig.area_fill,
			fig.style_val,
			fig.join_style,
			fig.cap_style,
			fig.radius,
			fig.forward_arrow,
			fig.backward_arrow,
			5);

		wmf_stream_printf (API,out,"%d %d\n%d %d\n%d %d\n%d %d\n %d %d\n",
			TL.x,TL.y,
			TL.x,BR.y,
			BR.x,BR.y,
			BR.x,TL.y,
			TL.x,TL.y);

		wmf_stream_printf (API,out,"# end draw_rectangle\n");
	}
}
