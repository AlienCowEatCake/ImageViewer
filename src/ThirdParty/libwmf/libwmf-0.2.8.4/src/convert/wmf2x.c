/* libwmf (convert/wmf2gd.c): library for wmf conversion
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
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "wmfdefs.h"

#ifdef X_DISPLAY_MISSING

int main (int argc,char** argv)
{	fprintf (stderr,"%s: no support for X, sorry.\n",argv[0]);
	return (1);
}

#else /* X_DISPLAY_MISSING */ /* i.e., not */

#include "libwmf/x.h"

#include <X11/Xutil.h>
#include <X11/Xatom.h>

typedef struct
{	int    argc;
	char** argv;

	char* wmf_filename;

	wmf_x_t options;
} PlotData;

int  wmf2x_draw (PlotData*);

void wmf2x_init (PlotData*,int,char**);
void wmf2x_help (PlotData*);
int  wmf2x_args (PlotData*);

int  explicit_wmf_error (char*,wmf_error_t);

int wmf2x_draw (PlotData* pdata)
{	int status = 0;
	int redraw;
	int setmask;
	int length;

	float wmf_width  = 0;
	float wmf_height = 0;

	float ratio_wmf;
	float ratio_bounds;

	char key;

	unsigned int root_width  = 800; /* Unfortunately, need these before we open the display... */
	unsigned int root_height = 600;
	unsigned int disp_width  = 0;
	unsigned int disp_height = 0;

	unsigned long flags;

	wmf_error_t err;

	wmf_x_t* ddata = 0;

	wmfAPI* API = 0;

	wmfAPI_Options api_options;

	XEvent event;

	XWindowAttributes attributes;

	KeySym keysym;

	static char* icon_name = "wmf2x";

	flags = 0;

	flags |= WMF_OPT_FUNCTION;
	api_options.function = wmf_x_function;

	flags |= WMF_OPT_ARGS;
	api_options.argc = pdata->argc;
	api_options.argv = pdata->argv;
#ifndef DEBUG
	flags |= WMF_OPT_IGNORE_NONFATAL;
#endif
	err = wmf_api_create (&API,flags,&api_options);
	status = explicit_wmf_error ("wmf_api_create",err);

	if (status)
	{	if (API) wmf_api_destroy (API);
		return (status);
	}

	ddata = WMF_X_GetData (API);

	ddata->flags |= WMF_X_DISPLAY_OPEN;

	ddata->display_name = pdata->options.display_name;
	ddata->window_name = pdata->wmf_filename;
	ddata->icon_name = icon_name;

	length = strlen (pdata->wmf_filename);
#if defined (HAVE_EXPAT) || defined (HAVE_LIBXML2)
	if (length > 4)
	{
		if ((strcmp (pdata->wmf_filename + length - 4, ".xml") == 0) ||
		    (strcmp (pdata->wmf_filename + length - 4, ".XML") == 0))
		{	err = wmf_wmfxml_import (API,pdata->wmf_filename);
		}
		else err = wmf_file_open (API,pdata->wmf_filename);
	}
	else err = wmf_file_open (API,pdata->wmf_filename);
#else
	err = wmf_file_open (API,pdata->wmf_filename);
#endif
	status = explicit_wmf_error ("wmf_file_open",err);

	if (status)
	{	wmf_api_destroy (API);
		return (status);
	}

	err = wmf_scan (API,0,&(pdata->options.bbox));
	status = explicit_wmf_error ("wmf_scan",err);

	if (status)
	{	wmf_api_destroy (API);
		return (status);
	}

/* Okay, got this far, everything seems cool.
 */
	ddata->bbox = pdata->options.bbox;

	wmf_display_size (API,&disp_width,&disp_height,72,72);

	wmf_width  = (float) disp_width;
	wmf_height = (float) disp_height;

	if (pdata->options.x_width > 0)
	{	wmf_width  = pdata->options.x_width;
	}
	if (pdata->options.x_height > 0)
	{	wmf_height = pdata->options.x_height;
	}

	if ((wmf_width <= 0) || (wmf_height <= 0))
	{	fputs ("Bad image size - but this error shouldn't occur...\n",stderr);
		status = 1;
		wmf_api_destroy (API);
		return (status);
	}

	if ((wmf_width  > (float) root_width )
	 || (wmf_height > (float) root_height))
	{	ratio_wmf = wmf_height / wmf_width;
		ratio_bounds = (float) root_height / (float) root_width;

		if (ratio_wmf > ratio_bounds)
		{	ddata->x_height = root_height;
			ddata->x_width  = (unsigned int) ((float) ddata->x_height / ratio_wmf);
		}
		else
		{	ddata->x_width  = root_width;
			ddata->x_height = (unsigned int) ((float) ddata->x_width  * ratio_wmf);
		}
	}
	else
	{	ddata->x_width  = (unsigned int) ceil ((double) wmf_width );
		ddata->x_height = (unsigned int) ceil ((double) wmf_height);
	}

	redraw = 1;
	setmask = 1;
	while (status == 0)
	{	if (redraw)
		{	err = wmf_play (API,0,&(pdata->options.bbox));
			status = explicit_wmf_error ("wmf_play",err);
			if (status) break;
		}
		redraw = 0;

		if (setmask)
		{	XSelectInput (ddata->display,ddata->window,KeyPressMask|StructureNotifyMask);
			setmask = 0;
		}

		XNextEvent (ddata->display,&event);

		switch (event.type)
		{
		case KeyPress:
			XLookupString (&(event.xkey),&key,1,&keysym,0);

			switch (key)
			{
			case 'q':
			case 'Q':
				status = -1;
			break;

			case 'r':
			case 'R':
				redraw = 1;
			break;

			default:
			break;
			}
		break;

		case ConfigureNotify: /* This is all a bit clumsy, because I'm being lazy and letting    *
			               * the library ipa do most of the work that ought to be done here! */

			XGetWindowAttributes (ddata->display,ddata->window,&attributes);

			if ((ddata->x_width != attributes.width) || (ddata->x_height != attributes.height))
			{	ddata->x_width  = attributes.width;
				ddata->x_height = attributes.height;

				XFreePixmap (ddata->display,ddata->pixmap);

				ddata->flags &= ~WMF_X_PIXMAP_CLOSE;

				ddata->pixmap = XCreatePixmap (ddata->display,ddata->root,
				                               ddata->x_width,ddata->x_height,ddata->depth);

				if (ddata->pixmap == None)
				{	WMF_ERROR (API,"unable to create pixmap!");
					status = 1;
					break;
				}

				ddata->flags |= WMF_X_PIXMAP_CLOSE;

				XSetWindowBackgroundPixmap (ddata->display,ddata->window,ddata->pixmap);

				redraw = 1;
			}
		break;

		default:
		break;
		}
		if (status == (-1))
		{	status = 0;
			break;
		}
	}

	wmf_api_destroy (API);

	return (status);
}

