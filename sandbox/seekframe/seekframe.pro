QT += widgets

CONFIG += \
    sdk_no_version_check

HEADERS += \
    ../../image/image.h \
    ../../tests/createimage.h \
    ../../video/decoder.h \
    ../../video/encoder.h

SOURCES += \
    main.cpp \
    ../../image/image.cpp \
    ../../tests/createimage.cpp \
    ../../video/decoder.cpp \
    ../../video/encoder.cpp

INCLUDEPATH += \
    $$PWD/../..

include(../../external/FFmpeg.pri)
