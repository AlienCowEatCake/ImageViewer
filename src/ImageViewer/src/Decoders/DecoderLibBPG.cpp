/*
   Copyright (C) 2017 Peter S. Zhigalov <peter.zhigalov@gmail.com>

   This file is part of the `ImageViewer' program.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>

extern "C" {
#include <libbpg.h>
}

#include <QGraphicsPixmapItem>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>
#include <QDebug>

#include "IDecoder.h"
#include "Internal/DecoderAutoRegistrator.h"
#include "Internal/Animation/IAnimationProvider.h"
#include "Internal/Animation/AnimationUtils.h"
#include "Internal/Animation/FrameCompositor.h"

namespace {

// ====================================================================================================

//void bpgShowInfo(const char *filename, int show_extensions)
//{
//    uint8_t *buf;
//    int buf_len, ret, buf_len_max;
//    FILE *f;
//    BPGImageInfo p_s, *p = &p_s;
//    BPGExtensionData *first_md, *md;
//    static const char *format_str[6] = {
//        "Gray",
//        "4:2:0",
//        "4:2:2",
//        "4:4:4",
//        "4:2:0_video",
//        "4:2:2_video",
//    };
//    static const char *color_space_str[BPG_CS_COUNT] = {
//        "YCbCr",
//        "RGB",
//        "YCgCo",
//        "YCbCr_BT709",
//        "YCbCr_BT2020",
//    };
//    static const char *extension_tag_str[] = {
//        "Unknown",
//        "EXIF",
//        "ICC profile",
//        "XMP",
//        "Thumbnail",
//        "Animation control",
//    };

//    f = fopen(filename, "rb");
//    if (!f) {
//        fprintf(stderr, "Could not open %s\n", filename);
//        exit(1);
//    }

//    if (show_extensions) {
//        fseek(f, 0, SEEK_END);
//        buf_len_max = ftell(f);
//        fseek(f, 0, SEEK_SET);
//    } else {
//        /* if no extension are shown, just need the header */
//        buf_len_max = BPG_DECODER_INFO_BUF_SIZE;
//    }
//    buf = malloc(buf_len_max);
//    buf_len = fread(buf, 1, buf_len_max, f);

//    ret = bpg_decoder_get_info_from_buf(p, show_extensions ? &first_md : NULL,
//                                        buf, buf_len);
//    free(buf);
//    fclose(f);
//    if (ret < 0) {
//        fprintf(stderr, "Not a BPG image\n");
//        exit(1);
//    }
//    printf("size=%dx%d color_space=%s",
//           p->width, p->height,
//           p->format == BPG_FORMAT_GRAY ? "Gray" : color_space_str[p->color_space]);
//    if (p->has_w_plane) {
//        printf(" w_plane=%d", p->has_w_plane);
//    }
//    if (p->has_alpha) {
//        printf(" alpha=%d premul=%d",
//               p->has_alpha, p->premultiplied_alpha);
//    }
//    printf(" format=%s limited_range=%d bit_depth=%d animation=%d\n",
//           format_str[p->format],
//           p->limited_range,
//           p->bit_depth,
//           p->has_animation);

//    if (first_md) {
//        const char *tag_name;
//        printf("Extension data:\n");
//        for(md = first_md; md != NULL; md = md->next) {
//            if (md->tag <= 5)
//                tag_name = extension_tag_str[md->tag];
//            else
//                tag_name = extension_tag_str[0];
//            printf("  tag=%d (%s) length=%d\n",
//                   md->tag, tag_name, md->buf_len);
//        }
//        bpg_decoder_free_extension_data(first_md);
//    }
//}


// ====================================================================================================

class DecoderLibBpg : public IDecoder
{
public:
    QString name() const
    {
        return QString::fromLatin1("DecoderLibBpg");
    }

    QStringList supportedFormats() const
    {
        return QStringList()
                << QString::fromLatin1("bpg");
    }

    QGraphicsItem *loadImage(const QString &filePath)
    {
        const QFileInfo fileInfo(filePath);
        if(!fileInfo.exists() || !fileInfo.isReadable())
            return NULL;
//        return AnimationUtils::CreateGraphicsItem(new WebPAnimationProvider(filePath));
        return NULL;
    }
};

DecoderAutoRegistrator registrator(new DecoderLibBpg);

// ====================================================================================================

} // namespace
