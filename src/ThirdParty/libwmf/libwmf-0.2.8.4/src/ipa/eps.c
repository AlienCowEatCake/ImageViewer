/* libwmf (ipa/eps.c): library for wmf conversion
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


#ifdef HAVE_CONFIG_H
#include "wmfconfig.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "wmfdefs.h"

#ifndef WITHOUT_LAYERS

#include "libwmf/eps.h"

static void wmf_eps_device_open (wmfAPI*);
static void wmf_eps_device_close (wmfAPI*);
static void wmf_eps_device_begin (wmfAPI*);
static void wmf_eps_device_end (wmfAPI*);
static void wmf_eps_flood_interior (wmfAPI*,wmfFlood_t*);
static void wmf_eps_flood_exterior (wmfAPI*,wmfFlood_t*);
static void wmf_eps_draw_pixel (wmfAPI*,wmfDrawPixel_t*);
static void wmf_eps_draw_pie (wmfAPI*,wmfDrawArc_t*);
static void wmf_eps_draw_chord (wmfAPI*,wmfDrawArc_t*);
static void wmf_eps_draw_arc (wmfAPI*,wmfDrawArc_t*);
static void wmf_eps_draw_ellipse (wmfAPI*,wmfDrawArc_t*);
static void wmf_eps_draw_line (wmfAPI*,wmfDrawLine_t*);
static void wmf_eps_poly_line (wmfAPI*,wmfPolyLine_t*);
static void wmf_eps_draw_polygon (wmfAPI*,wmfPolyLine_t*);
static void wmf_eps_draw_rectangle (wmfAPI*,wmfDrawRectangle_t*);
static void wmf_eps_rop_draw (wmfAPI*,wmfROP_Draw_t*);
static void wmf_eps_bmp_draw (wmfAPI*,wmfBMP_Draw_t*);
static void wmf_eps_bmp_read (wmfAPI*,wmfBMP_Read_t*);
static void wmf_eps_bmp_free (wmfAPI*,wmfBMP*);
static void wmf_eps_draw_text (wmfAPI*,wmfDrawText_t*);
static void wmf_eps_udata_init (wmfAPI*,wmfUserData_t*);
static void wmf_eps_udata_copy (wmfAPI*,wmfUserData_t*);
static void wmf_eps_udata_set (wmfAPI*,wmfUserData_t*);
static void wmf_eps_udata_free (wmfAPI*,wmfUserData_t*);
static void wmf_eps_region_frame (wmfAPI*,wmfPolyRectangle_t*);
static void wmf_eps_region_paint (wmfAPI*,wmfPolyRectangle_t*);
static void wmf_eps_region_clip (wmfAPI*,wmfPolyRectangle_t*);

#include "ipa/eps.h"
#include "ipa/eps/bmp.h"
#include "ipa/eps/device.h"
#include "ipa/eps/draw.h"
#include "ipa/eps/region.h"

#endif /* ! WITHOUT_LAYERS */

