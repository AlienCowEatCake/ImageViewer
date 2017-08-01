/* libwmf ("ipa/svg/draw.h"): library for wmf conversion
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


static void wmf_svg_flood_interior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_svg_t* ddata = WMF_SVG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]flood_interior");

	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
	{	WMF_DEBUG (API,"flood_interior unsupported.");
	}
	else
	{	WMF_ERROR (API,"flood_interior unsupported.");
		API->err = wmf_E_Glitch;
	}
}

static void wmf_svg_flood_exterior (wmfAPI* API,wmfFlood_t* flood)
{	/* wmf_svg_t* ddata = WMF_SVG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]flood_exterior");

	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
	{	WMF_DEBUG (API,"flood_exterior unsupported.");
	}
	else
	{	WMF_ERROR (API,"flood_exterior unsupported.");
		API->err = wmf_E_Glitch;
	}
}

static void wmf_svg_draw_pixel (wmfAPI* API,wmfDrawPixel_t* draw_pixel)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	wmfRGB* rgb = &(draw_pixel->color);

	svgPoint pt;

	float width;
	float height;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_pixel");

	if (out == 0) return;

	pt = svg_translate (API,draw_pixel->pt);

	width  = svg_width  (API,(float) draw_pixel->pixel_width );
	height = svg_height (API,(float) draw_pixel->pixel_height);

	wmf_stream_printf (API,out,"<rect ");

	wmf_stream_printf (API,out,"x=\"%f\" ",pt.x);
	wmf_stream_printf (API,out,"y=\"%f\" ",pt.y);

	wmf_stream_printf (API,out, "width=\"%f\" ",width);
	wmf_stream_printf (API,out,"height=\"%f\" ",height);

	wmf_stream_printf (API,out,"style=\"fill:%s; stroke:none\" ",svg_color_closest (rgb));

	wmf_stream_printf (API,out,"/>\n");
}

static void wmf_svg_draw_pie (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_svg_t* ddata = WMF_SVG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_pie");

	svg_draw_arc (API,draw_arc,svg_arc_pie);
}

static void wmf_svg_draw_chord (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_svg_t* ddata = WMF_SVG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_chord");

	svg_draw_arc (API,draw_arc,svg_arc_chord);
}

static void wmf_svg_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	/* wmf_svg_t* ddata = WMF_SVG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_arc");

	svg_draw_arc (API,draw_arc,svg_arc_open);
}

static void wmf_svg_draw_ellipse (wmfAPI* API,wmfDrawArc_t* draw_arc)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	svgPoint TL;
	svgPoint BR;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_ellipse");

	if (out == 0) return;

	if (TO_FILL (draw_arc) || TO_DRAW (draw_arc))
	{	TL = svg_translate (API,draw_arc->TL);
		BR = svg_translate (API,draw_arc->BR);

		wmf_stream_printf (API,out,"<ellipse ");

		wmf_stream_printf (API,out,"cx=\"%f\" ",(BR.x+TL.x)/2);
		wmf_stream_printf (API,out,"cy=\"%f\" ",(BR.y+TL.y)/2);

		wmf_stream_printf (API,out,"rx=\"%f\" ",(BR.x-TL.x)/2);
		wmf_stream_printf (API,out,"ry=\"%f\" ",(BR.y-TL.y)/2);

		wmf_stream_printf (API,out,"\n\t");

		wmf_stream_printf (API,out,"style=\"");
		svg_style_fill (API,draw_arc->dc);
		wmf_stream_printf (API,out,"; ");
		svg_style_stroke (API,draw_arc->dc);
		wmf_stream_printf (API,out,"\" ");

		wmf_stream_printf (API,out,"/>\n");
	}
}

static void svg_draw_arc (wmfAPI* API,wmfDrawArc_t* draw_arc,svg_arc_t finish)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	svgPoint TL;
	svgPoint BR;

	svgPoint start;
	svgPoint end;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~svg_draw_arc");

	if (out == 0) return;

	if ((draw_arc->start.x == draw_arc->end.x) || (draw_arc->start.y == draw_arc->end.y))
	{	wmf_svg_draw_ellipse (API,draw_arc);
		return;
	}

	if (TO_FILL (draw_arc) || TO_DRAW (draw_arc))
	{	TL = svg_translate (API,draw_arc->TL);
		BR = svg_translate (API,draw_arc->BR);

		start = svg_translate (API,draw_arc->start);
		end   = svg_translate (API,draw_arc->end  );

		wmf_stream_printf (API,out,"<path ");

		wmf_stream_printf (API,out,"d=\"");
		wmf_stream_printf (API,out,"M%f,%f ",start.x,start.y); /* TODO: large angle flag in next line: */
		wmf_stream_printf (API,out,"A%f,%f 0 0,1 %f,%f ",(BR.x-TL.x)/2,(BR.y-TL.y)/2,end.x,end.y);
		if (finish == svg_arc_pie) wmf_stream_printf (API,out,"L%f,%f ",(BR.x+TL.x)/2,(BR.y+TL.y)/2);
		if (finish != svg_arc_open) wmf_stream_printf (API,out,"Z ");
		wmf_stream_printf (API,out,"\"");

		wmf_stream_printf (API,out,"\n\t");

		wmf_stream_printf (API,out,"style=\"");
		if (finish == svg_arc_open)
		{	wmf_stream_printf (API,out,"fill:none; ");
		}
		else
		{	svg_style_fill (API,draw_arc->dc);
			wmf_stream_printf (API,out,"; ");
		}
		svg_style_stroke (API,draw_arc->dc);
		wmf_stream_printf (API,out,"\" ");

		wmf_stream_printf (API,out,"/>\n");
	}
}

