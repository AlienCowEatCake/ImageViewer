/* libwmf (ipa/x.c): library for wmf conversion
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

#include "wmfdefs.h"

#ifdef X_DISPLAY_MISSING

void wmf_x_function (wmfAPI* API)
{	API->err = wmf_E_DeviceError;

	WMF_DEBUG (API,"No support for X");
}

#else /* X_DISPLAY_MISSING */ /* i.e., not */

#include <math.h>

#include "libwmf/x.h"

static void wmf_x_device_open (wmfAPI*);
static void wmf_x_device_close (wmfAPI*);
static void wmf_x_device_begin (wmfAPI*);
static void wmf_x_device_end (wmfAPI*);
static void wmf_x_flood_interior (wmfAPI*,wmfFlood_t*);
static void wmf_x_flood_exterior (wmfAPI*,wmfFlood_t*);
static void wmf_x_draw_pixel (wmfAPI*,wmfDrawPixel_t*);
static void wmf_x_draw_pie (wmfAPI*,wmfDrawArc_t*);
static void wmf_x_draw_chord (wmfAPI*,wmfDrawArc_t*);
static void wmf_x_draw_arc (wmfAPI*,wmfDrawArc_t*);
static void wmf_x_draw_ellipse (wmfAPI*,wmfDrawArc_t*);
static void wmf_x_draw_line (wmfAPI*,wmfDrawLine_t*);
static void wmf_x_poly_line (wmfAPI*,wmfPolyLine_t*);
static void wmf_x_draw_polygon (wmfAPI*,wmfPolyLine_t*);
static void wmf_x_draw_rectangle (wmfAPI*,wmfDrawRectangle_t*);
static void wmf_x_rop_draw (wmfAPI*,wmfROP_Draw_t*);
static void wmf_x_bmp_draw (wmfAPI*,wmfBMP_Draw_t*);
static void wmf_x_bmp_read (wmfAPI*,wmfBMP_Read_t*);
static void wmf_x_bmp_free (wmfAPI*,wmfBMP*);
static void wmf_x_draw_text (wmfAPI*,wmfDrawText_t*);
static void wmf_x_udata_init (wmfAPI*,wmfUserData_t*);
static void wmf_x_udata_copy (wmfAPI*,wmfUserData_t*);
static void wmf_x_udata_set (wmfAPI*,wmfUserData_t*);
static void wmf_x_udata_free (wmfAPI*,wmfUserData_t*);
static void wmf_x_region_frame (wmfAPI*,wmfPolyRectangle_t*);
static void wmf_x_region_paint (wmfAPI*,wmfPolyRectangle_t*);
static void wmf_x_region_clip (wmfAPI*,wmfPolyRectangle_t*);

#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include "ipa/x.h"
#include "ipa/x/bmp.h"
#include "ipa/x/color.h"
#include "ipa/x/device.h"
#include "ipa/x/draw.h"
#include "ipa/x/font.h"
#include "ipa/x/region.h"

void wmf_x_function (wmfAPI* API)
{	wmf_x_t* ddata = 0;

	wmfFunctionReference* FR = (wmfFunctionReference*) API->function_reference;

	if ((API->flags & API_STANDARD_INTERFACE) == 0)
	{	WMF_ERROR (API,"Can't use this device layer with 'lite' interface!");
		API->err = wmf_E_DeviceError;
		return;
	}

/* IPA function reference links
 */
	FR->device_open    = wmf_x_device_open;
	FR->device_close   = wmf_x_device_close;
	FR->device_begin   = wmf_x_device_begin;
	FR->device_end     = wmf_x_device_end;
	FR->flood_interior = wmf_x_flood_interior;
	FR->flood_exterior = wmf_x_flood_exterior;
	FR->draw_pixel     = wmf_x_draw_pixel;
	FR->draw_pie       = wmf_x_draw_pie;
	FR->draw_chord     = wmf_x_draw_chord;
	FR->draw_arc       = wmf_x_draw_arc;
	FR->draw_ellipse   = wmf_x_draw_ellipse;
	FR->draw_line      = wmf_x_draw_line;
	FR->poly_line      = wmf_x_poly_line;
	FR->draw_polygon   = wmf_x_draw_polygon;
	FR->draw_rectangle = wmf_x_draw_rectangle;
	FR->rop_draw       = wmf_x_rop_draw;
	FR->bmp_draw       = wmf_x_bmp_draw;
	FR->bmp_read       = wmf_x_bmp_read;
	FR->bmp_free       = wmf_x_bmp_free;
	FR->draw_text      = wmf_x_draw_text;
	FR->udata_init     = wmf_x_udata_init;
	FR->udata_copy     = wmf_x_udata_copy;
	FR->udata_set      = wmf_x_udata_set;
	FR->udata_free     = wmf_x_udata_free;
	FR->region_frame   = wmf_x_region_frame;
	FR->region_paint   = wmf_x_region_paint;
	FR->region_clip    = wmf_x_region_clip;

/* Allocate device data structure
 */
	ddata = (wmf_x_t*) wmf_malloc (API,sizeof (wmf_x_t));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	API->device_data = (void*) ddata;

/* Device data defaults
 */
	ddata->display_name = 0;
	ddata->window_name = 0;
	ddata->icon_name = 0;

	ddata->display = 0;

	ddata->root = None;
	ddata->window = None;
	ddata->pixmap = None;
	ddata->hatch = None;
	ddata->brush = None;

	ddata->gc = 0;

	ddata->color = 0;

	ddata->bbox.TL.x = 0;
	ddata->bbox.TL.y = 0;
	ddata->bbox.BR.x = 0;
	ddata->bbox.BR.y = 0;

	ddata->flags = 0;
}

