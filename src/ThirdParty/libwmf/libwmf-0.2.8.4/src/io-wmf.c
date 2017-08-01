/* -*- Mode: C; tab-width: 8; c-basic-offset: 8 -*- */

/* GdkPixbuf library - WMF image loader
 *
 * Copyright (C) 2002 Dom Lachowicz
 * Copyright (C) 2002 Francis James Franklin
 *
 * Authors: Dom Lachowicz <cinamod@hotmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* This module is based on AbiWord and wvWare code and ideas done by
 * Francis James Franklin <fjf@alinameridon.com> and io-svg.c in the
 * librsvg module by Matthias Clasen <maclas@gmx.de>
 */

#include <stdio.h>
#include <stdlib.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <gdk-pixbuf/gdk-pixbuf-io.h>

#include "libwmf/api.h"
#include "libwmf/gd.h"

G_MODULE_EXPORT void fill_vtable (GdkPixbufModule *module);
G_MODULE_EXPORT void fill_info (GdkPixbufFormat *info);

typedef struct {
	GdkPixbufModuleUpdatedFunc  updated_func;
	GdkPixbufModulePreparedFunc prepared_func;
	GdkPixbufModuleSizeFunc     size_func;
	gpointer                    user_data;

	GByteArray                 *data;
} WmfContext;

static void
pixbuf_destroy_function (guchar * pixels, gpointer data)
{
	g_free (pixels);
}

static guchar *
pixbuf_gd_convert (int * gd_pixels, gint width, gint height)
{
	guchar  * pixels = NULL;
	guchar  * px_ptr = NULL;
	int     * gd_ptr = gd_pixels;
	gint      i = 0;
	gint      j = 0;
	gsize     alloc_size = width * height * sizeof (guchar) * 4;

	unsigned int pixel;
	guchar       r,g,b,a;

	pixels = (guchar *) g_try_malloc (alloc_size);
	if (pixels == NULL) return NULL;

	px_ptr = pixels;

	for (i = 0; i < height; i++)
		for (j = 0; j < width; j++)
		{
			pixel = (unsigned int) (*gd_ptr++);

			b = (guchar) (pixel & 0xff);
			pixel >>= 8;
			g = (guchar) (pixel & 0xff);
			pixel >>= 8;
			r = (guchar) (pixel & 0xff);
			pixel >>= 7;
			a = (guchar) (pixel & 0xfe);
			a ^= 0xff;

			*px_ptr++ = r;
			*px_ptr++ = g;
			*px_ptr++ = b;
			*px_ptr++ = a;
		}
	return pixels;
}

