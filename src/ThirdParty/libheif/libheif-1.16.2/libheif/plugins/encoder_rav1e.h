/*
 * HEIF codec.
 * Copyright (c) 2017 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of libheif.
 *
 * libheif is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * libheif is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with libheif.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBHEIF_ENCODER_RAV1E_H
#define LIBHEIF_ENCODER_RAV1E_H

#include "libheif/common_utils.h"

const struct heif_encoder_plugin* get_encoder_plugin_rav1e();

#if PLUGIN_RAV1E
extern "C" {
MAYBE_UNUSED LIBHEIF_API extern heif_plugin_info plugin_info;
}
#endif

#endif
