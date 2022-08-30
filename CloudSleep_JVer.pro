QT       += core gui network

win32: CONFIG(release, debug|release): QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += openglwidgets

CONFIG += c++17

RC_ICONS += CloudSleep_JVer.ico
RC_LANG = 0x0004
VERSION = 0.1.0
QMAKE_TARGET_DESCRIPTION = "CloudSleep_JVer"
QMAKE_TARGET_COPYRIGHT = "jkjkil4"

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Mainpage/mainpage.cpp \
    Mainpage/roledisplay.cpp \
    SleepRoom/overlay.cpp \
    SleepRoom/sleeproom.cpp \
    main.cpp \
    widget.cpp

HEADERS += \
    Mainpage/mainpage.h \
    Mainpage/roledisplay.h \
    SleepRoom/overlay.h \
    SleepRoom/sleeproom.h \
    widget.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    font.qrc \
    src.qrc

FORMS += \
    Mainpage/mainpage.ui \
    SleepRoom/overlay.ui

DISTFILES += \
    android_sources/AndroidManifest.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android_sources
