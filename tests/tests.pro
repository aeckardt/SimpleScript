include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
CONFIG += sdk_no_version_check

HEADERS += \
    createimage.h \
    ../video/decoder.h \ # for moc creation
    tst_image.h \
    tst_video.h

SOURCES += \
    createimage.cpp \
    main.cpp \
    ../image/image.cpp \
    ../video/decoder.cpp \
    ../video/encoder.cpp \
    ../video/videofile.cpp

INCLUDEPATH += \
    $$PWD/.. \
    /usr/local/include

LIBS += \
    -L/usr/local/lib \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale
