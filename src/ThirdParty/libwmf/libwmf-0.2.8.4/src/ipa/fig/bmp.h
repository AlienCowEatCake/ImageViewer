/* libwmf ("ipa/fig/bmp.h"): library for wmf conversion
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


static void wmf_fig_rop_draw (wmfAPI* API,wmfROP_Draw_t* rop_draw)
{	/* wmf_fig_t* ddata = WMF_FIG_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]rop_draw");

	
}

/* TODO ?? Care about bmp_draw->type
 */
static void wmf_fig_bmp_draw (wmfAPI* API,wmfBMP_Draw_t* bmp_draw)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	figDC fig;

	figPoint pt;

	int width;
	int height;

	int left;
	int right;
	int top;
	int bottom;
	
   	int npoints = 5;
	int flipped = 0;

	char* name = 0;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]bmp_draw");

	if (out == 0) return;

	if (bmp_draw->bmp.data == 0) return;

	if (ddata->image.name == 0) return;

	name = ddata->image.name (ddata->image.context);

	if (name == 0) return;

	if (WMF_FIG_ImageIsEPS (ddata))
	{	wmf_ipa_bmp_eps (API,bmp_draw,name);
	}
	else if (WMF_FIG_ImageIsPNG (ddata))
	{	wmf_ipa_bmp_png (API,bmp_draw,name);
	}
	else if (WMF_FIG_ImageIsJPG (ddata))
	{	wmf_ipa_bmp_jpg (API,bmp_draw,name);
	}
	else
	{	WMF_ERROR (API,"Glitch! Can't determine image format to use.");
		API->err = wmf_E_Glitch;
		return;
	}

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	/* Okay, if we've got this far then "name" is the filename of an eps (cropped) image */

	wmf_stream_printf (API,out,"# wmf_[fig_]bmp_draw\n");

	fig_set_style (API,bmp_draw->dc,&fig);

	ddata->depth -= ddata->ddec;

	wmf_stream_printf (API,out,
		"%d %d %d %d %d %d %d %d %d %f %d %d %d %d %d %d\n",
		O_POLYLINE,
		T_PICTURE,
		fig.line_style,
		fig.thickness,
		fig.pen_color,
		fig.fill_color,
		ddata->depth,
		fig.pen_style,
		fig.area_fill,
		fig.style_val,
		fig.join_style,
		fig.cap_style,
		fig.radius,
		fig.forward_arrow,
		fig.backward_arrow,
		npoints);

	width  = fig_width  (API,(float) ((double) bmp_draw->crop.w * bmp_draw->pixel_width ));
	height = fig_height (API,(float) ((double) bmp_draw->crop.h * bmp_draw->pixel_height));

	pt = fig_translate (API,bmp_draw->pt);

	left   = pt.x;
	right  = pt.x + width;
	top    = pt.y;
	bottom = pt.y + height;

	wmf_stream_printf (API,out,"%d %s\n",flipped,name);

	wmf_stream_printf (API,out,"%d %d ", left, top);
	wmf_stream_printf (API,out,"%d %d ", right,top);
	wmf_stream_printf (API,out,"%d %d ", right,bottom);
	wmf_stream_printf (API,out,"%d %d ", left, bottom);
	wmf_stream_printf (API,out,"%d %d\n",left, top);
}

static void wmf_fig_bmp_read (wmfAPI* API,wmfBMP_Read_t* bmp_read)
{	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]bmp_read");

	wmf_ipa_bmp_read (API,bmp_read);

	fig_bmp_add (API,bmp_read->bmp.data);
}

static void wmf_fig_bmp_free (wmfAPI* API,wmfBMP* bmp)
{	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]bmp_free");

	wmf_ipa_bmp_free (API,bmp);

	fig_bmp_remove (API,bmp->data);
}

static void fig_bmp_add (wmfAPI* API,void* bmp)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	fig_t* fig = (fig_t*) ddata->fig_data;

	int i;

	for (i = 0; i < NUMPATTERNS; i++)
	{	if (fig->equiv[i] == 0)
		{	fig->equiv[i] = bmp;
			break;
		}
	}
}

static int fig_bmp_pattern (wmfAPI* API,void* bmp)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	fig_t* fig = (fig_t*) ddata->fig_data;

	int i;

	for (i = 0; i < NUMPATTERNS; i++)
	{	if (fig->equiv[i] == bmp) break;
	}

	return ((i < NUMPATTERNS) ? (41 + i) : -1);
}

static void fig_bmp_remove (wmfAPI* API,void* bmp)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	fig_t* fig = (fig_t*) ddata->fig_data;

	int i;

	for (i = 0; i < NUMPATTERNS; i++)
	{	if (fig->equiv[i] == bmp)
		{	fig->equiv[i] = 0;
			break;
		}
	}
}
