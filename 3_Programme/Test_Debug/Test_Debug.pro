QT       += core gui widgets

CONFIG += c++17
CONFIG += debug

DEFINES += QT_DEPRECATED_WARNINGS

INCLUDEPATH += D:/opencv_build/install/include

LIBS += -LD:/opencv_build/install/x64/vc17/lib \
    -lopencv_core4100d \
    -lopencv_imgproc4100d \
    -lopencv_highgui4100d \
    -lopencv_imgcodecs4100d \
    -lopencv_videoio4100d \
    -lopencv_features2d4100d \
    -lopencv_calib3d4100d \
    -lopencv_objdetect4100d \
    -lopencv_dnn4100d \
    -lopencv_video4100d \
    -lopencv_cudaimgproc4100d \
    -lopencv_cudafilters4100d

SOURCES += \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    mainwindow.h

FORMS += \
    mainwindow.ui
