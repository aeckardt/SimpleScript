QT += widgets

CONFIG += sdk_no_version_check

SOURCES += \
    VideoView.cpp \
    main.cpp

INCLUDEPATH += \
    /usr/local/include

LIBS += \
    -L/usr/local/lib \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale \
    -framework Cocoa

HEADERS += \
    VideoView.h
