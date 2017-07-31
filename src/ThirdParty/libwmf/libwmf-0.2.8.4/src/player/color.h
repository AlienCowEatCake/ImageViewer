/* libwmf ("player/color.h"): library for wmf conversion
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


#ifndef WMFPLAYER_COLOR_H
#define WMFPLAYER_COLOR_H

/* These are the colors defined in the SVG standard (I haven't checked the final recommendation for changes)
 */
wmfRGB wmf_aliceblue		= { 240, 248, 255 };
wmfRGB wmf_antiquewhite		= { 250, 235, 215 };
wmfRGB wmf_aqua			= {   0, 255, 255 };
wmfRGB wmf_aquamarine		= { 127, 255, 212 };
wmfRGB wmf_azure		= { 240, 255, 255 };
wmfRGB wmf_beige		= { 245, 245, 220 };
wmfRGB wmf_bisque		= { 255, 228, 196 };
wmfRGB wmf_black		= {   0,   0,   0 };
wmfRGB wmf_blanchedalmond	= { 255, 235, 205 };
wmfRGB wmf_blue			= {   0,   0, 255 };
wmfRGB wmf_blueviolet		= { 138,  43, 226 };
wmfRGB wmf_brown		= { 165,  42,  42 };
wmfRGB wmf_burlywood		= { 222, 184, 135 };
wmfRGB wmf_cadetblue		= {  95, 158, 160 };
wmfRGB wmf_chartreuse		= { 127, 255,   0 };
wmfRGB wmf_chocolate		= { 210, 105,  30 };
wmfRGB wmf_coral		= { 255, 127,  80 };
wmfRGB wmf_cornflowerblue	= { 100, 149, 237 };
wmfRGB wmf_cornsilk		= { 255, 248, 220 };
wmfRGB wmf_crimson		= { 220,  20,  60 };
wmfRGB wmf_cyan			= {   0, 255, 255 };
wmfRGB wmf_darkblue		= {   0,   0, 139 };
wmfRGB wmf_darkcyan		= {   0, 139, 139 };
wmfRGB wmf_darkgoldenrod	= { 184, 134,  11 };
wmfRGB wmf_darkgray		= { 169, 169, 169 };
wmfRGB wmf_darkgreen		= {   0, 100,   0 };
wmfRGB wmf_darkgrey		= { 169, 169, 169 };
wmfRGB wmf_darkkhaki		= { 189, 183, 107 };
wmfRGB wmf_darkmagenta		= { 139,   0, 139 };
wmfRGB wmf_darkolivegreen	= {  85, 107,  47 };
wmfRGB wmf_darkorange		= { 255, 140,   0 };
wmfRGB wmf_darkorchid		= { 153,  50, 204 };
wmfRGB wmf_darkred		= { 139,   0,   0 };
wmfRGB wmf_darksalmon		= { 233, 150, 122 };
wmfRGB wmf_darkseagreen		= { 143, 188, 143 };
wmfRGB wmf_darkslateblue	= {  72,  61, 139 };
wmfRGB wmf_darkslategray	= {  47,  79,  79 };
wmfRGB wmf_darkslategrey	= {  47,  79,  79 };
wmfRGB wmf_darkturquoise	= {   0, 206, 209 };
wmfRGB wmf_darkviolet		= { 148,   0, 211 };
wmfRGB wmf_deeppink		= { 255,  20, 147 };
wmfRGB wmf_deepskyblue		= {   0, 191, 255 };
wmfRGB wmf_dimgray		= { 105, 105, 105 };
wmfRGB wmf_dimgrey		= { 105, 105, 105 };
wmfRGB wmf_dodgerblue		= {  30, 144, 255 };
wmfRGB wmf_firebrick		= { 178,  34,  34 };
wmfRGB wmf_floralwhite		= { 255, 250, 240 };
wmfRGB wmf_forestgreen		= {  34, 139,  34 };
wmfRGB wmf_fuchsia		= { 255,   0, 255 };
wmfRGB wmf_gainsboro		= { 220, 220, 220 };
wmfRGB wmf_ghostwhite		= { 248, 248, 255 };
wmfRGB wmf_gold			= { 255, 215,   0 };
wmfRGB wmf_goldenrod		= { 218, 165,  32 };
wmfRGB wmf_gray			= { 128, 128, 128 };
wmfRGB wmf_grey			= { 128, 128, 128 };
wmfRGB wmf_green		= {   0, 128,   0 };
wmfRGB wmf_greenyellow		= { 173, 255,  47 };
wmfRGB wmf_honeydew		= { 240, 255, 240 };
wmfRGB wmf_hotpink		= { 255, 105, 180 };
wmfRGB wmf_indianred		= { 205,  92,  92 };
wmfRGB wmf_indigo		= {  75,   0, 130 };
wmfRGB wmf_ivory		= { 255, 255, 240 };
wmfRGB wmf_khaki		= { 240, 230, 140 };
wmfRGB wmf_lavender		= { 230, 230, 250 };
wmfRGB wmf_lavenderblush	= { 255, 240, 245 };
wmfRGB wmf_lawngreen		= { 124, 252,   0 };
wmfRGB wmf_lemonchiffon		= { 255, 250, 205 };
wmfRGB wmf_lightblue		= { 173, 216, 230 };
wmfRGB wmf_lightcoral		= { 240, 128, 128 };
wmfRGB wmf_lightcyan		= { 224, 255, 255 };
wmfRGB wmf_lightgoldenrodyellow	= { 250, 250, 210 };
wmfRGB wmf_lightgray		= { 211, 211, 211 };
wmfRGB wmf_lightgreen		= { 144, 238, 144 };
wmfRGB wmf_lightgrey		= { 211, 211, 211 };
wmfRGB wmf_lightpink		= { 255, 182, 193 };
wmfRGB wmf_lightsalmon		= { 255, 160, 122 };
wmfRGB wmf_lightseagreen	= {  32, 178, 170 };
wmfRGB wmf_lightskyblue		= { 135, 206, 250 };
wmfRGB wmf_lightslategray	= { 119, 136, 153 };
wmfRGB wmf_lightslategrey	= { 119, 136, 153 };
wmfRGB wmf_lightsteelblue	= { 176, 196, 222 };
wmfRGB wmf_lightyellow		= { 255, 255, 224 };
wmfRGB wmf_lime			= {   0, 255,   0 };
wmfRGB wmf_limegreen		= {  50, 205,  50 };
wmfRGB wmf_linen		= { 250, 240, 230 };
wmfRGB wmf_magenta		= { 255,   0, 255 };
wmfRGB wmf_maroon		= { 128,   0,   0 };
wmfRGB wmf_mediumaquamarine	= { 102, 205, 170 };
wmfRGB wmf_mediumblue		= {   0,   0, 205 };
wmfRGB wmf_mediumorchid		= { 186,  85, 211 };
wmfRGB wmf_mediumpurple		= { 147, 112, 219 };
wmfRGB wmf_mediumseagreen	= {  60, 179, 113 };
wmfRGB wmf_mediumslateblue	= { 123, 104, 238 };
wmfRGB wmf_mediumspringgreen	= {   0, 250, 154 };
wmfRGB wmf_mediumturquoise	= {  72, 209, 204 };
wmfRGB wmf_mediumvioletred	= { 199,  21, 133 };
wmfRGB wmf_midnightblue		= {  25,  25, 112 };
wmfRGB wmf_mintcream		= { 245, 255, 250 };
wmfRGB wmf_mistyrose		= { 255, 228, 225 };
wmfRGB wmf_moccasin		= { 255, 228, 181 };
wmfRGB wmf_navajowhite		= { 255, 222, 173 };
wmfRGB wmf_navy			= {   0,   0, 128 };
wmfRGB wmf_oldlace		= { 253, 245, 230 };
wmfRGB wmf_olive		= { 128, 128,   0 };
wmfRGB wmf_olivedrab		= { 107, 142,  35 };
wmfRGB wmf_orange		= { 255, 165,   0 };
wmfRGB wmf_orangered		= { 255,  69,   0 };
wmfRGB wmf_orchid		= { 218, 112, 214 };
wmfRGB wmf_palegoldenrod	= { 238, 232, 170 };
wmfRGB wmf_palegreen		= { 152, 251, 152 };
wmfRGB wmf_paleturquoise	= { 175, 238, 238 };
wmfRGB wmf_palevioletred	= { 219, 112, 147 };
wmfRGB wmf_papayawhip		= { 255, 239, 213 };
wmfRGB wmf_peachpuff		= { 255, 218, 185 };
wmfRGB wmf_peru			= { 205, 133,  63 };
wmfRGB wmf_pink			= { 255, 192, 203 };
wmfRGB wmf_plum			= { 221, 160, 221 };
wmfRGB wmf_powderblue		= { 176, 224, 230 };
wmfRGB wmf_purple		= { 128,   0, 128 };
wmfRGB wmf_red			= { 255,   0,   0 };
wmfRGB wmf_rosybrown		= { 188, 143, 143 };
wmfRGB wmf_royalblue		= {  65, 105, 225 };
wmfRGB wmf_saddlebrown		= { 139,  69,  19 };
wmfRGB wmf_salmon		= { 250, 128, 114 };
wmfRGB wmf_sandybrown		= { 244, 164,  96 };
wmfRGB wmf_seagreen		= {  46, 139,  87 };
wmfRGB wmf_seashell		= { 255, 245, 238 };
wmfRGB wmf_sienna		= { 160,  82,  45 };
wmfRGB wmf_silver		= { 192, 192, 192 };
wmfRGB wmf_skyblue		= { 135, 206, 235 };
wmfRGB wmf_slateblue		= { 106,  90, 205 };
wmfRGB wmf_slategray		= { 112, 128, 144 };
wmfRGB wmf_slategrey		= { 112, 128, 144 };
wmfRGB wmf_snow			= { 255, 250, 250 };
wmfRGB wmf_springgreen		= {   0, 255, 127 };
wmfRGB wmf_steelblue		= {  70, 130, 180 };
wmfRGB wmf_tan			= { 210, 180, 140 };
wmfRGB wmf_teal			= {   0, 128, 128 };
wmfRGB wmf_thistle		= { 216, 191, 216 };
wmfRGB wmf_tomato		= { 255,  99,  71 };
wmfRGB wmf_turquoise		= {  64, 224, 208 };
wmfRGB wmf_violet		= { 238, 130, 238 };
wmfRGB wmf_wheat		= { 245, 222, 179 };
wmfRGB wmf_white		= { 255, 255, 255 };
wmfRGB wmf_whitesmoke		= { 245, 245, 245 };
wmfRGB wmf_yellow		= { 255, 255,   0 };
wmfRGB wmf_yellowgreen		= { 154, 205,  50 };