void wmf_eps_function (wmfAPI* API)
{
#ifndef WITHOUT_LAYERS
	wmf_eps_t* ddata = 0;

	wmfFunctionReference* FR = (wmfFunctionReference*) API->function_reference;

	if ((API->flags & API_STANDARD_INTERFACE) == 0)
	{	WMF_ERROR (API,"Can't use this device layer with 'lite' interface!");
		API->err = wmf_E_DeviceError;
		return;
	}

/* IPA function reference links
 */
	FR->device_open    = wmf_eps_device_open;
	FR->device_close   = wmf_eps_device_close;
	FR->device_begin   = wmf_eps_device_begin;
	FR->device_end     = wmf_eps_device_end;
	FR->flood_interior = wmf_eps_flood_interior;
	FR->flood_exterior = wmf_eps_flood_exterior;
	FR->draw_pixel     = wmf_eps_draw_pixel;
	FR->draw_pie       = wmf_eps_draw_pie;
	FR->draw_chord     = wmf_eps_draw_chord;
	FR->draw_arc       = wmf_eps_draw_arc;
	FR->draw_ellipse   = wmf_eps_draw_ellipse;
	FR->draw_line      = wmf_eps_draw_line;
	FR->poly_line      = wmf_eps_poly_line;
	FR->draw_polygon   = wmf_eps_draw_polygon;
	FR->draw_rectangle = wmf_eps_draw_rectangle;
	FR->rop_draw       = wmf_eps_rop_draw;
	FR->bmp_draw       = wmf_eps_bmp_draw;
	FR->bmp_read       = wmf_eps_bmp_read;
	FR->bmp_free       = wmf_eps_bmp_free;
	FR->draw_text      = wmf_eps_draw_text;
	FR->udata_init     = wmf_eps_udata_init;
	FR->udata_copy     = wmf_eps_udata_copy;
	FR->udata_set      = wmf_eps_udata_set;
	FR->udata_free     = wmf_eps_udata_free;
	FR->region_frame   = wmf_eps_region_frame;
	FR->region_paint   = wmf_eps_region_paint;
	FR->region_clip	   = wmf_eps_region_clip;

/* Allocate device data structure
 */
	ddata = (wmf_eps_t*) wmf_malloc (API,sizeof (wmf_eps_t));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	API->device_data = (void*) ddata;

/* Device data defaults
 */
	ddata->bbox.TL.x = 0;
	ddata->bbox.TL.y = 0;
	ddata->bbox.BR.x = 0;
	ddata->bbox.BR.y = 0;

	ddata->out = 0;

	ddata->Title = 0;
	ddata->Creator = 0;
	ddata->Date = 0;
	ddata->For = 0;

	ddata->eps_x = 0;
	ddata->eps_y = 0;
	ddata->eps_width = 0;
	ddata->eps_height = 0;

	ddata->page_width = 596;
	ddata->page_height = 842;

	ddata->flags = 0;
#else /* ! WITHOUT_LAYERS */
	API->device_data = 0;

	API->err = wmf_E_DeviceError;
#endif /* ! WITHOUT_LAYERS */
}
#ifndef WITHOUT_LAYERS
static void wmf_eps_draw_text (wmfAPI* API,wmfDrawText_t* draw_text)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	wmfStream* out = ddata->out;

	wmfFont* font = 0;

	wmfRGB* rgb = 0;

	float red;
	float green;
	float blue;
	float size;
	float ratio;
	float theta;

	unsigned int i;
	unsigned int length;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]draw_text");

	if (out == 0) return;

	size = (float) draw_text->font_height;

	ratio = (float) draw_text->font_ratio;

	font = WMF_DC_FONT (draw_text->dc);

	theta = (float) (WMF_TEXT_ANGLE (font) * 180 / PI);

	wmf_stream_printf (API,out,"gsave %% wmf_[eps_]draw_text\n");

	wmf_stream_printf (API,out,"/%s findfont %f scalefont setfont\n",WMF_FONT_PSNAME (font),size);

	wmf_stream_printf (API,out,"%f %f translate 1 -1 scale %f rotate ",draw_text->pt.x,draw_text->pt.y,theta);
	wmf_stream_printf (API,out,"%f 1 scale\n",ratio);

	wmf_stream_printf (API,out,"(");
	length = strlen (draw_text->str);
	for (i = 0; i < length; i++)
	{	if (draw_text->str[i] == ')') wmf_stream_printf (API,out,"\\)");
		else if (draw_text->str[i] == '(') wmf_stream_printf (API,out,"\\(");
		else wmf_stream_printf (API,out,"%c",draw_text->str[i]);
	}
	wmf_stream_printf (API,out,")\n");

	if (WMF_DC_OPAQUE (draw_text->dc))
	{	wmf_stream_printf (API,out,"dup stringwidth pop dup ");
		wmf_stream_printf (API,out,"newpath 0 %f moveto 0 rlineto 0 %f rlineto neg 0 rlineto closepath ",
		         - 0.29 * size,1.07 * size);

		rgb = WMF_DC_BACKGROUND (draw_text->dc);

		red   = (float) ((int) rgb->r) / 255;
		green = (float) ((int) rgb->g) / 255;
		blue  = (float) ((int) rgb->b) / 255;

		wmf_stream_printf (API,out,"%f %f %f setrgbcolor fill ",red,green,blue);
	}

	rgb = WMF_DC_TEXTCOLOR (draw_text->dc);

	red   = (float) ((int) rgb->r) / 255;
	green = (float) ((int) rgb->g) / 255;
	blue  = (float) ((int) rgb->b) / 255;

	wmf_stream_printf (API,out,"%f %f %f setrgbcolor ",red,green,blue);

	wmf_stream_printf (API,out,"0 0 moveto show\n");

	wmf_stream_printf (API,out,"grestore\n");
}

static void wmf_eps_udata_init (wmfAPI* API,wmfUserData_t* user_data)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]udata_init");

	
}

static void wmf_eps_udata_copy (wmfAPI* API,wmfUserData_t* user_data)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]udata_copy");

	
}

static void wmf_eps_udata_set (wmfAPI* API,wmfUserData_t* user_data)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]udata_set");

	
}

static void wmf_eps_udata_free (wmfAPI* API,wmfUserData_t* user_data)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]udata_free");

	
}

/* Write out postscript path, *minus* the ultimate `fill', then call eps_path_fill
 * Note: I KNOW there is a better way to do patterns, but my ghostscript hangs when I try!
 * 
 * Assumes that lbStyle != BS_NULL
 */
