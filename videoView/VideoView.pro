QT += widgets

SOURCES += \
    main.cpp \
    VideoView.cpp \
    ../hotkey/qhotkey.cpp \
    ../image/video.cpp

win32 {
    SOURCES += \
        ../image/screenshot_win.cpp \
        ../hotkey/qhotkey_win.cpp

    LIBS += \
        -lgdi32
}
macx {
    SOURCES += \
        ../image/screenshot_mac.cpp \
        ../hotkey/qhotkey_mac.cpp

    LIBS += \
        -framework ApplicationServices \
        -framework Carbon
}

HEADERS += \
    VideoView.h \
    ../hotkey/qhotkey.h \
    ../hotkey/qhotkey_p.h \
    ../image/screenshot.h \
    ../image/video.h

INCLUDEPATH += \
    ..