/* Color table
 */

/**
 * Initialize internal color table
 * 
 * @param API the API handle
 */
void wmf_ipa_color_init (wmfAPI* API)
{	wmfColorData* color;

	API->color_data = wmf_malloc (API,sizeof (wmfColorData));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	color = (wmfColorData*) API->color_data;

	color->max = 32;
	color->count = 0;
	color->rgb = (wmfRGB*) wmf_malloc (API,color->max * sizeof (wmfRGB));

	if (ERR (API))
	{	WMF_DEBUG (API,"bailing...");
		return;
	}

	color->rgb[0] = wmf_red; /* not a real entry; don't increment count... */
}

/**
 * Add a color to the internal table.
 * 
 * @param API the API handle
 * @param rgb pointer to the color to be added
 */
void wmf_ipa_color_add (wmfAPI* API,wmfRGB* rgb)
{	wmfRGB* more = 0;

	wmfColorData* color = (wmfColorData*) API->color_data;

	unsigned long i;

	int new_color = 1;

	for (i = 0; i < color->count; i++)
	{	if ( (rgb->r == color->rgb[i].r)
		  && (rgb->g == color->rgb[i].g)
		  && (rgb->b == color->rgb[i].b))
		{	new_color = 0;
			break;
		}
	}
	if (!new_color) return;

	if (color->count == color->max)
	{	more = wmf_realloc (API,color->rgb,(color->max + 32) * sizeof (wmfRGB));

		if (ERR (API))
		{	WMF_DEBUG (API,"bailing...");
			return;
		}

		color->rgb = more;
		color->max += 32;
	}

	color->rgb[color->count] = (*rgb);
	color->count++;
}

