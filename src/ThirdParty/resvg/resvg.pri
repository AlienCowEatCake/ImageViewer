# URL: https://github.com/RazrFalcon/resvg
# License: Mozilla Public License 2.0 - https://github.com/RazrFalcon/resvg/blob/master/LICENSE.txt

include($${PWD}/../../Features.pri)

!disable_resvg {

    system_resvg {

        DEFINES += HAS_RESVG

        *msvc*: LIBS += resvg.lib
        else: LIBS += -lresvg

    }

}
