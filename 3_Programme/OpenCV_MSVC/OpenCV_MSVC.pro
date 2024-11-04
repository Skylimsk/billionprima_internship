QT       += core gui printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# Project configuration
TARGET = ImageProcessor
TEMPLATE = app
CONFIG += c++17

# MSVC specific configuration
msvc {
    # Remove conflicting warning level flags
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON -= -W4

    QMAKE_CXXFLAGS += /wd4100  # unreferenced formal parameter
    QMAKE_CXXFLAGS += /wd4702  # unreachable code
    QMAKE_CXXFLAGS += /wd4389  # signed/unsigned mismatch comparison

    # Add our desired warning level and disable specific warnings
    QMAKE_CXXFLAGS += /W4         # Use W4 consistently
    QMAKE_CXXFLAGS += /wd4819     # Character encoding warning
    QMAKE_CXXFLAGS += /wd4251     # DLL interface warning
    QMAKE_CXXFLAGS += /wd4127     # Conditional expression is constant
    QMAKE_CXXFLAGS += /wd4100     # Unreferenced formal parameter
    QMAKE_CXXFLAGS += /wd4458     # Declaration hides class member
    QMAKE_CXXFLAGS += /wd4267     # Conversion from size_t to int
    QMAKE_CXXFLAGS += /wd4244     # Conversion from double to float
    QMAKE_CXXFLAGS += /wd4005     # Macro redefinition
    QMAKE_CXXFLAGS += /wd4042     # Object specified more than once

    # Optimization and other flags
    QMAKE_CXXFLAGS += /MP
    QMAKE_CXXFLAGS += /permissive-
    QMAKE_CXXFLAGS += /Zc:wchar_t

    QMAKE_CXXFLAGS_RELEASE += /O2
    QMAKE_CXXFLAGS_RELEASE += /Oi
    QMAKE_CXXFLAGS_RELEASE += /GL
    QMAKE_LFLAGS_RELEASE += /LTCG

    # Definitions
    DEFINES += _CRT_SECURE_NO_WARNINGS
    DEFINES += NOMINMAX
    DEFINES += WIN32_LEAN_AND_MEAN
}

# OpenCV configuration
OPENCV_DIR = C:/opencv-build/install
INCLUDEPATH += $${OPENCV_DIR}/include

CONFIG(debug, debug|release) {
    LIBS += -L$${OPENCV_DIR}/x64/vc17/lib \
            -lopencv_core4100d \
            -lopencv_imgproc4100d \
            -lopencv_highgui4100d \
            -lopencv_cudaimgproc4100d \
            -lopencv_cudaarithm4100d \
            -lopencv_dnn4100d \
            -lopencv_imgcodecs4100d \
            -lopencv_cudacodec4100d \
            -lopencv_videoio4100d \
            -lopencv_cudawarping4100d \
            -lopencv_cudafilters4100d
} else {
    LIBS += -L$${OPENCV_DIR}/x64/vc17/lib \
            -lopencv_core4100 \
            -lopencv_imgproc4100 \
            -lopencv_highgui4100 \
            -lopencv_cudaimgproc4100 \
            -lopencv_cudaarithm4100 \
            -lopencv_dnn4100 \
            -lopencv_imgcodecs4100 \
            -lopencv_cudacodec4100 \
            -lopencv_videoio4100 \
            -lopencv_cudawarping4100 \
            -lopencv_cudafilters4100
}

# QCustomPlot configuration
INCLUDEPATH += $$PWD/third_party/qcustomplot

# Remove duplicate entries
SOURCES = \
    ThreadLogger.cpp \
    adjustments.cpp \
    dark_line.cpp \
    main.cpp \
    control_panel.cpp \
    image_label.cpp \
    image_processor.cpp \
    histogram.cpp \
    CLAHE.cpp \
    third_party/qcustomplot/qcustomplot.cpp

HEADERS = \
    ThreadLogger.h \
    adjustments.h \
    control_panel.h \
    dark_line.h \
    image_label.h \
    image_processing_params.h \
    image_processor.h \
    histogram.h \
    CLAHE.h \
    third_party/qcustomplot/qcustomplot.h

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
