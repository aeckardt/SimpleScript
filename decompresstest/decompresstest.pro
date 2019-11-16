QT += core gui widgets

CONFIG += sdk_no_version_check

SOURCES += \
    main.cpp \
    ../image/video.cpp

HEADERS += \
    ../image/screenshot.h \
    ../image/video.h

INCLUDEPATH += \
    ..

win32 {
    SOURCES += \
        ../image/screenshot_win.cpp

    LIBS += \
        -lgdi32
}
macx {
    SOURCES += \
        ../image/screenshot_mac.cpp

    LIBS += \
        -framework ApplicationServices
}
