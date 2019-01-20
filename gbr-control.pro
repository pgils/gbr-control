TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(release, debug|release): DEFINES += NDEBUG

SOURCES += \
    gbrdatabasehandler.cpp \
    gbrxml.cpp \
    gbrsocketlistener.cpp \
    gbrcontrol.cpp

HEADERS += \
    gbrdatabasehandler.h \
    gbrxml.h \
    gbrsocketlistener.h \
    gbrcontrol.h

LIBS +=  \
    -ltinyxml2 \
    -lsqlite3  \
    -lpthread

DISTFILES += \
    gbr-control_create.sql