static void wmf_svg_draw_line (wmfAPI* API,wmfDrawLine_t* draw_line)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	svgPoint from;
	svgPoint to;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_line");

	if (out == 0) return;

	if (TO_DRAW (draw_line))
	{	from = svg_translate (API,draw_line->from);
		to   = svg_translate (API,draw_line->to  );

		wmf_stream_printf (API,out,"<line ");

		wmf_stream_printf (API,out,"x1=\"%f\" ",from.x);
		wmf_stream_printf (API,out,"y1=\"%f\" ",from.y);

		wmf_stream_printf (API,out,"x2=\"%f\" ",to.x);
		wmf_stream_printf (API,out,"y2=\"%f\" ",to.y);

		wmf_stream_printf (API,out,"\n\t");

		wmf_stream_printf (API,out,"style=\"");
		svg_style_stroke (API,draw_line->dc);
		wmf_stream_printf (API,out,"\" ");

		wmf_stream_printf (API,out,"/>\n");
	}
}

static void wmf_svg_poly_line (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	svgPoint pt;

	U16 i;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]poly_line");

	if (out == 0) return;

	if (poly_line->count <= 1) return;

	if (TO_DRAW (poly_line))
	{	wmf_stream_printf (API,out,"<polyline ");

		wmf_stream_printf (API,out,"points=\"");
		for (i = 0; i < poly_line->count; i++)
		{	if ((i & 3) == 1) wmf_stream_printf (API,out,"\n\t");

			pt = svg_translate (API,poly_line->pt[i]);

			wmf_stream_printf (API,out,"%f,%f ",pt.x,pt.y);
		}
		wmf_stream_printf (API,out,"\"\n\t");

		wmf_stream_printf (API,out,"style=\"");
		svg_style_stroke (API,poly_line->dc);
		wmf_stream_printf (API,out,"\" ");

		wmf_stream_printf (API,out,"/>\n");
	}
}

static void wmf_svg_draw_polygon (wmfAPI* API,wmfPolyLine_t* poly_line)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	svgPoint pt;

	U16 i;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_polygon");

	if (out == 0) return;

	if (poly_line->count <= 2) return;

	if (TO_FILL (poly_line) || TO_DRAW (poly_line))
	{	wmf_stream_printf (API,out,"<polygon ");

		wmf_stream_printf (API,out,"points=\"");
		for (i = 0; i < poly_line->count; i++)
		{	if ((i & 3) == 1) wmf_stream_printf (API,out,"\n\t");

			pt = svg_translate (API,poly_line->pt[i]);

			wmf_stream_printf (API,out,"%f,%f ",pt.x,pt.y);
		}
		wmf_stream_printf (API,out,"\"\n\t");

		wmf_stream_printf (API,out,"style=\"");
		svg_style_fill (API,poly_line->dc);
		wmf_stream_printf (API,out,"; ");
		svg_style_stroke (API,poly_line->dc);
		wmf_stream_printf (API,out,"\" ");

		wmf_stream_printf (API,out,"/>\n");
	}
}