static void eps_path_fill (wmfAPI* API,wmfDC* dc,wmfD_Rect* bbox)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	wmfStream* out = ddata->out;

	wmfBrush* brush;

	wmfRGB* rgb;

	wmfBMP* bmp;

	float red;
	float green;
	float blue;

	float side;

	WMF_DEBUG (API,"~~~~~~~~eps_path_fill");

	if (out == 0) return;

	brush = WMF_DC_BRUSH (dc);

	switch (WMF_BRUSH_STYLE (brush))
	{
	case BS_NULL:
		WMF_ERROR (API,"Attempt to set null fill-style!");
		API->err = wmf_E_Glitch;
	break;

	case BS_HATCHED:
		wmf_stream_printf (API,out,"clip ");

		if (WMF_DC_OPAQUE (dc))
		{	rgb = WMF_DC_BACKGROUND (dc);

			red   = (float) ((int) rgb->r) / 255;
			green = (float) ((int) rgb->g) / 255;
			blue  = (float) ((int) rgb->b) / 255;

			wmf_stream_printf (API,out,"%f %f %f setrgbcolor ",red,green,blue);

			wmf_stream_printf (API,out,"fill ");
		}

		wmf_stream_printf (API,out,"\n");
		wmf_stream_printf (API,out,"1 setlinewidth ");
		wmf_stream_printf (API,out,"[] 0 setdash ");

		rgb = WMF_BRUSH_COLOR (brush);

		red   = (float) ((int) rgb->r) / 255;
		green = (float) ((int) rgb->g) / 255;
		blue  = (float) ((int) rgb->b) / 255;

		wmf_stream_printf (API,out,"%f %f %f setrgbcolor\n",red,green,blue);

		switch (WMF_BRUSH_HATCH (brush))
		{
		case HS_HORIZONTAL:
			wmf_stream_printf (API,out,"%f 5 %f { newpath dup %f exch moveto %f exch lineto stroke } for\n",
			         bbox->TL.y,bbox->BR.y,bbox->TL.x,bbox->BR.x);
		break;

		case HS_VERTICAL:
			wmf_stream_printf (API,out,"%f 5 %f { newpath dup %f moveto %f lineto stroke } for\n",
			         bbox->TL.x,bbox->BR.x,bbox->TL.y,bbox->BR.y);
		break;

		case HS_FDIAGONAL:
			wmf_stream_printf (API,out,"gsave %% HS_FDIAGONAL\n");
			wmf_stream_printf (API,out,"%f %f translate -45 rotate ",
			         bbox->TL.x-(bbox->BR.y-bbox->TL.y)/2,(bbox->TL.y+bbox->BR.y)/2);
			side = ((bbox->BR.x-bbox->TL.x) + (bbox->BR.y-bbox->TL.y)) / 1.41421356237309504880;
			wmf_stream_printf (API,out,"0 5 %f { newpath dup 0 moveto %f lineto stroke } for ",
			         side,side);
			wmf_stream_printf (API,out,"grestore\n");
		break;

		case HS_BDIAGONAL:
			wmf_stream_printf (API,out,"gsave %% HS_BDIAGONAL\n");
			wmf_stream_printf (API,out,"%f %f translate -45 rotate ",
			         bbox->TL.x-(bbox->BR.y-bbox->TL.y)/2,(bbox->TL.y+bbox->BR.y)/2);
			side = ((bbox->BR.x-bbox->TL.x) + (bbox->BR.y-bbox->TL.y)) / 1.41421356237309504880;
			wmf_stream_printf (API,out,"0 5 %f { newpath dup 0 exch moveto %f exch lineto stroke } for ",
			         side,side);
			wmf_stream_printf (API,out,"grestore\n");
		break;

		case HS_CROSS:
			wmf_stream_printf (API,out,"%f 5 %f { newpath dup %f exch moveto %f exch lineto stroke } for\n",
			         bbox->TL.y,bbox->BR.y,bbox->TL.x,bbox->BR.x);
			wmf_stream_printf (API,out,"%f 5 %f { newpath dup %f moveto %f lineto stroke } for\n",
			         bbox->TL.x,bbox->BR.x,bbox->TL.y,bbox->BR.y);
		break;

		case HS_DIAGCROSS:
			wmf_stream_printf (API,out,"gsave %% HS_DIAGCROSS\n");
			wmf_stream_printf (API,out,"%f %f translate -45 rotate ",
			         bbox->TL.x-(bbox->BR.y-bbox->TL.y)/2,(bbox->TL.y+bbox->BR.y)/2);
			side = ((bbox->BR.x-bbox->TL.x) + (bbox->BR.y-bbox->TL.y)) / 1.41421356237309504880;
			wmf_stream_printf (API,out,"0 5 %f { newpath dup 0 moveto %f lineto stroke } for ",
			         side,side);
			wmf_stream_printf (API,out,"0 5 %f { newpath dup 0 exch moveto %f exch lineto stroke } for ",
			         side,side);
			wmf_stream_printf (API,out,"grestore\n");
		break;

		default:
			if (API->flags & WMF_OPT_IGNORE_NONFATAL)
			{	WMF_DEBUG (API,"Unsupported brush/hatch style!");
			}
			else
			{	WMF_ERROR (API,"Unsupported brush/hatch style!");
				API->err = wmf_E_Glitch;
			}
		break;
		}
	break;

	case BS_DIBPATTERN:
		bmp = WMF_BRUSH_BITMAP (brush);
		if (bmp->data == 0)
		{	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
			{	WMF_DEBUG (API,"Attempt to fill with non-existent pattern!");
			}
			else
			{	WMF_ERROR (API,"Attempt to fill with non-existent pattern!");
				API->err = wmf_E_Glitch;
				break;
			}
		}
		/* no break here - TODO: implement bitmap fill */
	default:
		if (API->flags & WMF_OPT_IGNORE_NONFATAL)
		{	WMF_DEBUG (API,"Unsupported brush style!");
			/* no break here */
		}
		else
		{	WMF_ERROR (API,"Unsupported brush style!");
			API->err = wmf_E_Glitch;
			break;
		}
	case BS_SOLID:
		rgb = WMF_BRUSH_COLOR (brush);

		red   = (float) ((int) rgb->r) / 255;
		green = (float) ((int) rgb->g) / 255;
		blue  = (float) ((int) rgb->b) / 255;

		wmf_stream_printf (API,out,"%f %f %f setrgbcolor fill\n",red,green,blue);
	break;
	}
}

