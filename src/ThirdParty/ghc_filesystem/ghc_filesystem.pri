# URL: https://github.com/gulrak/filesystem
# License: MIT License - https://github.com/gulrak/filesystem/blob/master/LICENSE

include($${PWD}/../../Features.pri)

!disable_ghc_filesystem {

    DEFINES += HAS_GHC_FILESYSTEM

    THIRDPARTY_GHC_FILESYSTEM_PATH = $${PWD}/include

    INCLUDEPATH += $${THIRDPARTY_GHC_FILESYSTEM_PATH}
    DEPENDPATH += $${THIRDPARTY_GHC_FILESYSTEM_PATH}

}
