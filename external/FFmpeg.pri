win32 {
    FFMPEG_PATH = $$PWD/FFmpeg

    INCLUDEPATH += \
        $$FFMPEG_PATH/include

    LIBS += \
        -L$$FFMPEG_PATH/lib
}

macx {
    INCLUDEPATH += \
        /usr/local/include

    LIBS += \
        -L/usr/local/lib
}

INCLUDEPATH += \
    $$PWD/FFmpeg

LIBS += \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale
