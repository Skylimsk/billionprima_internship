QT += core gui widgets printsupport

# Project configuration
TARGET = ImageProcessor
TEMPLATE = app
CONFIG += c++17

# Ensure Qt modules are found - using specific Qt path
QTDIR = C:/Qt/6.8.0/msvc2022_64    # Updated to match your actual Qt version
INCLUDEPATH += C:/Qt/6.8.0/msvc2022_64/include
INCLUDEPATH += C:/Qt/6.8.0/msvc2022_64/include/QtWidgets
INCLUDEPATH += C:/Qt/6.8.0/msvc2022_64/include/QtCore
INCLUDEPATH += C:/Qt/6.8.0/msvc2022_64/include/QtGui

# Add this to ensure all Qt widgets are available
requires(qtConfig(combobox))
requires(qtConfig(label))
requires(qtConfig(messagebox))

# Qt library paths
LIBS += -L$${QTDIR}/lib

# Runtime library configuration - match with CGProcessImage.lib
CONFIG(debug, debug|release) {
    # Debug configuration
    QMAKE_CXXFLAGS_DEBUG += /MDd       # Debug dynamic runtime
    DEFINES += _ITERATOR_DEBUG_LEVEL=2  # Debug iterator level
    DEFINES += _DEBUG                   # Debug mode
} else {
    # Release configuration
    QMAKE_CXXFLAGS_RELEASE += /MD      # Release dynamic runtime
    DEFINES += _ITERATOR_DEBUG_LEVEL=0  # Release iterator level
    DEFINES += NDEBUG                   # Release mode
}

# MSVC specific configuration
msvc {
    # Remove conflicting warning level flags
    QMAKE_CXXFLAGS_WARN_ON -= -W3
    QMAKE_CXXFLAGS_WARN_ON -= -W4

    # Add our desired warning level and disable specific warnings
    QMAKE_CXXFLAGS += /W4                  # Use W4 consistently
    QMAKE_CXXFLAGS += /wd4100             # unreferenced formal parameter
    QMAKE_CXXFLAGS += /wd4189             # local variable is initialized but not referenced
    QMAKE_CXXFLAGS += /wd4456             # declaration hides previous local declaration
    QMAKE_CXXFLAGS += /wd4458             # declaration hides class member
    QMAKE_CXXFLAGS += /wd4996             # deprecated functions and types
    QMAKE_CXXFLAGS += /wd4819             # Character encoding warning
    QMAKE_CXXFLAGS += /wd4251             # DLL interface warning
    QMAKE_CXXFLAGS += /wd4127             # Conditional expression is constant
    QMAKE_CXXFLAGS += /wd4267             # Conversion from size_t to int
    QMAKE_CXXFLAGS += /wd4244             # Conversion from double to float
    QMAKE_CXXFLAGS += /wd4005             # Macro redefinition
    QMAKE_CXXFLAGS += /wd4042             # Object specified more than once
    QMAKE_CXXFLAGS += /wd4101             # unreferenced local variable

    # STL specific warnings suppression
    DEFINES += _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
    DEFINES += _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS

    # Optimization and other flags
    QMAKE_CXXFLAGS += /MP                 # Multi-processor compilation
    QMAKE_CXXFLAGS += /permissive-        # Standards conformance
    QMAKE_CXXFLAGS += /Zc:wchar_t         # wchar_t is native type

    # Release configuration
    QMAKE_CXXFLAGS_RELEASE += /O2         # Maximum optimization
    QMAKE_CXXFLAGS_RELEASE += /Oi         # Enable intrinsic functions
    QMAKE_CXXFLAGS_RELEASE += /GL         # Whole program optimization
    QMAKE_LFLAGS_RELEASE += /LTCG         # Link-time code generation

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

# CGProcessImage configuration
CONFIG(debug, debug|release) {
    LIBS += -L"D:/billionprima_internship/3_Programme/OpenCV_MSVC/lib" -lCGProcessImaged
} else {
    LIBS += -L"D:/billionprima_internship/3_Programme/OpenCV_MSVC/lib" -lCGProcessImage
}

# Sources and Headers
SOURCES = \
    ThreadLogger.cpp \
    adjustments.cpp \
    dark_line.cpp \
    darkline_pointer.cpp \
    display_window.cpp \
    interlace.cpp \
    main.cpp \
    control_panel.cpp \
    image_label.cpp \
    image_processor.cpp \
    histogram.cpp \
    CLAHE.cpp \
    pointer_operations.cpp \
    third_party/qcustomplot/qcustomplot.cpp \
    zoom.cpp

HEADERS = \
    Control_Panel/control_panel_utils.h \
    ThreadLogger.h \
    adjustments.h \
    control_panel.h \
    dark_line.h \
    darkline_pointer.h \
    display_window.h \
    image_label.h \
    image_processing_params.h \
    image_processor.h \
    histogram.h \
    CLAHE.h \
    interlace.h \
    pointer_operations.h \
    third_party/qcustomplot/qcustomplot.h \
    zoom.h

INCLUDEPATH += "D:/billionprima_internship/3_Programme/OpenCV_MSVC/include"
DEPENDPATH += "D:/billionprima_internship/3_Programme/OpenCV_MSVC/include"

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
