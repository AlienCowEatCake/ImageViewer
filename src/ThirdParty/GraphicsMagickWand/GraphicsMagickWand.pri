# URL: http://www.graphicsmagick.org/
# License: GraphicsMagick License - https://sourceforge.net/p/graphicsmagick/code/ci/default/tree/Copyright.txt

include($${PWD}/../../Features.pri)

!disable_graphicsmagickwand {

    system_graphicsmagickwand {

        DEFINES += HAS_GRAPHICSMAGICKWAND

        !disable_pkgconfig {
            PKGCONFIG += GraphicsMagickWand
        }

    }

}