void wmf2x_init (PlotData* pdata,int argc,char** argv)
{	pdata->argc = argc;
	pdata->argv = argv;

	pdata->wmf_filename = 0;

	pdata->options.display_name = 0;

	pdata->options.x_width  = 0;
	pdata->options.x_height = 0;

	pdata->options.flags = 0;
}

void wmf2x_help (PlotData* pdata)
{	fputs ("\
Usage: wmf2x [OPTION]... <file.wmf>\n\
Display metafile image.\n\
\n\
  -display <display>  where <display> is the display name.\n\
  --width=<w>         where <w> is the width of the image.\n\
  --height=<h>        where <h> is the height of the image.\n\
  --version           display version info and exit.\n\
  --help              display this help and exit.\n\
  --wmf-help          display wmf-related help and exit.\n\
\n\
Report bugs to <http://www.wvware.com/>.\n",stdout);
}

int wmf2x_args (PlotData* pdata)
{	int status = 0;
	int arg = 0;

	int    argc = pdata->argc;
	char** argv = pdata->argv;

	while ((++arg) < argc)
	{	if (strcmp (argv[arg],"--help") == 0)
		{	wmf2x_help (pdata);
			status = argc; /* i.e., not an error but don't continue */
			break;
		}

		if (strcmp (argv[arg],"--wmf-help") == 0)
		{	fputs (wmf_help (),stdout);
			status = argc; /* i.e., not an error but don't continue */
			break;
		}

		if (strcmp (argv[arg],"--version") == 0)
		{	fprintf (stdout,"%s: version %s\n",PACKAGE,VERSION);
			status = argc; /* i.e., not an error but don't continue */
			break;
		}

		if (strcmp (argv[arg],"-display") == 0)
		{	if ((++arg) < argc)
			{	pdata->options.display_name = argv[arg];
				continue;
			}
			fprintf (stderr,"usage: `wmf2x -display <displayname> <file.wmf>'.\n");
			fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
			status = arg;
			break;
		}

		if (strncmp (argv[arg],"--width=",8) == 0)
		{	if (sscanf (argv[arg]+8,"%u",&(pdata->options.x_width)) != 1)
			{	fputs ("usage: --width=<width>, where <width> is +ve integer.\n",stderr);
				status = arg;
				break;
			}
			continue;
		}
		if (strncmp (argv[arg],"--height=",9) == 0)
		{	if (sscanf (argv[arg]+9,"%u",&(pdata->options.x_height)) != 1)
			{	fputs ("usage: --height=<height>, where <height> is +ve integer.\n",stderr);
				status = arg;
				break;
			}
			continue;
		}

		if (strncmp (argv[arg],"--wmf-",6) == 0)
		{	continue;
		}

		if (argv[arg][0] != '-')
		{	pdata->wmf_filename = argv[arg];
			continue;
		}

		fprintf (stderr,"option `%s' not recognized.\n",argv[arg]);
		fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
		status = arg;
		break;
	}

	if (status == 0)
	{	if (pdata->wmf_filename == 0)
		{	fprintf (stderr,"No input file specified!\n");
			fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
			status = argc;
		}
	}

	return (status);
}

int main (int argc,char** argv)
{	int status = 0;

	PlotData PData;

	wmf2x_init (&PData,argc,argv);

	status = wmf2x_args (&PData);

	if (status) return (status);

	status = wmf2x_draw (&PData);

	return (status);
}

int explicit_wmf_error (char* str,wmf_error_t err)
{	int status = 0;

	switch (err)
	{
	case wmf_E_None:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_None.\n",str);
#endif
		status = 0;
	break;

	case wmf_E_InsMem:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_InsMem.\n",str);
#endif
		status = 1;
	break;

	case wmf_E_BadFile:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_BadFile.\n",str);
#endif
		status = 1;
	break;

	case wmf_E_BadFormat:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_BadFormat.\n",str);
#endif
		status = 1;
	break;

	case wmf_E_EOF:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_EOF.\n",str);
#endif
		status = 1;
	break;

	case wmf_E_DeviceError:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_DeviceError.\n",str);
#endif
		status = 1;
	break;

	case wmf_E_Glitch:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_Glitch.\n",str);
#endif
		status = 1;
	break;

	case wmf_E_Assert:
#ifdef DEBUG
		fprintf (stderr,"%s returned with wmf_E_Assert.\n",str);
#endif
		status = 1;
	break;

	default:
#ifdef DEBUG
		fprintf (stderr,"%s returned unexpected value.\n",str);
#endif
		status = 1;
	break;
	}

	return (status);
}

#endif /* X_DISPLAY_MISSING */
