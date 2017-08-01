/* libwmf ("ipa/fig/font.h"): library for wmf conversion
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


static PS_to_FIG PSFontNo[35] = {
	{ "Times-Roman",			 0 },
	{ "Times-Italic",			 1 },
	{ "Times-Bold",				 2 },
	{ "Times-BoldItalic",			 3 },
	{ "AvantGarde-Book",			 4 },
	{ "AvantGarde-BookOblique",		 5 },
	{ "AvantGarde-Demi",			 6 },
	{ "AvantGarde-DemiOblique",		 7 },
	{ "Bookman-Light",			 8 },
	{ "Bookman-LightItalic",		 9 },
	{ "Bookman-Demi",			10 },
	{ "Bookman-DemiItalic",			11 },
	{ "Courier",				12 },
	{ "Courier-Oblique",			13 },
	{ "Courier-Bold",			14 },
	{ "Courier-BoldOblique",		15 },
	{ "Helvetica",				16 },
	{ "Helvetica-Oblique",			17 },
	{ "Helvetica-Bold",			18 },
	{ "Helvetica-BoldOblique",		19 },
	{ "Helvetica-Narrow",			20 },
	{ "Helvetica-Narrow-Oblique",		21 },
	{ "Helvetica-Narrow-Bold",		22 },
	{ "Helvetica-Narrow-BoldOblique",	23 },
	{ "NewCenturySchlbk-Roman",		24 },
	{ "NewCenturySchlbk-Italic",		25 },
	{ "NewCenturySchlbk-Bold",		26 },
	{ "NewCenturySchlbk-BoldItalic",	27 },
	{ "Palatino-Roman",			28 },
	{ "Palatino-Italic",			29 },
	{ "Palatino-Bold",			30 },
	{ "Palatino-BoldItalic",		31 },
	{ "Symbol",				32 },
	{ "ZapfChancery-MediumItalic",		33 },
	{ "ZapfDingbats",			34 }};

static void wmf_fig_draw_text (wmfAPI* API,wmfDrawText_t* draw_text)
{	wmf_fig_t* ddata = WMF_FIG_GetData (API);

	wmfStream* out = ddata->out;

	wmfFont* font = 0;

	figPoint pt;

	char* ps_name = 0;

	float ratio;
	float theta;

	int i;
	int fig_font;
	int sub_type;
	int pen_style = 0;

	unsigned int size;
	unsigned int length;
	unsigned int font_flags;

	unsigned long pen_color;
	unsigned long bg_color;
	unsigned long height;

	WMF_DEBUG (API,"~~~~~~~~wmf_[fig_]draw_text");

	if (out == 0) return;

	if (WMF_DC_OPAQUE (draw_text->dc))
	{	bg_color = wmf_ipa_color_index (API,WMF_DC_BACKGROUND (draw_text->dc));

		/* TODO */
	}

	font = WMF_DC_FONT (draw_text->dc);

	height = draw_text->font_height;

	ratio = (float) draw_text->font_ratio;

	/* In radians: */
	theta = (float) (WMF_TEXT_ANGLE (font));

	ddata->depth -= ddata->ddec;

	/* PS points / FIG units */
	size = (fig_height (API,height) * 72) / ddata->dpi;

	pt = fig_translate (API,draw_text->pt);

	length = 0;
	
	wmf_stream_printf (API,out,"# wmf_[fig_]draw_text\n");

	pen_color = wmf_ipa_color_index (API,WMF_DC_TEXTCOLOR (draw_text->dc));
	
	fig_font = -1; /* Default. */
	ps_name = WMF_FONT_PSNAME (font);
	for (i = 0; i < 35; i++)
	{	if (strcmp (ps_name,PSFontNo[i].PS_FontName) == 0)
		{	fig_font = PSFontNo[i].FIG_FontNumber;
		}
	}

	sub_type = 0; /* Left justified for now; interpreter should do rest... */
	
	font_flags = 0x4; /* PostScript font for now */
	
	wmf_stream_printf (API,out,
		"%d %d %lu %d %d %d %u %f %u %lu %u %d %d ",
		O_TEXT,
		sub_type,
		pen_color,
		ddata->depth,
		pen_style,
		fig_font,
		size,
		theta,
		font_flags,
		height,
		length,
		pt.x,
		pt.y);

	wmf_stream_printf (API,out,"%s\\001\n",draw_text->str);

	wmf_stream_printf (API,out,"# end draw_text\n");
}

