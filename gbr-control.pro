TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

CONFIG(release, debug|release): DEFINES += NDEBUG

SOURCES += \
        main.cpp \
    gbrdatabasehandler.cpp \
    gbrxml.cpp

HEADERS += \
    gbrdatabasehandler.h \
    gbrxml.h

LIBS +=  \
    -ltinyxml2 \
    -lsqlite3
