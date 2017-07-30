/* wmf2plot (convert/wmf2plot.c): utility for wmf conversion
   Copyright (C) 2001 The Free Software Foundation

   wmf2plot is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   wmf2plot is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public
   License along with the libwmf Library; see the file COPYING.GPL.  If not,
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

#ifndef HAVE_LIBPLOT

int main (int argc,char** argv)
{	fprintf (stderr,"%s: no support for GNU plotutils, sorry.\n",argv[0]);
	return (1);
}

#else /* ! HAVE_LIBPLOT */

#include "libwmf/plot.h"

typedef struct
{	int    argc;
	char** argv;

	char** auto_files;
	char*  wmf_filename;
	char*  plot_filename;

	wmf_plot_t options;

	unsigned int max_width;
	unsigned int max_height;

	unsigned long max_flags;
} PlotData;

#define WMF2PLOT_MAXPECT (1 << 0)
#define WMF2PLOT_MAXSIZE (1 << 1)

int  wmf2plot_draw (PlotData*);

void wmf2plot_init (PlotData*,int,char**);
void wmf2plot_help (PlotData*);
int  wmf2plot_args (PlotData*);
int  wmf2plot_auto (PlotData*);
int  wmf2plot_file (PlotData*);

int  explicit_wmf_error (char*,wmf_error_t);

int wmf2plot_draw (PlotData* pdata)
{	int status = 0;

	float wmf_width;
	float wmf_height;

	float ratio_wmf;
	float ratio_bounds;

	unsigned long flags;
	unsigned long max_flags;

	wmf_error_t err;

	wmf_plot_t* ddata = 0;

	wmfAPI* API = 0;

	wmfAPI_Options api_options;

	flags = 0;

	flags |= WMF_OPT_FUNCTION;
	api_options.function = wmf_plot_function;

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

	ddata = WMF_PLOT_GetData (API);

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
	ddata->type = pdata->options.type;

	ddata->file = pdata->options.file;

	ddata->bbox = pdata->options.bbox;

	wmf_size (API,&wmf_width,&wmf_height);

	if ((wmf_width <= 0) || (wmf_height <= 0))
	{	fputs ("Bad image size - but this error shouldn't occur...\n",stderr);
		status = 1;
		wmf_api_destroy (API);
		return (status);
	}

	max_flags = pdata->max_flags;

	if ((wmf_width  > (float) pdata->max_width )
	 || (wmf_height > (float) pdata->max_height))
	{	if (max_flags == 0) max_flags = WMF2PLOT_MAXPECT;
	}

	if (max_flags == WMF2PLOT_MAXPECT) /* scale the image */
	{	ratio_wmf = wmf_height / wmf_width;
		ratio_bounds = (float) pdata->max_height / (float) pdata->max_width;

		if (ratio_wmf > ratio_bounds)
		{	ddata->height = pdata->max_height;
			ddata->width  = (unsigned int) ((float) ddata->height / ratio_wmf);
		}
		else
		{	ddata->width  = pdata->max_width;
			ddata->height = (unsigned int) ((float) ddata->width  * ratio_wmf);
		}
	}
	else if (max_flags == WMF2PLOT_MAXSIZE) /* bizarre option, really */
	{	ddata->width  = pdata->max_width;
		ddata->height = pdata->max_height;
	}
	else
	{	ddata->width  = (unsigned int) ceil ((double) wmf_width );
		ddata->height = (unsigned int) ceil ((double) wmf_height);
	}

	if (status == 0)
	{	err = wmf_play (API,0,&(pdata->options.bbox));
		status = explicit_wmf_error ("wmf_play",err);
	}

	wmf_api_destroy (API);

	return (status);
}

void wmf2plot_init (PlotData* pdata,int argc,char** argv)
{	pdata->argc = argc;
	pdata->argv = argv;

	pdata->auto_files = 0;
	pdata->wmf_filename = 0;
	pdata->plot_filename = 0;

	pdata->options.type = wmf_plot_svg;

	/* */

	pdata->options.width  = 0;
	pdata->options.height = 0;

	pdata->options.flags = 0;

	pdata->max_width  = 768;
	pdata->max_height = 512;

	pdata->max_flags = 0;
}

