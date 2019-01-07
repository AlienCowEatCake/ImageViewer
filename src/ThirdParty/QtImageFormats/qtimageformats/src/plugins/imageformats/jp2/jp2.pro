TARGET  = qjp2

HEADERS += qjp2handler_p.h
SOURCES += main.cpp \
           qjp2handler.cpp
OTHER_FILES += jp2.json

msvc: LIBS += libjasper.lib
else: LIBS += -ljasper

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QJp2Plugin
load(qt_plugin)
