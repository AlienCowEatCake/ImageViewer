/* libwmf ("ipa/fig/color.h"): library for wmf conversion
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


static fig_color_t fig_color[32] = {
	{ 0.000, 0.000, 0.000 },	/* Black   */
	{ 0.000, 0.000, 1.000 },	/* Blue    */
	{ 0.000, 1.000, 0.000 },	/* Green   */
	{ 0.000, 1.000, 1.000 },	/* Cyan    */
	{ 1.000, 0.000, 0.000 },	/* Red     */
	{ 1.000, 0.000, 1.000 },	/* Magenta */
	{ 1.000, 1.000, 0.000 },	/* Yellow  */
	{ 1.000, 1.000, 1.000 },	/* White   */
	{ 0.000, 0.000, 0.560 },
	{ 0.000, 0.000, 0.690 },
	{ 0.000, 0.000, 0.820 },
	{ 0.530, 0.810, 1.000 },
	{ 0.000, 0.560, 0.000 },
	{ 0.000, 0.690, 0.000 },
	{ 0.000, 0.820, 0.000 },
	{ 0.000, 0.560, 0.560 },
	{ 0.000, 0.690, 0.690 },
	{ 0.000, 0.820, 0.820 },
	{ 0.560, 0.000, 0.000 },
	{ 0.690, 0.000, 0.000 },
	{ 0.820, 0.000, 0.000 },
	{ 0.560, 0.000, 0.560 },
	{ 0.690, 0.000, 0.690 },
	{ 0.820, 0.000, 0.820 },
	{ 0.500, 0.190, 0.000 },
	{ 0.630, 0.250, 0.000 },
	{ 0.750, 0.380, 0.000 },
	{ 1.000, 0.500, 0.500 },
	{ 1.000, 0.630, 0.630 },
	{ 1.000, 0.750, 0.750 },
	{ 1.000, 0.880, 0.880 },
	{ 1.000, 0.840, 0.000 }};

static void fig_std_colors (wmfAPI* API)
{	wmfRGB color;

	fig_color_t* fc = 0;

	int i;

	fc = fig_color;
	for (i = 0; i < 32; i++)
	{	color = wmf_rgb_color (API,fc->r,fc->g,fc->b);

		wmf_ipa_color_add (API,&color);

		fc++;
	}
}

static void fig_color_to_file (wmfAPI* API,wmfStream* out)
{	wmfRGB* rgb = 0;

	unsigned long count;
	unsigned long i;

	unsigned int r;
	unsigned int g;
	unsigned int b;

	count = wmf_ipa_color_count (API);

	/* Write all the collected extra colour from IPA color table
	 */
	for (i = 32; i < count; i++)
	{	rgb = wmf_ipa_color (API,i);

		r = (unsigned) rgb->r;
		g = (unsigned) rgb->g;
		b = (unsigned) rgb->b;

		wmf_stream_printf (API,out,"0 %lu #%02x%02x%02x\n",i,r,g,b);
	}
}
