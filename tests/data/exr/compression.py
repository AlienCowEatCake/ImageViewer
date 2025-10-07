#!/usr/bin/env python3

import os
import imageio.v3
import OpenEXR

scriptdir = os.path.dirname(os.path.realpath(__file__))

filepath = os.path.join(scriptdir, '..', 'tiff', 'Qt', 'Format_RGBA16FPx4_NoCompression.tiff')
for compression in [OpenEXR.ZIP_COMPRESSION, OpenEXR.HTJ2K256_COMPRESSION, OpenEXR.HTJ2K32_COMPRESSION]:
    for type in [OpenEXR.scanlineimage, OpenEXR.tiledimage]:
        for datatype in ['float16', 'float32']:
            RGB = imageio.v3.imread(filepath).astype(datatype)
            channels = {"RGB": RGB}
            header = {"compression": compression, "type": type}
            if type in [OpenEXR.tiledimage, OpenEXR.deeptile]:
                header["tiles"] = OpenEXR.TileDescription()
            compression_str = str(compression)
            compression_str = compression_str[compression_str.find('.') + 1:]
            compression_str = compression_str[0:compression_str.find('_')]
            type_str = str(type)
            type_str = type_str[type_str.find('.') + 1:]
            filename = 'alpha_' + datatype + '_' + compression_str + '_' + type_str + '.exr'
            with OpenEXR.File(header, channels) as outfile:
                outfile.write(os.path.join(scriptdir, filename))

# filepath = os.path.join(scriptdir, '..', 'tiff', 'lena_zip.tif')
# for compression in [OpenEXR.ZIP_COMPRESSION, OpenEXR.HTJ2K256_COMPRESSION, OpenEXR.HTJ2K32_COMPRESSION]:
#     for type in [OpenEXR.scanlineimage, OpenEXR.tiledimage]:
#         for datatype in ['float16', 'float32']:
#             RGB = imageio.v3.imread(filepath).astype(datatype) / 255
#             channels = {"RGB": RGB}
#             header = {"compression": compression, "type": type}
#             if type in [OpenEXR.tiledimage, OpenEXR.deeptile]:
#                 header["tiles"] = OpenEXR.TileDescription()
#             compression_str = str(compression)
#             compression_str = compression_str[compression_str.find('.') + 1:]
#             compression_str = compression_str[0:compression_str.find('_')]
#             type_str = str(type)
#             type_str = type_str[type_str.find('.') + 1:]
#             filename = 'lena_' + datatype + '_' + compression_str + '_' + type_str + '.exr'
#             with OpenEXR.File(header, channels) as outfile:
#                 outfile.write(os.path.join(scriptdir, filename))