static gboolean
gdk_pixbuf__wmf_image_stop_load (gpointer data, GError **error)
{
	/* will be used to represent the WMF in-memory
	 * during the decoding process
	 */
	WmfContext      *context = (WmfContext *)data;  
	gboolean         result  = FALSE;

	guchar           *pixels   = NULL;
	GdkPixbuf        *pixbuf   = NULL;
 
	/* the bits we need to decode the WMF via libwmf2's GD layer
	 */
	wmf_error_t       err;
	unsigned long     flags;        
	wmf_gd_t         *ddata = NULL;        
	wmfAPI           *API   = NULL;        
	wmfAPI_Options    api_options;        
	wmfD_Rect         bbox;

	int              *gd_pixels = NULL;
        
	/* the natural width and height of the WMF or the user-specified with+height
	 */
	gint              width, height;

	double            resolution_x, resolution_y;

	if (error != NULL)
		*error = NULL;

	/* TODO: be smarter about getting the resolution from screen 
	 */
	resolution_x = resolution_y = 72.0;
	width = height = -1;

	flags = WMF_OPT_IGNORE_NONFATAL | WMF_OPT_FUNCTION;
	api_options.function = wmf_gd_function;

	err = wmf_api_create (&API, flags, &api_options);
	if (err != wmf_E_None) {
		g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY,
			     "Couldn't create WMF reader");
		goto _wmf_error;
	}

	ddata = WMF_GD_GetData (API);
	ddata->type = wmf_gd_image;

	err = wmf_mem_open (API, context->data->data, context->data->len);
	if (err != wmf_E_None) {
		g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_FAILED,
			     "Couldn't create reader API");
		goto _wmf_error;
	}
        
	err = wmf_scan (API, 0, &bbox);
	if (err != wmf_E_None) {
		g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
			     "Error scanning WMF file");
		goto _wmf_error;
	}

	/* find out how large the app wants the pixbuf to be
	 */
	if (context->size_func != NULL)
		(*context->size_func) (&width, &height, context->user_data);

	/* if these are <= 0, assume user wants the natural size of the wmf
	 */
	if (width <= 0 || height <= 0) {
		err = wmf_display_size (API, &width, &height, resolution_x, resolution_y);

		if (err != wmf_E_None || width <= 0 || height <= 0) {
			g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
				     "Couldn't determine image size");
			goto _wmf_error;
		}
	}

	ddata->bbox          = bbox;        
	ddata->width         = width;
	ddata->height        = height;
        
	err = wmf_play (API, 0, &bbox);
	if (err != wmf_E_None) {
		g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
			     "Couldn't decode WMF file into pixbuf");
		goto _wmf_error;
	}

	wmf_mem_close (API);

	if (ddata->gd_image != NULL) gd_pixels = wmf_gd_image_pixels (ddata->gd_image);
	if (gd_pixels == NULL) {
		g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
			     "Couldn't decode WMF file - no output (huh?)");
		goto _wmf_error;
	}

	pixels = pixbuf_gd_convert (gd_pixels, width, height);
	if (pixels == NULL) {
		g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_INSUFFICIENT_MEMORY,
			     "Couldn't create RGBA buffer");
		goto _wmf_error;
	}

	wmf_api_destroy (API);
	API = NULL;

	pixbuf = gdk_pixbuf_new_from_data (pixels, GDK_COLORSPACE_RGB, TRUE, 8,
					   width, height, width * 4,
					   pixbuf_destroy_function, NULL);
	if (pixbuf == NULL) {
		g_set_error (error, GDK_PIXBUF_ERROR, GDK_PIXBUF_ERROR_CORRUPT_IMAGE,
			     "Couldn't decode WMF file");
		goto _wmf_error;
	}

	if (context->prepared_func != NULL)
		(* context->prepared_func) (pixbuf, NULL, context->user_data);

	if (context->updated_func != NULL)
		(* context->updated_func) (pixbuf, 0, 0, 
					   gdk_pixbuf_get_width (pixbuf), 
					   gdk_pixbuf_get_height (pixbuf), 
					   context->user_data);
	result = TRUE;

 _wmf_error:

	if (API != NULL) wmf_api_destroy (API);

	g_byte_array_free (context->data, TRUE);
	g_free (context);

	return result;
}

static gboolean
gdk_pixbuf__wmf_image_load_increment (gpointer data,
				      const guchar *buf, guint size,
				      GError **error)
{
	WmfContext *context = (WmfContext *)data;

	if (error != NULL)
		*error = NULL;
	g_byte_array_append (context->data, buf, size);
	return TRUE;
}

static gpointer
gdk_pixbuf__wmf_image_begin_load (GdkPixbufModuleSizeFunc size_func,
				  GdkPixbufModulePreparedFunc prepared_func, 
				  GdkPixbufModuleUpdatedFunc  updated_func,
				  gpointer user_data,
				  GError **error)
{
	WmfContext *context    = g_new0 (WmfContext, 1);

	if (error != NULL)
		*error = NULL;

	context->prepared_func = prepared_func;
	context->updated_func  = updated_func;
	context->size_func     = size_func;
	context->user_data     = user_data;        
	context->data          = g_byte_array_new ();

	return (gpointer)context;
}

void
fill_vtable (GdkPixbufModule *module)
{
	module->begin_load     = gdk_pixbuf__wmf_image_begin_load;
	module->stop_load      = gdk_pixbuf__wmf_image_stop_load;
	module->load_increment = gdk_pixbuf__wmf_image_load_increment;
}

void
fill_info (GdkPixbufFormat *info)
{
	/* Magic header is sometimes: "\327\315\306\232" or "\001\000\011\000"
	 * Impossible to recognize all WMFs this way, as not all WMFs have headers
	 */
	static GdkPixbufModulePattern signature[] = {
		{ "\xd7\xcd\xc6\x9a", NULL, 100 },
		{ "\x01\x00\x09\x00", NULL, 100 },
		{ NULL, NULL, 0 }
	};
	static gchar *mime_types[] = { 
		"image/x-wmf",
		NULL 
	};
	static gchar *extensions[] = { 
		"wmf", 
		"apm",
		NULL 
	};

	info->name        = "wmf";
	info->signature   = signature;
	info->description = "Windows Metafile";
	info->mime_types  = mime_types;
	info->extensions  = extensions;
	info->flags       = 0;
}