/**
 * Find closest matching color in internal table and return its index.
 * 
 * @param API the API handle
 * @param rgb pointer to the color to be matched
 * 
 * @return Returns the index of the closest matching color.
 */
unsigned long wmf_ipa_color_index (wmfAPI* API,wmfRGB* rgb)
{	wmfColorData* color = (wmfColorData*) API->color_data;

	unsigned long i;
	unsigned long best = 0;

	unsigned int dbest = 766;
	unsigned int di;
	unsigned int dr;
	unsigned int dg;
	unsigned int db;

	if (color->count == 0)
	{	if (API->flags & WMF_OPT_IGNORE_NONFATAL)
		{	WMF_DEBUG (API,"Color table has no entries!");
		}
		else
		{	WMF_ERROR (API,"Color table has no entries!");
			API->err = wmf_E_Glitch;
		}
		return (best); /* This should dereference safely to red. */
	}

	/* First check for an exact match...
	 */
	for (i = 0; i < color->count; i++)
	{	if ((rgb->r == color->rgb[i].r)
		 && (rgb->g == color->rgb[i].g)
		 && (rgb->b == color->rgb[i].b))
		{	 best = i;
			dbest = 0;
			break;
		}
	}
	if (dbest == 0) return (best);

	/* Otherwise return best match; I'm going to use minimax rather than r.m.s.
	 */
	for (i = 0; i < color->count; i++)
	{	dr = (unsigned int) ABS (((int) (rgb->r)) - ((int) (color->rgb[i].r)));
		dg = (unsigned int) ABS (((int) (rgb->g)) - ((int) (color->rgb[i].g)));
		db = (unsigned int) ABS (((int) (rgb->b)) - ((int) (color->rgb[i].b)));

		di = MAX (dr,dg);
		di = MAX (di,db);

		if (di < dbest)
		{	 best =  i;
			dbest = di;
		}
	}

	return (best);
}

