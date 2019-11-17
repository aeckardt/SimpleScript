QT += widgets

CONFIG += sdk_no_version_check

SOURCES += \
    main.cpp \
    VideoView.cpp \
    ../hotkey/qhotkey.cpp \
    ../image/recorder.cpp \
    ../image/video.cpp

HEADERS += \
    VideoView.h \
    ../hotkey/qhotkey.h \
    ../hotkey/qhotkey_p.h \
    ../image/recorder.h \
    ../image/screenshot.h \
    ../image/video.h

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

DEFINES += \
    TEST_THREADING

INCLUDEPATH += \
    ..

RESOURCES += \
    ../resources/resources.qrc
