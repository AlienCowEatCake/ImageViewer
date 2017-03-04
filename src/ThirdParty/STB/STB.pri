# https://github.com/nothings/stb

THIRDPARTY_STB_PATH = $${PWD}/stb

CONFIG += has_thirdparty_qtextended
INCLUDEPATH += $${THIRDPARTY_STB_PATH}
DEPENDPATH += $${THIRDPARTY_STB_PATH}
DEFINES += HAS_THIRDPARTY_STB

HEADERS += \
    $$files($${THIRDPARTY_STB_PATH}/*.h)

THIRDPARTY_STB_SOURCES += \
    $$files($${THIRDPARTY_STB_PATH}/*.c)

*g++*|*clang*|*msvc* {
    thirdparty_stb_compiler.name = thirdparty_stb_compiler
    thirdparty_stb_compiler.input = THIRDPARTY_STB_SOURCES
    thirdparty_stb_compiler.dependency_type = TYPE_C
    thirdparty_stb_compiler.variable_out = OBJECTS
    thirdparty_stb_compiler.output = ${QMAKE_VAR_OBJECTS_DIR}${QMAKE_FILE_BASE}$${first(QMAKE_EXT_OBJ)}
    thirdparty_stb_compiler.commands = $${QMAKE_CC} $(CFLAGS)
    *g++*|*clang* {
        thirdparty_stb_compiler.commands += -Wno-unused-parameter
        thirdparty_stb_compiler.commands += $(INCPATH) -c ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}
    }
    *msvc* {
        thirdparty_stb_compiler.commands += -w44100
        thirdparty_stb_compiler.commands += $(INCPATH) -c ${QMAKE_FILE_IN} -Fo${QMAKE_FILE_OUT}
    }
    QMAKE_EXTRA_COMPILERS += thirdparty_stb_compiler
} else {
    SOURCES += $${THIRDPARTY_STB_SOURCES}
}

TR_EXCLUDE += $${THIRDPARTY_STB_PATH}/*

