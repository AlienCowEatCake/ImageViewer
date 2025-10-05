#!/usr/bin/env python3

import os
import imageio.v3
import OpenEXR

scriptdir = os.path.dirname(os.path.realpath(__file__))
filepath = os.path.join(scriptdir, '..', 'tiff', 'Qt', 'Format_RGBA16FPx4_NoCompression.tiff')
RGB = imageio.v3.imread(filepath)

for compression in [OpenEXR.ZIP_COMPRESSION, OpenEXR.HTJ2K256_COMPRESSION, OpenEXR.HTJ2K32_COMPRESSION]:
    channels = { "RGB" : RGB }
    header = { "compression" : compression, "type" : OpenEXR.scanlineimage }
    filename = str(compression)
    filename = filename[filename.find('.') + 1:].replace('.', '_').lower() + '.exr'
    with OpenEXR.File(header, channels) as outfile:
        outfile.write(os.path.join(scriptdir, filename))
