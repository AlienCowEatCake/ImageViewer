# URL: https://github.com/linebender/resvg
# License: Apache License 2.0 or MIT License - https://github.com/linebender/resvg/blob/main/README.md#license

include($${PWD}/../../Features.pri)

!disable_resvg {

    system_resvg {

        DEFINES += HAS_RESVG

        *msvc*: LIBS += resvg.lib
        else: LIBS += -lresvg

    }

}
