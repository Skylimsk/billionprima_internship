QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = opencvtest
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
    control_panel.cpp \
    main.cpp \
    image_label.cpp \
    image_processor.cpp \
    mainwindow.cpp

HEADERS += \
    control_panel.h \
    mainwindow.h \
    image_label.h \
    image_processing_params.h \
    image_processor.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += C:\opencv\build\include

LIBS += C:\opencv_build\bin\libopencv_core4100.dll
LIBS += C:\opencv_build\bin\libopencv_highgui4100.dll
LIBS += C:\opencv_build\bin\libopencv_imgcodecs4100.dll
LIBS += C:\opencv_build\bin\libopencv_imgproc4100.dll
LIBS += C:\opencv_build\bin\libopencv_features2d4100.dll
LIBS += C:\opencv_build\bin\libopencv_calib3d4100.dll

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
