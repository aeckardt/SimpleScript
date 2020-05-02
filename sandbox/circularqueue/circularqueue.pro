QT += widgets

CONFIG += \
    sdk_no_version_check

SOURCES += \
    main.cpp \
    ../../image/image.cpp \
    ../../tests/createimage.cpp

HEADERS += \
    ../../image/image.h \
    ../../utils/circularqueue.h \
    ../../utils/memoryusage.h

INCLUDEPATH += \
    $$PWD/../..

win32 {
    SOURCES += \
        ../../utils/memoryusage_win.cpp

    LIBS += \
        -lpsapi
}

macx {
    SOURCES += \
        ../../utils/memoryusage_mac.cpp
}
