CONFIG += sdk_no_version_check

SOURCES += \
    main.c

INCLUDEPATH += \
    /usr/local/include

LIBS += \
    -L/usr/local/lib \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lavdevice \
    -lavfilter \
    -lswscale \
    -lswresample
