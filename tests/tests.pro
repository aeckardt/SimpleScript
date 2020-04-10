include(gtest_dependency.pri)

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += thread
CONFIG -= qt
CONFIG += sdk_no_version_check

HEADERS += \
    tst_imageclass.h

SOURCES += \
    main.cpp