void wmf2plot_help (PlotData* pdata)
{	fputs ("\
Usage: wmf2plot [OPTION]... [-o <file.png>] <file.wmf>\n\
  or:  wmf2plot [OPTION]... [-t jpeg] [-o <file.jpg>] <file.wmf>\n\
  or:  wmf2plot [OPTION]... --auto <file1.wmf> [<file2.wmf> ...]\n\
Convert metafile image to one of: png,jpg.\n\
\n\
  -t <type>       where <type> is one of: generic,bitmap,meta,tek,\n\
                  regis,hpgl,pcl,fig,cgm,ps,ai,svg,gif,pnm,png,X.\n\
  --maxwidth=<w>  where <w> is maximum width image may have.\n\
  --maxheight=<h> where <h> is maximum height image may have.\n\
  --maxpect       scale image to maximum size keeping aspect.\n\
  --maxsize       scale image to maximum size.\n\
  --version       display version info and exit.\n\
  --help          display this help and exit.\n\
  --wmf-help      display wmf-related help and exit.\n\
\n\
Report bugs to <http://www.wvware.com/>.\n",stdout);
}

int wmf2plot_args (PlotData* pdata)
{	int status = 0;
	int arg = 0;

	int    argc = pdata->argc;
	char** argv = pdata->argv;

	char* page = 0;

	while ((++arg) < argc)
	{	if (strcmp (argv[arg],"--help") == 0)
		{	wmf2plot_help (pdata);
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

		if (strncmp (argv[arg],"--maxwidth=",11) == 0)
		{	if (sscanf (argv[arg]+11,"%u",&(pdata->max_width)) != 1)
			{	fputs ("usage: --maxwidth=<width>, where <width> is +ve integer.\n",stderr);
				status = arg;
				break;
			}
			continue;
		}
		if (strncmp (argv[arg],"--maxheight=",12) == 0)
		{	if (sscanf (argv[arg]+12,"%u",&(pdata->max_height)) != 1)
			{	fputs ("usage: --maxheight=<height>, where <height> is +ve integer.\n",stderr);
				status = arg;
				break;
			}
			continue;
		}

		if (strcmp (argv[arg],"-t") == 0)
		{	if ((++arg) < argc)
			{	if (strcmp (argv[arg],"generic") == 0)
				{	pdata->options.type = wmf_plot_generic;
					continue;
				}
				if (strcmp (argv[arg],"bitmap") == 0)
				{	pdata->options.type = wmf_plot_bitmap;
					continue;
				}
				if (strcmp (argv[arg],"meta") == 0)
				{	pdata->options.type = wmf_plot_meta;
					continue;
				}
				if (strcmp (argv[arg],"tek") == 0)
				{	pdata->options.type = wmf_plot_tek;
					continue;
				}
				if (strcmp (argv[arg],"regis") == 0)
				{	pdata->options.type = wmf_plot_regis;
					continue;
				}
				if (strcmp (argv[arg],"hpgl") == 0)
				{	pdata->options.type = wmf_plot_hpgl;
					continue;
				}
				if (strcmp (argv[arg],"pcl") == 0)
				{	pdata->options.type = wmf_plot_pcl;
					continue;
				}
				if (strcmp (argv[arg],"fig") == 0)
				{	pdata->options.type = wmf_plot_fig;
					continue;
				}
				if (strcmp (argv[arg],"cgm") == 0)
				{	pdata->options.type = wmf_plot_cgm;
					continue;
				}
				if (strcmp (argv[arg],"ps") == 0)
				{	pdata->options.type = wmf_plot_ps;
					continue;
				}
				if (strcmp (argv[arg],"ai") == 0)
				{	pdata->options.type = wmf_plot_ai;
					continue;
				}
				if (strcmp (argv[arg],"svg") == 0)
				{	pdata->options.type = wmf_plot_svg;
					continue;
				}
				if (strcmp (argv[arg],"gif") == 0)
				{	pdata->options.type = wmf_plot_gif;
					continue;
				}
				if (strcmp (argv[arg],"pnm") == 0)
				{	pdata->options.type = wmf_plot_pnm;
					continue;
				}
				if (strcmp (argv[arg],"png") == 0)
				{	pdata->options.type = wmf_plot_png;
					continue;
				}
				if (strcmp (argv[arg],"X") == 0)
				{	pdata->options.type = wmf_plot_X;
					continue;
				}
				fprintf (stderr,"wmf2plot: Sorry! type `%s' not supported!\n",argv[arg]);
				status = arg;
				break;
			}
			fprintf (stderr,"usage: `wmf2plot -t <type> <file.wmf>'.\n");
			fprintf (stderr,"Try `%s --help' for more information.\n",argv[0]);
			status = arg;
			break;
		}

		if (strcmp (argv[arg],"--maxpect") == 0)
		{	pdata->max_flags = WMF2PLOT_MAXPECT;
			continue;
		}
		if (strcmp (argv[arg],"--maxsize") == 0)
		{	pdata->max_flags = WMF2PLOT_MAXSIZE;
			continue;
		}

		if (strcmp (argv[arg],"--auto") == 0)
		{	pdata->auto_files = argv + arg + 1;
			break;
		}
		if (strcmp (argv[arg],"-o") == 0)
		{	if ((++arg) < argc)
			{	pdata->plot_filename = argv[arg];
				continue;
			}
			fprintf (stderr,"usage: `wmf2plot -o <file.plot> <file.wmf>'.\n");
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

int wmf2plot_auto (PlotData* pdata)
{	int status = 0;

	while (1)
	{	pdata->wmf_filename = (*(pdata->auto_files));

		if (pdata->wmf_filename == 0) break;

		if (strcmp (pdata->wmf_filename+strlen(pdata->wmf_filename)-4,".wmf"))
		{	fprintf (stderr,"%s: expected suffix `.wmf'. ",pdata->wmf_filename);
			fprintf (stderr,"skipping...\n");
			status++;
		}
		else
		{	pdata->plot_filename = malloc (strlen(pdata->wmf_filename)+5);

			if (pdata->plot_filename == 0)
			{	fprintf (stderr,"mem_alloc_err: skipping %s...\n",pdata->wmf_filename);
				status++;
				continue;
			}

			strcpy (pdata->plot_filename,pdata->wmf_filename);
			switch (pdata->options.type)
			{
			case wmf_plot_generic:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"generic");
			break;

			case wmf_plot_bitmap:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"bitmap");
			break;

			case wmf_plot_meta:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"meta");
			break;

			case wmf_plot_tek:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"tek");
			break;

			case wmf_plot_regis:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"regis");
			break;

			case wmf_plot_hpgl:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"hpgl");
			break;

			case wmf_plot_pcl:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"pcl");
			break;

			case wmf_plot_fig:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"fig");
			break;

			case wmf_plot_cgm:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"cgm");
			break;

			case wmf_plot_ps:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"ps");
			break;

			case wmf_plot_ai:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"ai");
			break;

			case wmf_plot_svg:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"svg");
			break;

			case wmf_plot_gif:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"gif");
			break;

			case wmf_plot_pnm:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"pnm");
			break;

			case wmf_plot_png:
				strcpy (pdata->plot_filename+strlen(pdata->plot_filename)-3,"png");
			break;

			case wmf_plot_X:
				fprintf (stderr,"X with auto? skipping %s...\n",pdata->wmf_filename);
				free (pdata->plot_filename);
				pdata->plot_filename = 0;
			break;

			default:
				fprintf (stderr,"glitch: bad type: skipping %s...\n",pdata->wmf_filename);
				free (pdata->plot_filename);
				pdata->plot_filename = 0;
			break;
			}
			if (pdata->plot_filename)
			{	if (wmf2plot_file (pdata)) status++;
				free (pdata->plot_filename);
			}
		}
		
		pdata->auto_files++;
	}

	return (status);
}

int wmf2plot_file (PlotData* pdata)
{	int status = 0;

	pdata->options.file = stdout;
	if (pdata->plot_filename && (pdata->options.type != wmf_plot_X))
	{	if ((pdata->options.file = fopen (pdata->plot_filename,"w")) == 0)
		{	fprintf (stderr,"unable to write to `%s'. ",pdata->plot_filename);
			fprintf (stderr,"skipping...\n");
			status = 1;
			return (status);
		}
	}

	status = wmf2plot_draw (pdata);

	if (pdata->options.file != stdout) fclose (pdata->options.file);

	return (status);
}

int main (int argc,char** argv)
{	int status = 0;

	PlotData PData;

	wmf2plot_init (&PData,argc,argv);

	status = wmf2plot_args (&PData);

	if (status) return (status);

	if (PData.auto_files) status = wmf2plot_auto (&PData);
	else                  status = wmf2plot_file (&PData);

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

#endif /* ! HAVE_LIBPLOT */
