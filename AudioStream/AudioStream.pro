#-------------------------------------------------
#
# Project created by QtCreator 2019-08-27T18:20:13
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = AudioStream
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        CAudioUtil.cpp \
        main.cpp \
        mainwidget.cpp

HEADERS += \
        CAudioUtil.h \
        mainwidget.h

FORMS += \
        mainwidget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# 附加包含路径
INCLUDEPATH += \
        $$PWD/pjproject-2.9/include/pjlib/include \
        $$PWD/pjproject-2.9/include/pjlib-util/include \
        $$PWD/pjproject-2.9/include/pjmedia/include \
        $$PWD/pjproject-2.9/include/pjnath/include \
        $$PWD/pjproject-2.9/include/pjsip/include

# ***********************************************************
# Win平台下配置
# ***********************************************************
win32 {
    contains(QT_ARCH, i386) {
        message("x86")

        # 依赖模块
        CONFIG(debug, debug|release) {
            LIBS += \
                    -L$$PWD/pjproject-2.9/lib/win libpjproject-i386-Win32-vc14-Debug.lib
        } else {
            LIBS += \
                    -L$$PWD/pjproject-2.9/lib/win libpjproject-i386-Win32-vc14-Release.lib
        }
        LIBS += ws2_32.lib Ole32.lib
    }
}

# ***********************************************************
# Mac平台下配置
# ***********************************************************
macos {
    LIBS += \
            -L$$PWD/pjproject-2.9/lib/mac -lpj-x86_64-apple-darwin18.2.0 \
            -L$$PWD/pjproject-2.9/lib/mac -lpjmedia-audiodev-x86_64-apple-darwin18.2.0 \
            -L$$PWD/pjproject-2.9/lib/mac -lpjmedia-x86_64-apple-darwin18.2.0 \
            -L$$PWD/pjproject-2.9/lib/mac -lwebrtc-x86_64-apple-darwin18.2.0 \
            -L$$PWD/pjproject-2.9/lib/mac -lpjlib-util-x86_64-apple-darwin18.2.0 \
            -L$$PWD/pjproject-2.9/lib/mac -lpjmedia-codec-x86_64-apple-darwin18.2.0 \
            -L$$PWD/pjproject-2.9/lib/mac -lspeex-x86_64-apple-darwin18.2.0
    LIBS += \
            -framework AudioToolbox \
            -framework CoreAudio
}
