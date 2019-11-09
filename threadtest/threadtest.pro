QT += core gui widgets

TARGET = threadtest
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += \
    c++11 \
    sdk_no_version_check

SOURCES += \
    main.cpp \
    ../image/video.cpp \
    ../image/recorder.cpp \
    ../hotkey/qhotkey.cpp \
    ../hotkey/qhotkey_mac.cpp

HEADERS += \
    ../image/screenshot.h \
    ../image/video.h \
    ../image/recorder.h \
    ../hotkey/qhotkey.h \
    ../hotkey/qhotkey_p.h

DEFINES += TEST_THREADING

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
        -framework ApplicationServices \
        -framework Carbon
}