static void wmf_x_udata_init (wmfAPI* API,wmfUserData_t* userdata)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]udata_init");

}

static void wmf_x_udata_copy (wmfAPI* API,wmfUserData_t* userdata)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]udata_copy");

}

static void wmf_x_udata_set (wmfAPI* API,wmfUserData_t* userdata)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]udata_set");

}

static void wmf_x_udata_free (wmfAPI* API,wmfUserData_t* userdata)
{	/* wmf_x_t* ddata = WMF_X_GetData (API); */

	WMF_DEBUG (API,"wmf_[x_]udata_free");

}

static void setbrushstyle (wmfAPI* API,wmfDC* dc)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	wmfBMP* bmp = 0;

	wmfRGB pixel;

	wmfBrush* brush = 0;

	int opacity;
	int fill_style;

	U16 i;
	U16 j;

	XGCValues values;

	brush = WMF_DC_BRUSH (dc);

	values.function = Our_XROPfunction[WMF_DC_ROP (dc) - 1];

	if (values.function == GXinvert)
	{	values.function = GXxor;
		values.foreground = ddata->black;
	}
	else
	{	values.foreground = get_color (API,WMF_BRUSH_COLOR (brush));
	}

	values.background = get_color (API,WMF_DC_BACKGROUND (dc));

	switch (WMF_BRUSH_STYLE (brush))
	{	/* TODO: these arrays are convenient but how SEG-safe are they? */
	case BS_HATCHED:
		if (ddata->hatch != None) XFreePixmap (ddata->display,ddata->hatch);
		ddata->hatch = XCreateBitmapFromData (ddata->display,ddata->root,
		                                      HatchBrushes[WMF_BRUSH_HATCH (brush)],8,8);
		fill_style = ((WMF_DC_OPAQUE (dc)) ? FillOpaqueStippled : FillStippled);
		XSetStipple (ddata->display,ddata->gc,ddata->hatch);
	break;

	case BS_DIBPATTERN:
		setdefaultstyle (API);

		bmp = WMF_BRUSH_BITMAP (brush);

		if (ddata->brush != None)
		{	XFreePixmap (ddata->display,ddata->brush);
			ddata->brush = None;
		}
		if (bmp->data == 0)
		{	fill_style = FillSolid;
			break;
		}
		ddata->brush = XCreatePixmap (ddata->display,ddata->root,
		                              bmp->width,bmp->height,ddata->depth);
		if (ddata->brush == None)
		{	fill_style = FillSolid;
			break;
		}

		for (j = 0; j < bmp->height; j++)
		{	for (i = 0; i < bmp->width; i++)
			{	opacity = wmf_ipa_bmp_color (API,bmp,&pixel,i,j);

				XSetForeground (ddata->display,ddata->gc,get_color (API,&pixel));

				XDrawPoint (ddata->display,ddata->brush,ddata->gc,i,j);
			}
		}

		XSetTile (ddata->display,ddata->gc,ddata->brush);
		fill_style = FillTiled;
	break;

	case BS_NULL:
	case BS_SOLID:
	case BS_PATTERN:
	default:
		fill_style = FillSolid;
	break;
	}

	XSetFillStyle (ddata->display,ddata->gc,fill_style);

	switch (WMF_DC_POLYFILL (dc))
	{
	case ALTERNATE:
		values.fill_rule = EvenOddRule;
	break;

	case WINDING:
		values.fill_rule = WindingRule;
	break;

	default:
		values.fill_rule = EvenOddRule;
	break;
	}

	XChangeGC (ddata->display,ddata->gc,GCFunction|GCForeground|GCBackground|GCFillRule,&values);
}

