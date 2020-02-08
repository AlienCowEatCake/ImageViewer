# URL: https://github.com/ImageMagick/ImageMagick
# License: ImageMagick License - https://imagemagick.org/script/license.php

include($${PWD}/../../Features.pri)

!disable_magickcore {

    DEFINES += HAS_MAGICKCORE

    !disable_pkgconfig {
        PKGCONFIG += MagickCore
    }

}
