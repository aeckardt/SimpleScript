FFMPEG_DIR = $$PWD/FFmpeg

win32 {
    INCLUDEPATH += \
        $$FFMPEG_DIR \
        $$FFMPEG_DIR/include

    LIBS += \
        -L$$FFMPEG_DIR/lib
}

macx {
    INCLUDEPATH += \
        /usr/local/include

    LIBS += \
        -L/usr/local/lib
}

LIBS += \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale
