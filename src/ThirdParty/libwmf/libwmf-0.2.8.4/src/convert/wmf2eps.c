/* libwmf (convert/wmf2eps.c): library for wmf conversion
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

#include "libwmf/eps.h"

#define A4_Width  596
#define A4_Height 842

#define Default_Margin 60

typedef struct
{	int    argc;
	char** argv;

	char** auto_files;
	char*  wmf_filename;
	char*  eps_filename;

	FILE* out;

	wmf_eps_t options;

	int centre;
	int maxpect;
} PlotData;

int  wmf2eps_draw (PlotData*);

void wmf2eps_init (PlotData*,int,char**);
void wmf2eps_help (PlotData*);
int  wmf2eps_args (PlotData*);
int  wmf2eps_auto (PlotData*);
int  wmf2eps_file (PlotData*);

int  bbox_translate (PlotData*,char*);

int  explicit_wmf_error (char*,wmf_error_t);

int wmf2eps_draw (PlotData* pdata)
{	int status = 0;

	unsigned int wmf_width;
	unsigned int wmf_height;

	unsigned int page_width;
	unsigned int page_height;

	unsigned long flags;

	int page_margin;

	float def_width  = 0;
	float def_height = 0;

	float ratio_wmf;
	float ratio_page;

	static char* Default_Creator = "wmf2eps";

	wmf_error_t err;

	wmf_eps_t* ddata = 0;

	wmfAPI* API = 0;

	wmfAPI_Options api_options;

	flags = 0;

	flags |= WMF_OPT_FUNCTION;
	api_options.function = wmf_eps_function;

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

	err = wmf_file_open (API,pdata->wmf_filename);
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
	ddata = WMF_EPS_GetData (API);

	ddata->out = wmf_stream_create (API,pdata->out);

	if (pdata->options.Title) ddata->Title = pdata->options.Title;
	else                      ddata->Title = pdata->wmf_filename;

	if (pdata->options.Creator) ddata->Creator = pdata->options.Creator;
	else                        ddata->Creator = Default_Creator;

	if (pdata->options.Date) ddata->Date = pdata->options.Date;

	if (pdata->options.For) ddata->For = pdata->options.For;

	ddata->bbox = pdata->options.bbox;

	ddata->eps_x = pdata->options.eps_x;           /* eps_x, eps_y, eps_width */
	ddata->eps_y = pdata->options.eps_y;           /* & eps_height may be     */

	ddata->eps_width  = pdata->options.eps_width;  /* changed if --centre or  */
	ddata->eps_height = pdata->options.eps_height; /* --maxpect are set.      */

	ddata->page_width  = pdata->options.page_width;
	ddata->page_height = pdata->options.page_height;

	ddata->flags = pdata->options.flags;

	if (ddata->flags & WMF_EPS_STYLE_PS)
	{	if (ddata->flags & WMF_EPS_LANDSCAPE)
		{	page_width  = ddata->page_height;
			page_height = ddata->page_width;
		}
		else
		{	page_width  = ddata->page_width;
			page_height = ddata->page_height;
		}
		page_margin = Default_Margin;

		if ((page_width < 2 * page_margin) || (page_height < 2 * page_margin))
		{	page_margin = 0;
			fputs ("wmf2eps: warning: small page: no margins.\n",stderr);
		}
		page_width  -= 2 * page_margin;
		page_height -= 2 * page_margin;

		wmf_size (API,&def_width,&def_height);

		wmf_width  = (unsigned int) ceil (def_width );
		wmf_height = (unsigned int) ceil (def_height);

		if ((wmf_width == 0) || (wmf_height == 0))
		{	fprintf (stderr,"image `%s' has no size!\n",pdata->wmf_filename);
			wmf_api_destroy (API);
			return (1);
		}

		/* adjust figure placement as necessary here */
		if (pdata->maxpect)
		{	ratio_wmf  = (float)  wmf_width / (float)  wmf_height;
			ratio_page = (float) page_width / (float) page_height;

			if (ratio_wmf > ratio_page)
			{	ddata->eps_width  = page_width;
				ddata->eps_height = (unsigned int) ((float) page_width  / (float) ratio_wmf);
			}
			else
			{	ddata->eps_height = page_height;
				ddata->eps_width  = (unsigned int) ((float) page_height * (float) ratio_wmf);
			}

			pdata->centre = 1;
		}
		if (pdata->centre)
		{	ddata->eps_x = page_margin + (page_width  - ddata->eps_width ) / 2;
			ddata->eps_y = page_margin + (page_height - ddata->eps_height) / 2;

			if ((ddata->eps_x < 0) || (ddata->eps_y < 0))
			{	fputs ("wmf2eps: warning: figure exceeds page.\n",stderr);
			}
		}
	}

	if (status == 0)
	{	err = wmf_play (API,0,&(pdata->options.bbox));
		status = explicit_wmf_error ("wmf_play",err);
	}

	wmf_api_destroy (API);

	return (status);
}

