include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
CONFIG += sdk_no_version_check

HEADERS += \
    createimage.h \
    ../video/decoder.h \ # for moc creation
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
    FFMPEG_PATH = $$PWD/../external/FFmpeg

    SOURCES += \
        ../image/image_win.cpp

    INCLUDEPATH += \
        $$FFMPEG_PATH/include

    LIBS += \
        -lgdi32 \
        -L$$FFMPEG_PATH/lib
}

macx {
    SOURCES += \
        ../image/image_mac.cpp

    INCLUDEPATH += \
        /usr/local/include

    LIBS += \
        -framework ApplicationServices \
        -L/usr/local/lib
}

INCLUDEPATH += \
    $$PWD/..

LIBS += \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale
