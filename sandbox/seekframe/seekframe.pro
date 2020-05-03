QT += widgets

CONFIG += \
    sdk_no_version_check

SOURCES += \
    main.cpp

INCLUDEPATH += \
    $$PWD/../..

include(../../external/FFmpeg.pri)
