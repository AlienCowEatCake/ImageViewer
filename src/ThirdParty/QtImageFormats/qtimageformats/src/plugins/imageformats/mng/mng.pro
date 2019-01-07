TARGET  = qmng

HEADERS += qmnghandler_p.h
SOURCES += main.cpp \
           qmnghandler.cpp
OTHER_FILES += mng.json

msvc: LIBS += libmng.lib
else: LIBS += -lmng

PLUGIN_TYPE = imageformats
PLUGIN_CLASS_NAME = QMngPlugin
load(qt_plugin)