static void setlinestyle (wmfAPI* API,wmfDC* dc)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	wmfPen* pen = 0;

	unsigned long color;

	char* dashes = 0;

	int dash_len;

	XGCValues values;

	pen = WMF_DC_PEN (dc);

	color = get_color (API,WMF_PEN_COLOR (pen));

	switch (WMF_DC_ROP (dc))
	{
	case R2_BLACK:
		values.foreground = ddata->black;
		values.function = GXcopy;
	break;

	case R2_WHITE:
		values.foreground = ddata->white;
		values.function = GXcopy;
	break;

	case R2_XORPEN:
		values.foreground = color; /* It is very unlikely someone wants to XOR with 0 */
		                           /* This fixes the rubber-drawings in paintbrush    */
		if (values.foreground == 0)
		{	values.foreground = ((ddata->white) ? ddata->white : ddata->black);
		}
		values.function = GXxor;
	break;

	default:
		values.foreground = color;
		values.function = Our_XROPfunction[WMF_DC_ROP (dc) - 1];
	break;
	}

	switch (WMF_PEN_STYLE (pen))
	{
	case PS_DASH:
		dashes = (char*) PEN_dash;
		dash_len = 2;
		values.line_style = ((WMF_DC_OPAQUE (dc)) ? LineDoubleDash : LineOnOffDash);
	break;

	case PS_DOT:
		dashes = (char*) PEN_dot;
		dash_len = 2;
		values.line_style = ((WMF_DC_OPAQUE (dc)) ? LineDoubleDash : LineOnOffDash);
	break;

	case PS_DASHDOT:
		dashes = (char*) PEN_dashdot;
		dash_len = 4;
		values.line_style = ((WMF_DC_OPAQUE (dc)) ? LineDoubleDash : LineOnOffDash);
	break;

	case PS_DASHDOTDOT:
		dashes = (char*) PEN_dashdotdot;
		dash_len = 6;
		values.line_style = ((WMF_DC_OPAQUE (dc)) ? LineDoubleDash : LineOnOffDash);
	break;

	case PS_ALTERNATE: /* FIXME: should be alternating _pixels_ that are set */
		dashes = (char*) PEN_alternate;
		dash_len = 2;
		values.line_style = ((WMF_DC_OPAQUE (dc)) ? LineDoubleDash : LineOnOffDash);
	break;

	case PS_USERSTYLE: /* FIXME */
	default:
		dashes = 0;
		dash_len = 2;
		values.line_style = LineSolid;
	break;
	}

	values.fill_style = FillSolid;

	values.line_width = (int) ceil ( (double) x_width  (API,WMF_PEN_WIDTH  (pen))
	                               + (double) x_height (API,WMF_PEN_HEIGHT (pen)) );

	if (values.line_width < 1) values.line_width = 1;

	switch (WMF_PEN_ENDCAP (pen))
	{
	case PS_ENDCAP_SQUARE:
		values.cap_style = CapProjecting;
	break;

	case PS_ENDCAP_FLAT:
		values.cap_style = CapButt;
	break;

	case PS_ENDCAP_ROUND:
	default:
		values.cap_style = CapRound;
	break;
	}

	switch (WMF_PEN_JOIN (pen))
	{
	case PS_JOIN_BEVEL:
		values.join_style = JoinBevel;
	break;

	case PS_JOIN_MITER:
		values.join_style = JoinMiter;
	break;

	case PS_JOIN_ROUND:
	default:
		values.join_style = JoinRound;
	break;
	}

	if (dashes) XSetDashes (ddata->display,ddata->gc,0,dashes,dash_len);

	values.background = get_color (API,WMF_DC_BACKGROUND (dc));

	XChangeGC (ddata->display,ddata->gc,
	           GCFunction | GCForeground | GCLineStyle | GCLineWidth |
	           GCCapStyle | GCBackground | GCJoinStyle | GCFillStyle,
	           &values);
}

static void setdefaultstyle (wmfAPI* API)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XGCValues values;

	values.function = GXcopy;

	values.foreground = ddata->black;
	values.background = ddata->white;

	values.fill_style = FillSolid;
	values.fill_rule = EvenOddRule;

	values.cap_style = CapRound;
	values.join_style = JoinRound;
	values.line_style = LineSolid;
	values.line_width = 1;

	XChangeGC (ddata->display,ddata->gc,
	           GCFunction | GCForeground | GCLineStyle | GCJoinStyle |
	           GCCapStyle | GCBackground | GCLineWidth | GCFillStyle | GCFillRule,
	           &values);
}

#endif /* X_DISPLAY_MISSING */
