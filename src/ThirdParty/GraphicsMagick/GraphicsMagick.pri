# URL: http://www.graphicsmagick.org/
# License: GraphicsMagick License - https://sourceforge.net/p/graphicsmagick/code/ci/default/tree/Copyright.txt

include($${PWD}/../../Features.pri)

!disable_graphicsmagick {

    DEFINES += HAS_GRAPHICSMAGICK

    !disable_pkgconfig {
        PKGCONFIG += GraphicsMagick
    }

}
