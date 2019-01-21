TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(release, debug|release): DEFINES += NDEBUG

SOURCES += \
    src/gbrcontrol.cpp \
    src/gbrdatabasehandler.cpp \
    src/gbrsocketlistener.cpp \
    src/gbrxml.cpp

HEADERS += \
    include/gbrcontrol.h \
    include/gbrdatabasehandler.h \
    include/gbrsocketlistener.h \
    include/gbrxml.h

INCLUDEPATH += include

LIBS +=  \
    -ltinyxml2 \
    -lsqlite3  \
    -lpthread
