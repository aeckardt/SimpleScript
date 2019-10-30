#-------------------------------------------------
#
# Project created by QtCreator 2019-10-20T12:16:17
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "Game Script"
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
    c++11 \
    sdk_no_version_check

SOURCES += \
    image/video.cpp \
    main.cpp \
    mainwindow.cpp \
    image/screenshot.cpp \
    imageView/ImageView.cpp \
    script/engine.cpp \
    script/highlighter.cpp \
    script/objecttypes.cpp \
    script/parser.cpp \
    script/tokenizer.cpp \
    script/treewalker.cpp \
    selectFrame/SelectFrameWidget.cpp

HEADERS += \
    image/video.h \
    mainwindow.h \
    script/highlighter.h \
    script/objecttypes.h \
    script/tokenizer.h \
    ui_mainwindow.h \
    image/screenshot.h \
    imageView/ImageView.h \
    script/engine.h \
    script/parser.h \
    script/treewalker.h \
    selectFrame/SelectFrameWidget.h \
    utils/measuretime.h

INCLUDEPATH += \
    /usr/local/include

LIBS += \
    -framework ApplicationServices

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    "Game Script.qrc"
