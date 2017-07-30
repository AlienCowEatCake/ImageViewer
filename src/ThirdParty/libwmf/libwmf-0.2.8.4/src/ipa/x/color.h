/* libwmf ("ipa/x/color.h"): library for wmf conversion
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


static void setup_color (wmfAPI* API)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	XColor rgb;

	int r;
	int g;
	int b;

	unsigned long c;

	ddata->flags &= ~WMF_X_CMAP_DESTROY;

/* Okay, trouble with X is all those visual classes, of which (thankfully)
 * PseudoColor and TrueColor are the most common;
 * Color allocation is always going to be tricky; here I set up a 6-bit
 * pseudo-TrueColor mapping for PseudoColor...
 */
	if (ddata->class == PseudoColor)
	{	ddata->colormap = DefaultColormap (ddata->display,DefaultScreen (ddata->display));

		if (ddata->visual->map_entries >= 64)
		{	ddata->color = wmf_malloc (API,64 * sizeof (unsigned long));

			if (ERR (API))
			{	WMF_DEBUG (API,"bailing...");
				return;
			}

			if (XAllocColorCells (ddata->display,ddata->colormap,False,0,0,ddata->color,64) == 0)
			{	ddata->colormap = XCopyColormapAndFree (ddata->display,ddata->colormap);
				ddata->flags |= WMF_X_CMAP_DESTROY;

				if (XAllocColorCells (ddata->display,ddata->colormap,False,0,0,ddata->color,64) == 0)
				{	WMF_ERROR (API,"setup_color: something bizarre going on here.");

					wmf_free (API,ddata->color);
					ddata->color = 0;

					XFreeColormap (ddata->display,ddata->colormap);

					ddata->colormap = DefaultColormap (ddata->display,DefaultScreen (ddata->display));
					ddata->flags &= ~WMF_X_CMAP_DESTROY;
				}
			}
		}

		if (ddata->color)
		{	rgb.flags = DoRed | DoGreen | DoBlue;
			c = 0;
			for (b = 65535; b >= 0; b -= 21845)
				for (g = 65535; g >= 0; g -= 21845)
					for (r = 65535; r >= 0; r -= 21845)
					{	rgb.pixel = ddata->color[c];
						rgb.red   = r;
						rgb.green = g;
						rgb.blue  = b;
						XStoreColor (ddata->display,ddata->colormap,&rgb);
						c++;
					}

			ddata->black = ddata->color[0];
			ddata->white = ddata->color[63];

			ddata->mask.red   = 48;
			ddata->mask.green = 12;
			ddata->mask.blue  =  3;
		}
		else
		{	WMF_DEBUG (API,"setup_color: too few colors; going B&W...");

			ddata->black = BlackPixel (ddata->display,DefaultScreen (ddata->display));
			ddata->white = WhitePixel (ddata->display,DefaultScreen (ddata->display));

			ddata->mask.red   = 0;
			ddata->mask.green = 0;
			ddata->mask.blue  = 0;
		}
	}
	else if (ddata->class == TrueColor)
	{	ddata->black = 0;
		ddata->white = ddata->visual->red_mask | ddata->visual->green_mask | ddata->visual->blue_mask;

		ddata->mask.red   = ddata->visual->red_mask;
		ddata->mask.green = ddata->visual->green_mask;
		ddata->mask.blue  = ddata->visual->blue_mask;
	}
	else
	{	WMF_ERROR (API,"setup_color: this class of visual is not supported!");
		WMF_ERROR (API,"             please contact us at http://www.wvware.com/");
		API->err = wmf_E_Glitch;
		return;
	}
}

static unsigned long get_color (wmfAPI* API,wmfRGB* rgb)
{	wmf_x_t* ddata = WMF_X_GetData (API);

	unsigned long pixel;
	unsigned long mask;

	unsigned long red;
	unsigned long green;
	unsigned long blue;

	int shift;
	int bits;

	if ((ddata->mask.red == 0) || (ddata->mask.green == 0) || (ddata->mask.blue == 0))
	{	if ((rgb->r > 0x7f) || (rgb->g > 0x7f) || (rgb->b > 0x7f))
		{	pixel = ddata->white;
		}
		else
		{	pixel = ddata->black;
		}

		return (pixel);
	}

/* lots of assumptions built into this next bit, so if your colors go
 * haywire then gosh.
 */
	mask = ddata->mask.red;
	shift = 0;
	while ((mask & 1) == 0)
	{	mask >>= 1;
		shift++;
	}
	bits = 0;
	while ((mask & 1) == 1)
	{	mask >>= 1;
		bits++;
	}
	red = (long) rgb->r;
	while (bits > 8)
	{	red <<= 1;
		bits--;
	}
	while (bits < 8)
	{	red >>= 1;
		bits++;
	}
	red <<= shift;

	mask = ddata->mask.green;
	shift = 0;
	while ((mask & 1) == 0)
	{	mask >>= 1;
		shift++;
	}
	bits = 0;
	while ((mask & 1) == 1)
	{	mask >>= 1;
		bits++;
	}
	green = (long) rgb->g;
	while (bits > 8)
	{	green <<= 1;
		bits--;
	}
	while (bits < 8)
	{	green >>= 1;
		bits++;
	}
	green <<= shift;

	mask = ddata->mask.blue;
	shift = 0;
	while ((mask & 1) == 0)
	{	mask >>= 1;
		shift++;
	}
	bits = 0;
	while ((mask & 1) == 1)
	{	mask >>= 1;
		bits++;
	}
	blue = (long) rgb->b;
	while (bits > 8)
	{	blue <<= 1;
		bits--;
	}
	while (bits < 8)
	{	blue >>= 1;
		bits++;
	}
	blue <<= shift;

	pixel = red | green | blue;

	if (ddata->color) pixel = ddata->color[pixel]; /* map color */

	return (pixel);
}