void wmf2eps_init (PlotData* pdata,int argc,char** argv)
{	pdata->argc = argc;
	pdata->argv = argv;

	pdata->auto_files = 0;
	pdata->wmf_filename = 0;
	pdata->eps_filename = 0;

	pdata->out = 0;

	pdata->options.Title = 0;
	pdata->options.Creator = 0;
	pdata->options.Date = 0;
	pdata->options.For = 0;

	pdata->options.eps_x = Default_Margin;
	pdata->options.eps_y = Default_Margin;
	pdata->options.eps_width  = 0;
	pdata->options.eps_height = 0;

	pdata->options.page_width  = A4_Width;
	pdata->options.page_height = A4_Height;

	pdata->options.flags = 0;

	pdata->centre = 0;
	pdata->maxpect = 0;
}

void wmf2eps_help (PlotData* pdata)
{	fputs ("\
Usage: wmf2eps [OPTION]... [-o <file.eps>] <file.wmf>\n\
  or:  wmf2eps [OPTION]... --auto <file1.wmf> [<file2.wmf> ...]\n\
Convert metafile image to postscript.\n\
\n\
  --eps           output as eps (default).\n\
  --ps            output as ps.\n\
  --page=<page>   where <page> is one of A4 (default).        [ps-mode only]\n\
  --landscape     switch to landscape view.                   [ps-mode only]\n\
  --portrait      switch to portrait view (default).          [ps-mode only]\n\
  --bbox=<geom>   geometry setting: --bbox=WxH+X+Y            [ps-mode only]\n\
  --centre        centre image in page.                       [ps-mode only]\n\
  --maxpect       scale image to maximum size keeping aspect. [ps-mode only]\n\
  --title=<str>   postscript %%Title\n\
  --creator=<str> postscript %%Creator\n\
  --date=<str>    postscript %%Date\n\
  --for=<str>     postscript %%For\n\
  --version       display version info and exit.\n\
  --help          display this help and exit.\n\
  --wmf-help      display wmf-related help and exit.\n\
\n\
Report bugs to <http://www.wvware.com/>.\n",stdout);
}

