/* libwmf ("ipa/x/device.h"): library for wmf conversion
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


/* This is called by wmf_play() the *first* time the meta file is played
 */
static void wmf_x_device_open (wmfAPI* API)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XWindowAttributes root_attributes;

	XTextProperty wText;
	XSizeHints sizehints;

	XSetWindowAttributes attributes;

	unsigned int border;

	unsigned long valuemask;

	static char* default_name = "libwmf (x)";

	WMF_DEBUG (API,"wmf_[x_]device_open");

	if ((!(ddata->x_width > 0)) || (!(ddata->x_height > 0)))
	{	WMF_ERROR (API,"window/pixmap has unset or bad size.");
		API->err = wmf_E_Glitch;
		return;
	}

	if (ddata->flags & WMF_X_DISPLAY_OPEN)
	{	ddata->flags &= ~WMF_X_DISPLAY_CLOSE;
		ddata->flags &= ~WMF_X_WINDOW_OPEN;
		ddata->flags &= ~WMF_X_WINDOW_CLOSE;
		ddata->flags &= ~WMF_X_PIXMAP_OPEN;
		ddata->flags &= ~WMF_X_PIXMAP_CLOSE;

		ddata->flags &= ~WMF_X_CMAP_DESTROY;

		ddata->display = XOpenDisplay (ddata->display_name);

		if (ddata->display == 0)
		{	WMF_ERROR (API,"unable to open display!");
			API->err = wmf_E_DeviceError;
			return;
		}

		ddata->flags |= WMF_X_DISPLAY_CLOSE;
		ddata->flags |= WMF_X_WINDOW_OPEN;
		ddata->flags |= WMF_X_PIXMAP_OPEN;
	}
	else /* display already open */
	{	if (ddata->display == 0)
		{	WMF_ERROR (API,"display not open!");
			API->err = wmf_E_DeviceError;
			return;
		}
	}

	ddata->root = RootWindow (ddata->display,DefaultScreen (ddata->display));

	XGetWindowAttributes (ddata->display,ddata->root,&root_attributes);

	ddata->visual = root_attributes.visual;
	ddata->depth  = root_attributes.depth;

#if defined(__cplusplus) || defined(c_plusplus)
	ddata->class = ddata->visual->c_class;  /* C++ class of screen (monochrome, etc.) */
#else
	ddata->class = ddata->visual->class;    /*     class of screen (monochrome, etc.) */
#endif

	setup_color (API);

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	ddata->gc = XCreateGC (ddata->display,ddata->root,0,0);

	if (ddata->gc == 0)
	{	WMF_ERROR (API,"unable to create gc!");
		API->err = wmf_E_DeviceError;
		return;
	}

	XSetForeground (ddata->display,ddata->gc,ddata->white);

	if (ddata->flags & WMF_X_WINDOW_OPEN)
	{	ddata->flags &= ~WMF_X_WINDOW_CLOSE;

		border = 0; /* Default border size. */

		valuemask = 0;

		valuemask |= CWBackPixel;
		attributes.background_pixel = ddata->white;

		valuemask |= CWColormap;
		attributes.colormap = ddata->colormap;

		ddata->window = XCreateWindow (ddata->display,ddata->root,0,0,
		                               ddata->x_width,ddata->x_height,border,ddata->depth,
		                               InputOutput,ddata->visual,valuemask,&attributes);

		if (ddata->window == None)
		{	WMF_ERROR (API,"unable to create window!");
			API->err = wmf_E_DeviceError;
			return;
		}

		ddata->flags |= WMF_X_WINDOW_CLOSE;

		XFillRectangle (ddata->display,ddata->window,ddata->gc,
		                0,0,ddata->x_width,ddata->x_height);

		/* Window manager stuff
		 */
		if (ddata->window_name == 0) ddata->window_name = default_name;
		if (ddata->icon_name   == 0) ddata->icon_name   = default_name;

		XStringListToTextProperty (&(ddata->window_name),1,&wText);
		XSetWMName (ddata->display,ddata->window,&wText);

		XStringListToTextProperty (&(ddata->icon_name)  ,1,&wText);
		XSetWMIconName (ddata->display,ddata->window,&wText);

		sizehints.flags = PSize;

		sizehints.width  = (int) ddata->x_width;
		sizehints.height = (int) ddata->x_height;

		XSetWMNormalHints (ddata->display,ddata->window,&sizehints);

		XMapWindow (ddata->display,ddata->window);
	}

	if (ddata->flags & WMF_X_PIXMAP_OPEN)
	{	ddata->flags &= ~WMF_X_PIXMAP_CLOSE;

		ddata->pixmap = XCreatePixmap (ddata->display,ddata->root,
		                               ddata->x_width,ddata->x_height,ddata->depth);

		if (ddata->pixmap == None)
		{	WMF_ERROR (API,"unable to create pixmap!");
			API->err = wmf_E_DeviceError;
			return;
		}

		ddata->flags |= WMF_X_PIXMAP_CLOSE;

		XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,
		                0,0,ddata->x_width,ddata->x_height);

		if (ddata->window != None)
		{	XSetWindowBackgroundPixmap (ddata->display,ddata->window,ddata->pixmap);
			XClearWindow (ddata->display,ddata->window);
		}
	}

	XFlush (ddata->display);
}

