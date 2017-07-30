/* libwmf ("ipa/eps/bmp.h"): library for wmf conversion
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


static void wmf_eps_rop_draw (wmfAPI* API,wmfROP_Draw_t* rop_draw)
{	/* wmf_eps_t* ddata = WMF_EPS_GetData (API); */

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]rop_draw");

	
}

/* TODO ?? Care about bmp_draw->type
 */
static void wmf_eps_bmp_draw (wmfAPI* API,wmfBMP_Draw_t* bmp_draw)
{	wmf_eps_t* ddata = WMF_EPS_GetData (API);

	static const char hex[16] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

	wmfStream* out = ddata->out;

	U16 x;
	U16 y;
	U16 i;

	unsigned int ui_x;
	unsigned int ui_y;

	char buffer[80];

	float width;
	float height;

	wmfRGB rgb;

	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]bmp_draw");

	if (out == 0) return;

	wmf_stream_printf (API,out,"gsave %% wmf_[eps_]bmp_draw\n");

	wmf_stream_printf (API,out," %f %f translate\n",bmp_draw->pt.x,bmp_draw->pt.y);

	width  = (float) ((double) bmp_draw->crop.w * bmp_draw->pixel_width );
	height = (float) ((double) bmp_draw->crop.h * bmp_draw->pixel_height);

	wmf_stream_printf (API,out," 0 %f translate\n",height);
	wmf_stream_printf (API,out," %f %f scale\n",width,-height);

	/* I'm going to assume it's a color image - TODO: monochrome */

	wmf_stream_printf (API,out," /picstr %u 3 mul string def\n",bmp_draw->crop.w);

	wmf_stream_printf (API,out," %u %u 8\n",(unsigned int) bmp_draw->crop.w,(unsigned int) bmp_draw->crop.h);
	wmf_stream_printf (API,out," [ %u 0 0 %u 0 0 ]\n",(unsigned int) bmp_draw->crop.w,(unsigned int) bmp_draw->crop.h);

	wmf_stream_printf (API,out," { currentfile picstr readhexstring pop } false 3\n");
	wmf_stream_printf (API,out," colorimage\n");

	for (y = 0; y < bmp_draw->crop.h; y++)
	{	ui_y = (unsigned int) (y + bmp_draw->crop.y);

		i = 0;
		for (x = 0; x < bmp_draw->crop.w; x++)
		{	ui_x = (unsigned int) (x + bmp_draw->crop.x);

			if (i == 78)
			{	buffer[i++] = '\n';
				buffer[i] = 0;
				wmf_stream_printf (API,out,buffer);
				i = 0;
			}

			wmf_ipa_bmp_color (API,&(bmp_draw->bmp),&rgb,ui_x,ui_y);

			buffer[i++] = hex[(rgb.r & 0xf0) >> 4];
			buffer[i++] = hex[ rgb.r & 0x0f      ];
			buffer[i++] = hex[(rgb.g & 0xf0) >> 4];
			buffer[i++] = hex[ rgb.g & 0x0f      ];
			buffer[i++] = hex[(rgb.b & 0xf0) >> 4];
			buffer[i++] = hex[ rgb.b & 0x0f      ];
		}
		if (i > 0)
		{	buffer[i++] = '\n';
			buffer[i] = 0;
			wmf_stream_printf (API,out,buffer);
		}
	}

	wmf_stream_printf (API,out,"grestore\n");
}

static void wmf_eps_bmp_read (wmfAPI* API,wmfBMP_Read_t* bmp_read)
{	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]bmp_read");

	wmf_ipa_bmp_read (API,bmp_read);
}

static void wmf_eps_bmp_free (wmfAPI* API,wmfBMP* bmp)
{	WMF_DEBUG (API,"~~~~~~~~wmf_[eps_]bmp_free");

	wmf_ipa_bmp_free (API,bmp);
}