/**
 * Get the number of indexed colors.
 * 
 * @param API the API handle
 * 
 * @return Returns the number of indexed colors.
 */
unsigned long wmf_ipa_color_count (wmfAPI* API)
{	wmfColorData* color = (wmfColorData*) API->color_data;

	return (color->count);
}

/**
 * Get indexed color.
 * 
 * @param API   the API handle
 * @param index the index of the color
 * 
 * @return Returns pointer to the indexed color.
 */
wmfRGB* wmf_ipa_color (wmfAPI* API,unsigned long index)
{	wmfColorData* color = (wmfColorData*) API->color_data;

	if (index >= color->count)
	{	WMF_ERROR (API,"Glitch! Color index out of range!");
		API->err = wmf_E_Glitch;
		return (&wmf_red);
	}

	return (color->rgb + index);
}

static wmfRGB rgb (wmfAPI* API,U16 one,U16 two)
{	wmfRGB color;

	color.r = (unsigned char)  (one & 0x00FF);
	color.g = (unsigned char) ((one & 0xFF00) >> 8);
	color.b = (unsigned char)  (two & 0x00FF);

	return (color);
}

/**
 * The color white.
 * 
 * @param API the API handle
 * 
 * @return Returns \b wmf_white.
 */
wmfRGB wmf_rgb_white (wmfAPI* API)
{	return (wmf_white);
}

/**
 * The color black.
 * 
 * @param API the API handle
 * 
 * @return Returns \b wmf_black.
 */
wmfRGB wmf_rgb_black (wmfAPI* API)
{	return (wmf_black);
}

/**
 * Create a color with specified fractions of red, green and blue.
 * 
 * @param API   the API handle
 * @param red   fraction (0 to 1 inclusive) of color red
 * @param green fraction (0 to 1 inclusive) of color green
 * @param blue  fraction (0 to 1 inclusive) of color blue
 * 
 * \b wmf_rgb_color (API,1,1,1) returns \b wmf_white.
 * 
 * \b wmf_rgb_color (API,0,0,0) returns \b wmf_black.
 * 
 * @return Returns the color.
 */
wmfRGB wmf_rgb_color (wmfAPI* API,float red,float green,float blue)
{	wmfRGB color;

	int i_red;
	int i_green;
	int i_blue;

	if (red	  > 1) red   = 1;
	if (green > 1) green = 1;
	if (blue  > 1) blue  = 1;

	if (red	  < 0) red   = 0;
	if (green < 0) green = 0;
	if (blue  < 0) blue  = 0;

	i_red   = (int) (red   * (float) 256);
	i_green = (int) (green * (float) 256);
	i_blue  = (int) (blue  * (float) 256);

	if (i_red   > 255) i_red   = 255;
	if (i_green > 255) i_green = 255;
	if (i_blue  > 255) i_blue  = 255;

	if (i_red   < 0) i_red   = 0;
	if (i_green < 0) i_green = 0;
	if (i_blue  < 0) i_blue  = 0;

	color.r = (unsigned char) i_red;
	color.g = (unsigned char) i_green;
	color.b = (unsigned char) i_blue;

	return (color);
}

#endif /* ! WMFPLAYER_COLOR_H */
