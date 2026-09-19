/* libwmf (ipa/template.c): library for wmf conversion
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

/* If compiling as module, rename global functions to a standard
 */
#ifdef WMF_IPA_MODULE
#define wmf_template_device_open    wmf_device_open
#define wmf_template_device_close   wmf_device_close
#define wmf_template_device_begin   wmf_device_begin
#define wmf_template_device_end     wmf_device_end
#define wmf_template_flood_interior wmf_flood_interior
#define wmf_template_flood_exterior wmf_flood_exterior
#define wmf_template_draw_pixel     wmf_draw_pixel
#define wmf_template_draw_pie       wmf_draw_pie
#define wmf_template_draw_chord     wmf_draw_chord
#define wmf_template_draw_arc       wmf_draw_arc
#define wmf_template_draw_ellipse   wmf_draw_ellipse
#define wmf_template_draw_line      wmf_draw_line
#define wmf_template_poly_line      wmf_poly_line
#define wmf_template_draw_polygon   wmf_draw_polygon
#define wmf_template_draw_rectangle wmf_draw_rectangle
#define wmf_template_rop_draw       wmf_rop_draw
#define wmf_template_bmp_draw       wmf_bmp_draw
#define wmf_template_bmp_read       wmf_bmp_read
#define wmf_template_bmp_free       wmf_bmp_free
#define wmf_template_draw_text      wmf_draw_text
#define wmf_template_udata_init     wmf_udata_init
#define wmf_template_udata_copy     wmf_udata_copy
#define wmf_template_udata_set      wmf_udata_set
#define wmf_template_udata_free     wmf_udata_free
#define wmf_template_region_frame   wmf_region_frame
#define wmf_template_region_paint   wmf_region_paint
#define wmf_template_region_clip    wmf_region_clip
#endif /* WMF_IPA_MODULE */

#include "wmfdefs.h"

#include "libwmf/template.h"

#include "ipa/template.h"

void wmf_template_function (wmfAPI* API)
{	wmf_template_t* ddata = 0;

	wmfFunctionReference* FR = (wmfFunctionReference*) API->function_reference;

/* IPA function reference links
 */
	FR->device_open    = wmf_template_device_open;
	FR->device_close   = wmf_template_device_close;
	FR->device_begin   = wmf_template_device_begin;
	FR->device_end     = wmf_template_device_end;
	FR->flood_interior = wmf_template_flood_interior;
	FR->flood_exterior = wmf_template_flood_exterior;
	FR->draw_pixel     = wmf_template_draw_pixel;
	FR->draw_pie       = wmf_template_draw_pie;
	FR->draw_chord     = wmf_template_draw_chord;
	FR->draw_arc       = wmf_template_draw_arc;
	FR->draw_ellipse   = wmf_template_draw_ellipse;
	FR->draw_line      = wmf_template_draw_line;
	FR->poly_line      = wmf_template_poly_line;
	FR->draw_polygon   = wmf_template_draw_polygon;
	FR->draw_rectangle = wmf_template_draw_rectangle;
	FR->rop_draw       = wmf_template_rop_draw;
	FR->bmp_draw       = wmf_template_bmp_draw;
	FR->bmp_read       = wmf_template_bmp_read;
	FR->bmp_free       = wmf_template_bmp_free;
	FR->draw_text      = wmf_template_draw_text;
	FR->udata_init     = wmf_template_udata_init;
	FR->udata_copy     = wmf_template_udata_copy;
	FR->udata_set      = wmf_template_udata_set;
	FR->udata_free     = wmf_template_udata_free;
	FR->region_frame   = wmf_template_region_frame;
	FR->region_paint   = wmf_template_region_paint;
	FR->region_clip    = wmf_template_region_clip;

/* Allocate device data structure
 */
	ddata = (wmf_template_t*) wmf_malloc (API,sizeof (wmf_template_t));

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

	ddata->flags = 0;
}

/* This is called by wmf_play() the *first* time the meta file is played
 */
void wmf_template_device_open (wmfAPI* API)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]device_open");

	
}

/* This is called by wmf_api_destroy()
 */
void wmf_template_device_close (wmfAPI* API)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]device_close");

	
}

/* This is called from the beginning of each play for initial page setup
 */
void wmf_template_device_begin (wmfAPI* API)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]device_begin");

	
}

/* This is called from the end of each play for page termination
 */
void wmf_template_device_end (wmfAPI* API)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]device_end");

	
}

void wmf_template_flood_interior (wmfAPI* API,wmfFlood_t* flood)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]flood_interior");

	
}

void wmf_template_flood_exterior (wmfAPI* API,wmfFlood_t* flood)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]flood_exterior");

	
}

void wmf_template_draw_pixel (wmfAPI* API,wmfDrawPixel_t* drawpixel)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_pixel");

	
}

void wmf_template_draw_pie (wmfAPI* API,wmfDrawArc_t* drawarc)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_pie");

	
}

void wmf_template_draw_chord (wmfAPI* API,wmfDrawArc_t* drawarc)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_chord");

	
}

void wmf_template_draw_arc (wmfAPI* API,wmfDrawArc_t* drawarc)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_arc");

	
}

void wmf_template_draw_ellipse (wmfAPI* API,wmfDrawArc_t* drawarc)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_ellipse");

	
}

void wmf_template_draw_line (wmfAPI* API,wmfDrawLine_t* drawline)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_line");

	
}

void wmf_template_poly_line (wmfAPI* API,wmfPolyLine_t* polyline)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]poly_line");

	
}

void wmf_template_draw_polygon (wmfAPI* API,wmfPolyLine_t* polyline)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_polygon");

	
}

void wmf_template_draw_rectangle (wmfAPI* API,wmfDrawRectangle_t* drawrectangle)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_rectangle");

	
}

void wmf_template_rop_draw (wmfAPI* API,wmfROP_Draw_t* ropdraw)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]rop_draw");

	
}

void wmf_template_bmp_draw (wmfAPI* API,wmfBMP_Draw_t* bmpdraw)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]bmp_draw");

	
}

void wmf_template_bmp_read (wmfAPI* API,wmfBMP_Read_t* bmpread)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]bmp_read");

	
}

void wmf_template_bmp_free (wmfAPI* API,wmfBMP* bmp)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]bmp_free");

	
}

void wmf_template_draw_text (wmfAPI* API,wmfDrawText_t* drawtext)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]draw_text");

	
}

void wmf_template_udata_init (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]udata_init");

	
}

void wmf_template_udata_copy (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]udata_copy");

	
}

void wmf_template_udata_set (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]udata_set");

	
}

void wmf_template_udata_free (wmfAPI* API,wmfUserData_t* userdata)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]udata_free");

	
}

void wmf_template_region_frame (wmfAPI* API,wmfPolyRectangle_t* polyrectangle)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]region_frame");

	
}

void wmf_template_region_paint (wmfAPI* API,wmfPolyRectangle_t* polyrectangle)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]region_paint");

	
}

void wmf_template_region_clip (wmfAPI* API,wmfPolyRectangle_t* polyrectangle)
{	wmf_template_t* ddata = WMF_TEMPLATE_GetData (API);

	WMF_DEBUG (API,"wmf_[template_]region_clip");

	
}
