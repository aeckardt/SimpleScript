QT += widgets

CONFIG += \
    sdk_no_version_check

SOURCES += \
    main.cpp \
    ../../image/image.cpp \
    ../../tests/createimage.cpp \
    ../../video/decoder.cpp

HEADERS += \
    ../../image/image.h \
    ../../tests/createimage.h \
    ../../video/decoder.h

INCLUDEPATH += \
    $$PWD/../..

include(../../external/FFmpeg.pri)
