QT      += core
QT      += gui
#QT      += webkitwidgets
QT      += printsupport
QT      += webenginewidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HTYBrowser
TEMPLATE = app
include (./CTK.pri)
CONFIG(debug, debug|release){
    DESTDIR = ./../../debug
} else {
    DESTDIR = ./../../release
}
SOURCES += main.cpp\
        eventlistener.cpp \
        mainwindow.cpp

HEADERS  += mainwindow.h \
    eventlistener.h \
    pluginservice.h

FORMS    += mainwindow.ui \

RESOURCES += \
    jquery.qrc \
    res.qrc

DISTFILES += \
DEFINES += QT_QML_DEBUG_NO_WARNING

SUBDIRS += \