int wmf2eps_args (PlotData* pdata)
{	int status = 0;
	int arg = 0;

	int    argc = pdata->argc;
	char** argv = pdata->argv;

	char* page = 0;

	while ((++arg) < argc)
	{	if (strcmp (argv[arg],"--help") == 0)
		{	wmf2eps_help (pdata);
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

		if (strcmp (argv[arg],"--eps") == 0)
		{	pdata->options.flags &= ~WMF_EPS_STYLE_PS;
			continue;
		}
		if (strcmp (argv[arg],"--ps") == 0)
		{	pdata->options.flags |= WMF_EPS_STYLE_PS;
			continue;
		}

		if (strncmp (argv[arg],"--page=",7) == 0)
		{	page = argv[arg] + 7;
			if (strcmp (page,"A4") == 0)
			{	pdata->options.page_width  = A4_Width;
				pdata->options.page_height = A4_Height;
			}
			else
			{	fprintf (stderr,"wmf2eps: page `%s' not recognized.\n",page);
				status = arg;
				break;
			}
			continue;
		}

		if (strcmp (argv[arg],"--landscape") == 0)
		{	pdata->options.flags |= WMF_EPS_LANDSCAPE;
			continue;
		}
		if (strcmp (argv[arg],"--portrait") == 0)
		{	pdata->options.flags &= ~WMF_EPS_LANDSCAPE;
			continue;
		}

		if (strncmp (argv[arg],"--bbox=",7) == 0)
		{	if (bbox_translate (pdata,argv[arg] + 7))
			{	status = arg;
				break;
			}
			continue;
		}

		if ((strcmp (argv[arg],"--centre") == 0)
		 || (strcmp (argv[arg],"--center") == 0))
		{	pdata->centre = 1;
			continue;
		}
		if (strcmp (argv[arg],"--maxpect") == 0)
		{	pdata->maxpect = 1;
			continue;
		}

		if (strncmp (argv[arg],"--title=",8) == 0)
		{	pdata->options.Title = argv[arg] + 8;
			continue;
		}
		if (strncmp (argv[arg],"--creator=",10) == 0)
		{	pdata->options.Creator = argv[arg] + 10;
			continue;
		}
		if (strncmp (argv[arg],"--date=",7) == 0)
		{	pdata->options.Date = argv[arg] + 7;
			continue;
		}
		if (strncmp (argv[arg],"--for=",6) == 0)
		{	pdata->options.For = argv[arg] + 6;
			continue;
		}

		if (strcmp (argv[arg],"--auto") == 0)
		{	pdata->auto_files = argv + arg + 1;
			break;
		}
		if (strcmp (argv[arg],"-o") == 0)
		{	if ((++arg) < argc)
			{	pdata->eps_filename = argv[arg];
				continue;
			}
			fprintf (stderr,"usage: `wmf2eps -o <file.eps> <file.wmf>'.\n");
			fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
			status = arg;
			break;
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
	{	if ((pdata->auto_files == 0) && (pdata->wmf_filename == 0))
		{	fprintf (stderr,"No input file specified!\n");
			fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
			status = argc;
		}
	}

	return (status);
}

int wmf2eps_auto (PlotData* pdata)
{	int status = 0;

	while (1)
	{	pdata->wmf_filename = (*(pdata->auto_files));

		if (pdata->wmf_filename == 0) break;

		if (strcmp (pdata->wmf_filename+strlen(pdata->wmf_filename)-4,".wmf"))
		{	fprintf (stderr,"%s: expected suffix `.wmf'. ",pdata->wmf_filename);
			fprintf (stderr,"skipping...\n");
			status++;
		}
		else if ((pdata->eps_filename = malloc (strlen(pdata->wmf_filename)+1)) == 0)
		{	fprintf (stderr,"mem_alloc_err: skipping %s...\n",pdata->wmf_filename);
			status++;
		}
		else
		{	strcpy (pdata->eps_filename,pdata->wmf_filename);
			strcpy (pdata->eps_filename+strlen(pdata->eps_filename)-3,"eps");
			if (wmf2eps_file (pdata)) status++;
			free (pdata->eps_filename);
		}
		
		pdata->auto_files++;
	}

	return (status);
}

int wmf2eps_file (PlotData* pdata)
{	int status = 0;

	pdata->out = stdout;
	if (pdata->eps_filename)
	{	if ((pdata->out = fopen (pdata->eps_filename,"w")) == 0)
		{	fprintf (stderr,"unable to write to `%s'. ",pdata->eps_filename);
			fprintf (stderr,"skipping...\n");
			status = 1;
			return (status);
		}
	}

	status = wmf2eps_draw (pdata);

	if (pdata->out != stdout) fclose (pdata->out);

	return (status);
}

int main (int argc,char** argv)
{	int status = 0;

	PlotData PData;

	wmf2eps_init (&PData,argc,argv);

	status = wmf2eps_args (&PData);

	if (status) return (status);

	if (PData.auto_files) status = wmf2eps_auto (&PData);
	else                  status = wmf2eps_file (&PData);

	return (status);
}

int bbox_translate (PlotData* pdata,char* bbox)
{	int status = 0;
	int sign_x = 1;
	int sign_y = 1;
	int tmp;

	char* geom = 0;
	char* geom_x = 0;
	char* geom_y = 0;
	char* geom_width  = 0;
	char* geom_height = 0;
	char* ptr;

	geom = (char*) malloc (strlen (bbox) + 1);
	if (geom == 0)
	{	fputs ("wmf2eps: erk! insufficient memory!\n",stderr);
		return (1);
	}
	strcpy (geom,bbox);

	ptr = geom;
	geom_width = ptr;
	while ((*ptr) && (status == 0))
	{	if (!isdigit ((int)(*ptr)))
		{	if ((*ptr) == 'x')
			{	if (geom_height) status = 2;
				else geom_height = ptr + 1;
				(*ptr) = 0;
			}
			else if (((*ptr) == '+') || ((*ptr) == '-'))
			{	if (geom_height == 0) status = 2;
				else if (geom_x && geom_y) status = 2;
				else if (geom_x == 0)
				{	geom_x = ptr + 1;
					if ((*ptr) == '-') sign_x = -1;
					(*ptr) = 0;
				}
				else
				{	geom_y = ptr + 1;
					if ((*ptr) == '-') sign_y = -1;
					(*ptr) = 0;
				}
			}
			else status = 2;
		}
		ptr++;
	}
	if (status || (geom_height == 0) || (geom_x && (geom_y == 0)))
	{	fputs ("usage: wmf2eps --bbox=WxH+X+Y\n",stderr);
		return (2);
	}

	pdata->options.eps_x = 0;
	pdata->options.eps_y = 0;

	pdata->options.eps_width  = 0;
	pdata->options.eps_height = 0;

	if (sscanf (geom_width,"%d",&tmp) != 1) status++;
	else pdata->options.eps_width = tmp;

	if (sscanf (geom_height,"%d",&tmp) != 1) status++;
	else pdata->options.eps_height = tmp;

	if (geom_x && geom_y)
	{	if (sscanf (geom_x,"%d",&tmp) != 1) status++;
		else pdata->options.eps_x = tmp * sign_x;

		if (sscanf (geom_y,"%d",&tmp) != 1) status++;
		else pdata->options.eps_y = tmp * sign_y;
	}

	if (status)
	{	fputs ("usage: wmf2eps --bbox=WxH+X+Y\n",stderr);
		return (2 + status);
	}

	free (geom);

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
