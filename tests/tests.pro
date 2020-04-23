TEMPLATE = app
CONFIG += \
    c++17 \
    console \
    thread \
    sdk_no_version_check

CONFIG -= app_bundle

HEADERS += \
    createimage.h \
    ../image/image.h \ # for moc creation
    ../video/decoder.h \ # for moc creation
    ../video/encoder.h \ # for moc creation
    test_image.h \
    test_script.h \
    test_video.h

SOURCES += \
    createimage.cpp \
    main.cpp \
    ../script/astwalker.cpp \
    ../script/lexer.cpp \
    ../script/parameter.cpp \
    ../script/parser.cpp \
    ../image/image.cpp \
    ../video/decoder.cpp \
    ../video/encoder.cpp

win32 {
    SOURCES += \
        ../image/image_win.cpp

    LIBS += \
        -lgdi32
}

macx {
    SOURCES += \
        ../image/image_mac.cpp

    LIBS += \
        -framework ApplicationServices
}

INCLUDEPATH += \
    $$PWD/..

include(../external/googletest.pri)
include(../external/FFmpeg.pri)
