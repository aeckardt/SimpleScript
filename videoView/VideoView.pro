QT += widgets

SOURCES += \
    main.cpp \
    VideoView.cpp \
    ../hotkey/qhotkey.cpp \
    ../image/video.cpp

win {
    SOURCES += \
        ../image/screenshot_win.cpp

    LIBS += \
        -lgdi32
}
macx {
    SOURCES += \
        ../hotkey/qhotkey_mac.cpp \
        ../image/screenshot_mac.cpp

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
