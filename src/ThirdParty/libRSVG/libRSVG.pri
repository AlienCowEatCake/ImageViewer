# URL: https://gitlab.gnome.org/GNOME/librsvg
# License: GNU LGPLv2.1 - https://gitlab.gnome.org/GNOME/librsvg/blob/master/COPYING.LIB

include($${PWD}/../../Features.pri)

!disable_librsvg {

    system_librsvg {

        DEFINES += HAS_LIBRSVG

        !disable_pkgconfig {
            PKGCONFIG += librsvg-2.0
        }

    }

}
