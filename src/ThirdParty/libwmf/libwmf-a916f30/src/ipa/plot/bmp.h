/* libwmf ("ipa/plot/bmp.h"): library for wmf conversion
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


static void wmf_plot_rop_draw (wmfAPI* API,wmfROP_Draw_t* ropdraw)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]rop_draw");

	
}

static void wmf_plot_bmp_draw (wmfAPI* API,wmfBMP_Draw_t* bmp_draw)
{	wmf_plot_t* ddata = WMF_PLOT_GetData (API);

	plot_t* plot = (plot_t*) ddata->plot_data;

	WMF_DEBUG (API,"wmf_[plot_]bmp_draw");

	
}

static void wmf_plot_bmp_read (wmfAPI* API,wmfBMP_Read_t* bmp_read)
{	WMF_DEBUG (API,"wmf_[plot_]bmp_read");

	wmf_ipa_bmp_read (API,bmp_read);
}

static void wmf_plot_bmp_free (wmfAPI* API,wmfBMP* bmp)
{	WMF_DEBUG (API,"wmf_[plot_]bmp_free");

	wmf_ipa_bmp_free (API,bmp);
}
