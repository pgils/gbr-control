TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(release, debug|release): DEFINES += NDEBUG

SOURCES += \
    gbrdatabasehandler.cpp \
    gbrxml.cpp \
    gbrsocketlistener.cpp \
    gbrsocketlistener_test.cpp

HEADERS += \
    gbrdatabasehandler.h \
    gbrxml.h \
    gbrsocketlistener.h

LIBS +=  \
    -ltinyxml2 \
    -lsqlite3
