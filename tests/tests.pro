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

include(../external/FFmpeg.pri)