/* Write out postscript path, *minus* the ultimate `stroke', then call eps_path_stroke
 * 
 * Assumes that lopnStyle != PS_NULL
 */
static void eps_path_stroke (wmfAPI* API,wmfDC* dc,float linewidth)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	wmfStream* out = ddata->out;

	wmfPen* pen = 0;

	wmfRGB* rgb = 0;

	float red;
	float green;
	float blue;

	WMF_DEBUG (API,"~~~~~~~~eps_path_stroke");

	if (out == 0) return;

	wmf_stream_printf (API,out,"\n");

	wmf_stream_printf (API,out,"%f setlinewidth ",linewidth);

	pen = WMF_DC_PEN (dc);

	rgb = WMF_PEN_COLOR (pen);

	red   = (float) ((int) rgb->r) / 255;
	green = (float) ((int) rgb->g) / 255;
	blue  = (float) ((int) rgb->b) / 255;

	wmf_stream_printf (API,out,"%f %f %f setrgbcolor ",red,green,blue);

	switch (WMF_PEN_ENDCAP (pen))
	{
	case PS_ENDCAP_SQUARE:
		wmf_stream_printf (API,out,"2 setlinecap ");
	break;

	case PS_ENDCAP_ROUND:
		wmf_stream_printf (API,out,"1 setlinecap ");
	break;

	case PS_ENDCAP_FLAT:
	default:
		wmf_stream_printf (API,out,"0 setlinecap ");
	break;
	}

	switch (WMF_PEN_JOIN (pen))
	{
	case PS_JOIN_BEVEL:
		wmf_stream_printf (API,out,"2 setlinejoin ");
	break;

	case PS_JOIN_ROUND:
		wmf_stream_printf (API,out,"1 setlinejoin ");
	break;

	case PS_JOIN_MITER:
	default:
		wmf_stream_printf (API,out,"0 setlinejoin ");
	break;
	}

	switch (WMF_PEN_STYLE (pen))
	{
	case PS_DASH: /* DASH_LINE */
		wmf_stream_printf (API,out,"[ %f %f ] 0 setdash ",
		         linewidth*10,linewidth*10);
	break;

	case PS_ALTERNATE:
	case PS_DOT: /* DOTTED_LINE */
		wmf_stream_printf (API,out,"[ %f %f ] 0 setdash ",
		         linewidth,linewidth*2);
	break;

	case PS_DASHDOT: /* DASH_DOT_LINE */
		wmf_stream_printf (API,out,"[ %f %f %f %f ] 0 setdash ",
		         linewidth*10,linewidth*2,linewidth,linewidth*2);
	break;

	case PS_DASHDOTDOT: /* DASH_2_DOTS_LINE */
		wmf_stream_printf (API,out,"[ %f %f %f %f %f %f ] 0 setdash ",
		         linewidth*10,linewidth*2,linewidth,linewidth*2,linewidth,linewidth*2);
	break;

	case PS_INSIDEFRAME: /* There is nothing to do in this case... */
	case PS_SOLID:
	default:
		wmf_stream_printf (API,out,"[] 0 setdash ");
	break;
	}

	wmf_stream_printf (API,out,"stroke\n");
}
#endif /* ! WITHOUT_LAYERS */

