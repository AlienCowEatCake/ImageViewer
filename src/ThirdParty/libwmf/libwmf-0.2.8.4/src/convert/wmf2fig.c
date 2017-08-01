/* libwmf (convert/wmf2fig.c): library for wmf conversion
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

#include "libwmf/fig.h"

#define A4_Width  596
#define A4_Height 842

#define Default_Margin 60

typedef struct
{	int    argc;
	char** argv;

	char** auto_files;
	char*  wmf_filename;
	char*  fig_filename;

	FILE* out;

	wmf_fig_t options;

	unsigned long image_format;
} PlotData;

typedef struct _ImageContext ImageContext;

struct _ImageContext
{	int number;
	char* prefix;
	char* suffix;
};

char* image_name (void*);

int  wmf2fig_draw (PlotData*);

void wmf2fig_init (PlotData*,int,char**);
void wmf2fig_help (PlotData*);
int  wmf2fig_args (PlotData*);
int  wmf2fig_auto (PlotData*);
int  wmf2fig_file (PlotData*);

int  explicit_wmf_error (char*,wmf_error_t);

int wmf2fig_draw (PlotData* pdata)
{	int status = 0;

#if 0
	unsigned int wmf_width;
	unsigned int wmf_height;

	unsigned int page_width;
	unsigned int page_height;
#endif

	unsigned long flags;
#if 0
	int page_margin;

	float def_width;
	float def_height;

	float ratio_wmf;
	float ratio_page;
#endif
	static char* Default_Creator = "wmf2fig";

	ImageContext IC;

	wmf_error_t err;

	wmf_fig_t* ddata = 0;

	wmfAPI* API = 0;

	wmfAPI_Options api_options;

	flags = 0;

	flags |= WMF_OPT_FUNCTION;
	api_options.function = wmf_fig_function;

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
	ddata = WMF_FIG_GetData (API);

	if (((ddata->flags & WMF_FIG_SUPPORTS_PNG) == 0)
	  && (pdata->image_format == WMF_FIG_IMAGE_PNG))
	{	fprintf (stderr,"Sorry! PNG image format is not supported!\n");
		wmf_api_destroy (API);
		status = 1;
		return (status);
	}
	if (((ddata->flags & WMF_FIG_SUPPORTS_JPEG) == 0)
	  && (pdata->image_format == WMF_FIG_IMAGE_JPEG))
	{	fprintf (stderr,"Sorry! JPEG image format is not supported!\n");
		wmf_api_destroy (API);
		status = 1;
		return (status);
	}
	ddata->flags |= pdata->image_format;

	ddata->out = wmf_stream_create (API,pdata->out);

	if (pdata->options.Title) ddata->Title = pdata->options.Title;
	else                      ddata->Title = pdata->wmf_filename;

	if (pdata->options.Creator) ddata->Creator = pdata->options.Creator;
	else                        ddata->Creator = Default_Creator;

	if (pdata->options.Date) ddata->Date = pdata->options.Date;

	if (pdata->options.For) ddata->For = pdata->options.For;

	ddata->dpi = pdata->options.dpi;
	
	ddata->format = pdata->options.format;

	ddata->ddec = pdata->options.ddec;

	ddata->bbox = pdata->options.bbox;

	ddata->fig_width  = pdata->options.fig_width; 
	ddata->fig_height = pdata->options.fig_height;

	ddata->flags |= pdata->options.flags;

	IC.number = 0;
	IC.prefix = (char*) malloc (strlen (pdata->wmf_filename) + 1);
	if (IC.prefix)
	{	strcpy (IC.prefix,pdata->wmf_filename);
		if (wmf_strstr (pdata->wmf_filename,".wmf")) IC.prefix[strlen (pdata->wmf_filename)-4] = 0;
		ddata->image.context = (void*) (&IC);
		ddata->image.name = image_name;
	}
	if (pdata->image_format == WMF_FIG_IMAGE_PNG)
	{	IC.suffix = "png";
	}
	else if (pdata->image_format == WMF_FIG_IMAGE_JPEG)
	{	IC.suffix = "jpg";
	}
	else
	{	IC.suffix = "eps";
	}

	if (status == 0)
	{	err = wmf_play (API,0,&(pdata->options.bbox));
		status = explicit_wmf_error ("wmf_play",err);
	}

	wmf_api_destroy (API);

	return (status);
}

void wmf2fig_init (PlotData* pdata,int argc,char** argv)
{	pdata->argc = argc;
	pdata->argv = argv;

	pdata->auto_files = 0;
	pdata->wmf_filename = 0;
	pdata->fig_filename = 0;

	pdata->out = 0;

	pdata->options.Title = 0;
	pdata->options.Creator = 0;
	pdata->options.Date = 0;
	pdata->options.For = 0;

	pdata->options.fig_x = 0;
	pdata->options.fig_y = 0;
	pdata->options.fig_width  = 0;
	pdata->options.fig_height = 0;

	pdata->options.format = wmf_P_A4;

	pdata->options.dpi = 1200;

	pdata->options.flags = 0;

	pdata->options.ddec = 1;

	pdata->image_format = 0;
}

void wmf2fig_help (PlotData* pdata)
{	fputs ("\
Usage: wmf2fig [OPTION]... [-o <file.eps>] <file.wmf>\n\
  or:  wmf2fig [OPTION]... --auto <file1.wmf> [<file2.wmf> ...]\n\
Convert metafile image to XFig format.\n\
\n\
  --figunit=<int> the FIG unit as fraction of inch, default: 1200\n\
  --page=<page>   where <page> is one of (default A4):\n\
                  A[01234] B5 Letter Legal Ledger Tabloid.\n\
  --landscape     switch to landscape view.\n\
  --portrait      switch to portrait view (default).\n\
  --maxpect       scale image: fit to page (1 inch margins)\n\
  --no-margins    ignore margins when scaling\n\
  --flat          render at one depth value\n\
  --image=<fmt>   where <fmt> is one of eps,png,jpg (default eps):\n\
  --title=<str>   comment # Title\n\
  --creator=<str> comment # Creator\n\
  --date=<str>    comment # Date\n\
  --for=<str>     comment # For\n\
  --version       display version info and exit.\n\
  --help          display this help and exit.\n\
  --wmf-help      display wmf-related help and exit.\n\
\n\
Report bugs to wvware-devel@lists.sourceforge.net.\n",stdout);
}

int wmf2fig_args (PlotData* pdata)
{	int status = 0;
	int arg = 0;

	int    argc = pdata->argc;
	char** argv = pdata->argv;

	char* page = 0;
	char* ifmt = 0;

	while ((++arg) < argc)
	{	if (strcmp (argv[arg],"--help") == 0)
		{	wmf2fig_help (pdata);
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

		if (strncmp (argv[arg],"--figunit=",10) == 0)
		{	if (sscanf (argv[arg] + 10,"%u",&(pdata->options.dpi)) != 1)
			{	fprintf (stderr,"can't interpret figunit `%s'...\n",argv[arg] + 10);
				status = arg;
				break;
			}
			else if (pdata->options.dpi == 0)
			{	fprintf (stderr,"figunit (dpi) cannot be 0!\n");
				status = arg;
				break;
			}
			continue;
		}

		if (strncmp (argv[arg],"--page=",7) == 0)
		{	page = argv[arg] + 7;
			     if (strcmp (page,"A4")      == 0) pdata->options.format = wmf_P_A4;
			else if (strcmp (page,"A3")      == 0) pdata->options.format = wmf_P_A3;
			else if (strcmp (page,"A2")      == 0) pdata->options.format = wmf_P_A2;
			else if (strcmp (page,"A1")      == 0) pdata->options.format = wmf_P_A1;
			else if (strcmp (page,"A0")      == 0) pdata->options.format = wmf_P_A0;
			else if (strcmp (page,"B5")      == 0) pdata->options.format = wmf_P_B5;
			else if (strcmp (page,"Letter")  == 0) pdata->options.format = wmf_P_Letter;
			else if (strcmp (page,"Legal")   == 0) pdata->options.format = wmf_P_Legal;
			else if (strcmp (page,"Ledger")  == 0) pdata->options.format = wmf_P_Ledger;
			else if (strcmp (page,"Tabloid") == 0) pdata->options.format = wmf_P_Tabloid;
			else
			{	fprintf (stderr,"wmf2fig: page `%s' not recognized.\n",page);
				status = arg;
				break;
			}
			continue;
		}

		if (strcmp (argv[arg],"--landscape") == 0)
		{	pdata->options.flags |= WMF_FIG_LANDSCAPE;
			continue;
		}
		if (strcmp (argv[arg],"--portrait") == 0)
		{	pdata->options.flags &= ~WMF_FIG_LANDSCAPE;
			continue;
		}

		if (strcmp (argv[arg],"--maxpect") == 0)
		{	pdata->options.flags |= WMF_FIG_MAXPECT;
			continue;
		}

		if (strcmp (argv[arg],"--no-margins") == 0)
		{	pdata->options.flags |= WMF_FIG_NO_MARGINS;
			continue;
		}

		if (strcmp (argv[arg],"--flat") == 0)
		{	pdata->options.ddec = 0;
			continue;
		}

		if (strcmp (argv[arg],"--image=eps") == 0)
		{	pdata->image_format = 0;
			continue;
		}
		if (strcmp (argv[arg],"--image=png") == 0)
		{	pdata->image_format = WMF_FIG_IMAGE_PNG;
			continue;
		}
		if ((strcmp (argv[arg],"--image=jpg" ) == 0)
		 || (strcmp (argv[arg],"--image=jpeg") == 0))
		{	pdata->image_format = WMF_FIG_IMAGE_JPEG;
			continue;
		}
		if (strncmp (argv[arg],"--image=",8) == 0)
		{	ifmt = argv[arg] + 8;
			fprintf (stderr,"wmf2fig: image format `%s' not recognized.\n",ifmt);
			status = arg;
			break;
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
			{	pdata->fig_filename = argv[arg];
				continue;
			}
			fprintf (stderr,"usage: `wmf2fig -o <file.fig> <file.wmf>'.\n");
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

int wmf2fig_auto (PlotData* pdata)
{	int status = 0;

	while (1)
	{	pdata->wmf_filename = (*(pdata->auto_files));

		if (pdata->wmf_filename == 0) break;

		if (strcmp (pdata->wmf_filename+strlen(pdata->wmf_filename)-4,".wmf"))
		{	fprintf (stderr,"%s: expected suffix `.wmf'. ",pdata->wmf_filename);
			fprintf (stderr,"skipping...\n");
			status++;
		}
		else if ((pdata->fig_filename = malloc (strlen(pdata->wmf_filename)+1)) == 0)
		{	fprintf (stderr,"mem_alloc_err: skipping %s...\n",pdata->wmf_filename);
			status++;
		}
		else
		{	strcpy (pdata->fig_filename,pdata->wmf_filename);
			strcpy (pdata->fig_filename+strlen(pdata->fig_filename)-3,"fig");
			if (wmf2fig_file (pdata)) status++;
			free (pdata->fig_filename);
		}
		
		pdata->auto_files++;
	}

	return (status);
}

int wmf2fig_file (PlotData* pdata)
{	int status = 0;

	pdata->out = stdout;
	if (pdata->fig_filename)
	{	if ((pdata->out = fopen (pdata->fig_filename,"w")) == 0)
		{	fprintf (stderr,"unable to write to `%s'. ",pdata->fig_filename);
			fprintf (stderr,"skipping...\n");
			status = 1;
			return (status);
		}
	}

	status = wmf2fig_draw (pdata);

	if (pdata->out != stdout) fclose (pdata->out);

	return (status);
}

int main (int argc,char** argv)
{	int status = 0;

	PlotData PData;

	wmf2fig_init (&PData,argc,argv);

	status = wmf2fig_args (&PData);

	if (status) return (status);

	if (PData.auto_files) status = wmf2fig_auto (&PData);
	else                  status = wmf2fig_file (&PData);

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

char* image_name (void* context)
{	ImageContext* IC = (ImageContext*) context;

	int length;

	char* name = 0;

	length = strlen (IC->prefix) + 16;

	name = malloc (length);

	if (name == 0) return (0);

	IC->number++;

	sprintf (name,"%s-%d.%s",IC->prefix,IC->number,IC->suffix);

	return (name);
}
