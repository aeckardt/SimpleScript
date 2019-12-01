CONFIG += sdk_no_version_check

SOURCES += \
    main.cpp

INCLUDEPATH += \
    /usr/local/include

LIBS += \
    -L/usr/local/lib \
    -lavcodec \
    -lavformat \
    -lavutil \
    -lswscale \
