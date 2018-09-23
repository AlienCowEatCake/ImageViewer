/*
 * libheif thumbnailer for Gnome desktop.
 * Copyright (c) 2018 struktur AG, Dirk Farin <farin@struktur.de>
 *
 * This file is part of convert, an example application using libheif.
 *
 * convert is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * convert is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with convert.  If not, see <http://www.gnu.org/licenses/>.
 */
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "string.h"

#if defined(HAVE_UNISTD_H)
#include <unistd.h>
#endif
#include <fstream>
#include <iostream>
#include <sstream>
#include <assert.h>

#include <libheif/heif.h>

#include "encoder.h"
#if HAVE_LIBPNG
#include "encoder_png.h"
#endif

#if defined(_MSC_VER)
#include "getopt.h"
#endif


static int usage(const char* command) {
  fprintf(stderr, "usage: %s [-s size] <filename> <output>\n", command);
  return 1;
}


int main(int argc, char** argv)
{
  int opt;
  int size = 512; // default thumbnail size

  while ((opt = getopt(argc, argv, "s:h")) != -1) {
    switch (opt) {
    case 's':
      size = atoi(optarg);
      break;
    case 'h':
    default:
      return usage(argv[0]);
    }
  }

  if (optind + 2 > argc) {
    // Need input and output filenames as additional arguments.
    return usage(argv[0]);
  }

  std::string input_filename(argv[optind++]);
  std::string output_filename(argv[optind++]);


  // --- read heif file

  std::shared_ptr<heif_context> context(heif_context_alloc(),
                                        [] (heif_context* c) { heif_context_free(c); });

  struct heif_error err;
  err = heif_context_read_from_file(context.get(), input_filename.c_str(), nullptr);
  if (err.code != 0) {
    std::cerr << "Could not read HEIF file: " << err.message << "\n";
    return 1;
  }



  // --- get primary image

  struct heif_image_handle* image_handle = NULL;
  err = heif_context_get_primary_image_handle(context.get(), &image_handle);
  if (err.code) {
    std::cerr << "Could not read HEIF image : " << err.message << "\n";
    return 1;
  }


  // --- if image has a thumbnail, use that instead

  heif_item_id thumbnail_ID;
  int nThumbnails = heif_image_handle_get_list_of_thumbnail_IDs(image_handle, &thumbnail_ID, 1);
  if (nThumbnails > 0) {
    struct heif_image_handle* thumbnail_handle;
    err = heif_image_handle_get_thumbnail(image_handle, thumbnail_ID, &thumbnail_handle);
    if (err.code) {
      std::cerr << "Could not read HEIF image : " << err.message << "\n";
      return 1;
    }

    // replace image handle with thumbnail handle

    heif_image_handle_release(image_handle);
    image_handle = thumbnail_handle;
  }



  // --- decode the image (or its thumbnail)

  std::unique_ptr<Encoder> encoder(new PngEncoder());

  struct heif_image* image = NULL;
  err = heif_decode_image(image_handle,
                          &image,
                          encoder->colorspace(false),
                          encoder->chroma(false),
                          NULL);
  if (err.code) {
    std::cerr << "Could not decode HEIF image : " << err.message << "\n";
    return 1;
  }

  assert(image);

  // --- compute output thumbnail size

  int input_width  = heif_image_handle_get_width(image_handle);
  int input_height = heif_image_handle_get_height(image_handle);

  int thumbnail_width = input_width;
  int thumbnail_height = input_height;

  if (input_width>size || input_height>size) {
    if (input_width>input_height) {
      thumbnail_height = input_height * size/input_width;
      thumbnail_width  = size;
    }
    else {
      thumbnail_width  = input_width * size/input_height;
      thumbnail_height = size;
    }


    // --- output thumbnail smaller than HEIF thumbnail -> scale down

    struct heif_image* scaled_image = NULL;
    err = heif_image_scale_image(image, &scaled_image,
                                 thumbnail_width, thumbnail_height,
                                 NULL);
    if (err.code) {
      std::cerr << "Could not scale image : " << err.message << "\n";
      return 1;
    }

    heif_image_release(image);
    image = scaled_image;
  }



  // --- write thumbnail image to PNG

  bool written = encoder->Encode(image_handle, image, output_filename.c_str());
  if (!written) {
    fprintf(stderr,"could not write image\n");
    return 1;
  }

  heif_image_release(image);

  heif_image_handle_release(image_handle);

  return 0;
}
