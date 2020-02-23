# URL: https://github.com/ImageMagick/ImageMagick
# License: ImageMagick License - https://imagemagick.org/script/license.php

include($${PWD}/../../Features.pri)

!disable_magickwand {

    system_magickwand {

        DEFINES += HAS_MAGICKWAND

        !disable_pkgconfig {
            PKGCONFIG += MagickWand
        }

    }

}