/* This is called by wmf_api_destroy()
 */
static void wmf_x_device_close (wmfAPI* API)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	WMF_DEBUG (API,"wmf_[x_]device_close");

	if (ddata->hatch != None) XFreePixmap (ddata->display,ddata->hatch);
	if (ddata->brush != None) XFreePixmap (ddata->display,ddata->brush);

	if (ddata->gc) XFreeGC (ddata->display,ddata->gc);

	if (ddata->flags & WMF_X_CMAP_DESTROY)  XFreeColormap  (ddata->display,ddata->colormap);
	if (ddata->flags & WMF_X_WINDOW_CLOSE)  XDestroyWindow (ddata->display,ddata->window);
	if (ddata->flags & WMF_X_PIXMAP_CLOSE)  XFreePixmap    (ddata->display,ddata->pixmap);
	if (ddata->flags & WMF_X_DISPLAY_CLOSE) XCloseDisplay  (ddata->display);
}

/* This is called from the beginning of each play for initial page setup
 */
static void wmf_x_device_begin (wmfAPI* API)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	WMF_DEBUG (API,"wmf_[x_]device_begin");

	setdefaultstyle (API);

	XSetClipMask (ddata->display,ddata->gc,None);

	XSetForeground (ddata->display,ddata->gc,ddata->white);

	if (ddata->window != None)
		XFillRectangle (ddata->display,ddata->window,ddata->gc,
		                0,0,ddata->x_width,ddata->x_height);
	if (ddata->pixmap != None)
		XFillRectangle (ddata->display,ddata->pixmap,ddata->gc,
		                0,0,ddata->x_width,ddata->x_height);

	XFlush (ddata->display);
}

/* This is called from the end of each play for page termination
 */
static void wmf_x_device_end (wmfAPI* API)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	WMF_DEBUG (API,"wmf_[x_]device_end");

	XFlush (ddata->display);
}

/* translation & scaling functions
 */
static XPoint x_translate (wmfAPI* API,wmfD_Coord d_pt)
{	return (x_translate_ft64 (API,d_pt,0));
}

static XPoint x_translate_ft64 (wmfAPI* API,wmfD_Coord d_pt,FT_Vector* pen)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XPoint x_pt;

	double x;
	double y;

	x = ((double) d_pt.x - (double) ddata->bbox.TL.x);
	x /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);
	x *= (double) ddata->x_width;

	y = ((double) d_pt.y - (double) ddata->bbox.TL.y);
	y /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);
	y *= (double) ddata->x_height;

	x_pt.x = (short) floor (x);
	x_pt.y = (short) floor (y);

	if (pen)
	{	pen->x = floor ((x - floor (x)) * 64);
		pen->y = floor ((y - floor (y)) * 64);
	}

	return (x_pt);
}

static float x_width (wmfAPI* API,float wmf_width)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	double width;

	width = (double) wmf_width * (double) ddata->x_width;
	width /= ((double) ddata->bbox.BR.x - (double) ddata->bbox.TL.x);

	return ((float) width);
}

static float x_height (wmfAPI* API,float wmf_height)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	double height;

	height = (double) wmf_height * (double) ddata->x_height;
	height /= ((double) ddata->bbox.BR.y - (double) ddata->bbox.TL.y);

	return ((float) height);
}