static void wmf_svg_draw_polypolygon(wmfAPI * API, wmfPolyPoly_t* polypolygon)
{
  wmf_svg_t* ddata = WMF_SVG_GetData (API);

  wmfStream* out = ddata->out;

  WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_polypolygon");

  if (out == 0) return;

  if (TO_FILL(polypolygon) || TO_DRAW(polypolygon))
    {
      int
        polygon,
        point;

      wmfPolyLine_t
        poly_line;

      svgPoint pt;

      wmf_stream_printf (API,out,"<path d=\"");
      for (polygon = 0; polygon < polypolygon->npoly; polygon++)
        {
          poly_line.dc = polypolygon->dc;
          poly_line.pt = polypolygon->pt[polygon];
          poly_line.count = polypolygon->count[polygon];
          if ((poly_line.count > 2) && poly_line.pt)
            {
              pt = svg_translate (API,poly_line.pt[0]);
              wmf_stream_printf (API,out,"M%f,%fL",pt.x,pt.y);
              for (point = 1; point < poly_line.count; point++)
                {
                  if ((point & 3) == 1) wmf_stream_printf (API,out,"\n\t");
                  pt = svg_translate (API,poly_line.pt[point]);
                  wmf_stream_printf (API,out,"%f,%f ",pt.x,pt.y);
                }
              wmf_stream_printf (API,out,"Z");
            }
        }
      wmf_stream_printf (API,out,"\"\n\t");

      wmf_stream_printf (API,out,"style=\"");
      svg_style_fill (API,polypolygon->dc);
      wmf_stream_printf (API,out,"; ");
      svg_style_stroke (API,polypolygon->dc);
      wmf_stream_printf (API,out,"\" ");

      wmf_stream_printf (API,out,"/>\n");
    }
}

static void wmf_svg_draw_rectangle (wmfAPI* API,wmfDrawRectangle_t* draw_rect)
{	wmf_svg_t* ddata = WMF_SVG_GetData (API);

	svgPoint TL;
	svgPoint BR;

	float width;
	float height;

	wmfStream* out = ddata->out;

	WMF_DEBUG (API,"~~~~~~~~wmf_[svg_]draw_rectangle");

	if (out == 0) return;

	if (TO_FILL (draw_rect) || TO_DRAW (draw_rect))
	{	TL = svg_translate (API,draw_rect->TL);
		BR = svg_translate (API,draw_rect->BR);

		wmf_stream_printf (API,out,"<rect ");

		wmf_stream_printf (API,out,"x=\"%f\" ",TL.x);
		wmf_stream_printf (API,out,"y=\"%f\" ",TL.y);

		wmf_stream_printf (API,out, "width=\"%f\" ",BR.x-TL.x);
		wmf_stream_printf (API,out,"height=\"%f\" ",BR.y-TL.y);

		if ((draw_rect->width > 0) || (draw_rect->height > 0))
		{	width  = svg_width  (API,draw_rect->width ) / 2;
			height = svg_height (API,draw_rect->height) / 2;

			wmf_stream_printf (API,out,"rx=\"%f\" ",width);
			wmf_stream_printf (API,out,"ry=\"%f\" ",height);
		}

		wmf_stream_printf (API,out,"\n\t");

		wmf_stream_printf (API,out,"style=\"");
		svg_style_fill (API,draw_rect->dc);
		wmf_stream_printf (API,out,"; ");
		svg_style_stroke (API,draw_rect->dc);
		wmf_stream_printf (API,out,"\" ");

		wmf_stream_printf (API,out,"/>\n");
	}
}
