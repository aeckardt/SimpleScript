QT += widgets

CONFIG += sdk_no_version_check

SOURCES += \
    main.cpp \
    ../../image/image.cpp \
    ../../tests/createimage.cpp \
    ../../utils/memoryusage_mac.cpp

HEADERS += \
    ../../image/image.h \
    ../../utils/memoryusage.h

INCLUDEPATH += \
    $$PWD/../..
