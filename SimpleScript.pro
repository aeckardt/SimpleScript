#-------------------------------------------------
#
# Project created by QtCreator 2019-10-20T12:16:17
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "Simple Script"
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += \
    c++17 \
    sdk_no_version_check

SOURCES += \
    mainWindow/main.cpp \
    mainWindow/mainwindow.cpp \
    script/engine.cpp \
    script/highlighter.cpp \
    script/parameter.cpp \
    script/parser.cpp \
    script/lexer.cpp \
    script/astwalker.cpp \
    frameSelector/selectframewidget.cpp \
    image/image.cpp \
    image/imageviewer.cpp \
    video/decoder.cpp \
    video/encoder.cpp \
    video/player.cpp \
    video/recorder.cpp

HEADERS += \
    mainWindow/mainwindow.h \
    mainWindow/ui_mainwindow.h \
    script/engine.h \
    script/highlighter.h \
    script/parser.h \
    script/lexer.h \
    script/astwalker.h \
    script/parameter.h \
    script/types.h \
    frameSelector/selectframewidget.h \
    image/image.h \
    image/imageviewer.h \
    utils/circularqueue.hpp \
    utils/memoryusage.h \
    video/decoder.h \
    video/encoder.h \
    video/player.h \
    video/recorder.h \
    video/videofile.h

win32 {
    SOURCES += \
        image/image_win.cpp \
        utils/memoryusage_win.cpp

    LIBS += \
        -lgdi32
}

macx {
    SOURCES += \
        image/image_mac.cpp \
        utils/memoryusage_mac.cpp

    LIBS += \
        -framework ApplicationServices
}

include(external/QHotkey/qhotkey.pri)
include(external/FFmpeg.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    "resources/resources.qrc"
