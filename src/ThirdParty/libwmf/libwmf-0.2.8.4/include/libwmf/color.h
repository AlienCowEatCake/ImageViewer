/* libwmf (<libwmf/color.h>): library for wmf conversion
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


#ifndef LIBWMF_COLOR_H
#define LIBWMF_COLOR_H

#include <libwmf/ipa.h>

/* These are the color names specified by W3C for the SVG standard
 */
extern wmfRGB wmf_aliceblue;
extern wmfRGB wmf_antiquewhite;
extern wmfRGB wmf_aqua;
extern wmfRGB wmf_aquamarine;
extern wmfRGB wmf_azure;
extern wmfRGB wmf_beige;
extern wmfRGB wmf_bisque;
extern wmfRGB wmf_black;
extern wmfRGB wmf_blanchedalmond;
extern wmfRGB wmf_blue;
extern wmfRGB wmf_blueviolet;
extern wmfRGB wmf_brown;
extern wmfRGB wmf_burlywood;
extern wmfRGB wmf_cadetblue;
extern wmfRGB wmf_chartreuse;
extern wmfRGB wmf_chocolate;
extern wmfRGB wmf_coral;
extern wmfRGB wmf_cornflowerblue;
extern wmfRGB wmf_cornsilk;
extern wmfRGB wmf_crimson;
extern wmfRGB wmf_cyan;
extern wmfRGB wmf_darkblue;
extern wmfRGB wmf_darkcyan;
extern wmfRGB wmf_darkgoldenrod;
extern wmfRGB wmf_darkgray;
extern wmfRGB wmf_darkgreen;
extern wmfRGB wmf_darkgrey;
extern wmfRGB wmf_darkkhaki;
extern wmfRGB wmf_darkmagenta;
extern wmfRGB wmf_darkolivegreen;
extern wmfRGB wmf_darkorange;
extern wmfRGB wmf_darkorchid;
extern wmfRGB wmf_darkred;
extern wmfRGB wmf_darksalmon;
extern wmfRGB wmf_darkseagreen;
extern wmfRGB wmf_darkslateblue;
extern wmfRGB wmf_darkslategray;
extern wmfRGB wmf_darkslategrey;
extern wmfRGB wmf_darkturquoise;
extern wmfRGB wmf_darkviolet;
extern wmfRGB wmf_deeppink;
extern wmfRGB wmf_deepskyblue;
extern wmfRGB wmf_dimgray;
extern wmfRGB wmf_dimgrey;
extern wmfRGB wmf_dodgerblue;
extern wmfRGB wmf_firebrick;
extern wmfRGB wmf_floralwhite;
extern wmfRGB wmf_forestgreen;
extern wmfRGB wmf_fuchsia;
extern wmfRGB wmf_gainsboro;
extern wmfRGB wmf_ghostwhite;
extern wmfRGB wmf_gold;
extern wmfRGB wmf_goldenrod;
extern wmfRGB wmf_gray;
extern wmfRGB wmf_grey;
extern wmfRGB wmf_green;
extern wmfRGB wmf_greenyellow;
extern wmfRGB wmf_honeydew;
extern wmfRGB wmf_hotpink;
extern wmfRGB wmf_indianred;
extern wmfRGB wmf_indigo;
extern wmfRGB wmf_ivory;
extern wmfRGB wmf_khaki;
extern wmfRGB wmf_lavender;
extern wmfRGB wmf_lavenderblush;
extern wmfRGB wmf_lawngreen;
extern wmfRGB wmf_lemonchiffon;
extern wmfRGB wmf_lightblue;
extern wmfRGB wmf_lightcoral;
extern wmfRGB wmf_lightcyan;
extern wmfRGB wmf_lightgoldenrodyellow;
extern wmfRGB wmf_lightgray;
extern wmfRGB wmf_lightgreen;
extern wmfRGB wmf_lightgrey;
extern wmfRGB wmf_lightpink;
extern wmfRGB wmf_lightsalmon;
extern wmfRGB wmf_lightseagreen;
extern wmfRGB wmf_lightskyblue;
extern wmfRGB wmf_lightslategray;
extern wmfRGB wmf_lightslategrey;
extern wmfRGB wmf_lightsteelblue;
extern wmfRGB wmf_lightyellow;
extern wmfRGB wmf_lime;
extern wmfRGB wmf_limegreen;
extern wmfRGB wmf_linen;
extern wmfRGB wmf_magenta;
extern wmfRGB wmf_maroon;
extern wmfRGB wmf_mediumaquamarine;
extern wmfRGB wmf_mediumblue;
extern wmfRGB wmf_mediumorchid;
extern wmfRGB wmf_mediumpurple;
extern wmfRGB wmf_mediumseagreen;
extern wmfRGB wmf_mediumslateblue;
extern wmfRGB wmf_mediumspringgreen;
extern wmfRGB wmf_mediumturquoise;
extern wmfRGB wmf_mediumvioletred;
extern wmfRGB wmf_midnightblue;
extern wmfRGB wmf_mintcream;
extern wmfRGB wmf_mistyrose;
extern wmfRGB wmf_moccasin;
extern wmfRGB wmf_navajowhite;
extern wmfRGB wmf_navy;
extern wmfRGB wmf_oldlace;
extern wmfRGB wmf_olive;
extern wmfRGB wmf_olivedrab;
extern wmfRGB wmf_orange;
extern wmfRGB wmf_orangered;
extern wmfRGB wmf_orchid;
extern wmfRGB wmf_palegoldenrod;
extern wmfRGB wmf_palegreen;
extern wmfRGB wmf_paleturquoise;
extern wmfRGB wmf_palevioletred;
extern wmfRGB wmf_papayawhip;
extern wmfRGB wmf_peachpuff;
extern wmfRGB wmf_peru;
extern wmfRGB wmf_pink;
extern wmfRGB wmf_plum;
extern wmfRGB wmf_powderblue;
extern wmfRGB wmf_purple;
extern wmfRGB wmf_red;
extern wmfRGB wmf_rosybrown;
extern wmfRGB wmf_royalblue;
extern wmfRGB wmf_saddlebrown;
extern wmfRGB wmf_salmon;
extern wmfRGB wmf_sandybrown;
extern wmfRGB wmf_seagreen;
extern wmfRGB wmf_seashell;
extern wmfRGB wmf_sienna;
extern wmfRGB wmf_silver;
extern wmfRGB wmf_skyblue;
extern wmfRGB wmf_slateblue;
extern wmfRGB wmf_slategray;
extern wmfRGB wmf_slategrey;
extern wmfRGB wmf_snow;
extern wmfRGB wmf_springgreen;
extern wmfRGB wmf_steelblue;
extern wmfRGB wmf_tan;
extern wmfRGB wmf_teal;
extern wmfRGB wmf_thistle;
extern wmfRGB wmf_tomato;
extern wmfRGB wmf_turquoise;
extern wmfRGB wmf_violet;
extern wmfRGB wmf_wheat;
extern wmfRGB wmf_white;
extern wmfRGB wmf_whitesmoke;
extern wmfRGB wmf_yellow;
extern wmfRGB wmf_yellowgreen;

#endif /* ! LIBWMF_COLOR_H */
