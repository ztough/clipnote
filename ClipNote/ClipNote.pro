QT       += core gui network websockets concurrent sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
RC_ICONS = logo.ico
include(QHotkey/qhotkey.pri)
SOURCES += \
    main.cpp \
    mainwindow.cpp
HEADERS += \
    mainwindow.h \
    videocapturethread.h \
    webvideocapturethread.h

FORMS += \
    mainwindow.ui

    # 指定 vcpkg 安装的 OpenCV 库路径
INCLUDEPATH += D:/vcpkg/installed/x64-windows/include
LIBS += -LD:/vcpkg/installed/x64-windows/lib
LIBS += -lopencv_core4
LIBS += -lopencv_imgproc4
LIBS += -lopencv_imgcodecs4
LIBS += -lopencv_videoio4
LIBS += -lopencv_highgui4
LIBS += -lavformat -lavcodec -lavutil -lswscale
CONFIG += lrelease
CONFIG += embed_translations
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc

