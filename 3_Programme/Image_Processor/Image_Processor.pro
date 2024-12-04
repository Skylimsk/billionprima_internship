QT       += core gui printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# OpenCV configuration for debug
win32 {
    INCLUDEPATH += D:/opencv_build/install/include

    CONFIG(debug, debug|release) {
        # OpenCV libs
        LIBS += -LD:/opencv_build/install/x64/vc17/lib \
                -lopencv_core4100d \
                -lopencv_imgproc4100d \
                -lopencv_highgui4100d \
                -lopencv_imgcodecs4100d \
                -lopencv_cudaarithm4100d \
                -lopencv_cudabgsegm4100d \
                -lopencv_cudacodec4100d \
                -lopencv_cudafeatures2d4100d \
                -lopencv_cudafilters4100d \
                -lopencv_cudaimgproc4100d \
                -lopencv_cudalegacy4100d \
                -lopencv_cudaobjdetect4100d \
                -lopencv_cudaoptflow4100d \
                -lopencv_cudastereo4100d \
                -lopencv_cudawarping4100d

        # 修改 DLL 复制命令
        OPENCV_DLL_PATH = D:/opencv_build/install/x64/vc17/bin

        # 使用 copy 命令替代 powershell
        QMAKE_POST_LINK += \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_core4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_imgproc4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_highgui4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_imgcodecs4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudaarithm4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudabgsegm4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudacodec4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudafeatures2d4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudafilters4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudaimgproc4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudalegacy4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudaobjdetect4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudaoptflow4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudastereo4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t)) \
            $$quote(cmd /c copy /y \"$$OPENCV_DLL_PATH\\opencv_cudawarping4100d.dll\" \"$(DESTDIR)\" $$escape_expand(\n\t))
    }

    CONFIG(release, debug|release) {
        LIBS += -LD:/opencv_build/install/x64/vc17/lib \
                -lopencv_core4100 \
                -lopencv_imgproc4100 \
                -lopencv_highgui4100 \
                -lopencv_imgcodecs4100 \
                -lopencv_cudaarithm4100 \
                -lopencv_cudabgsegm4100 \
                -lopencv_cudacodec4100 \
                -lopencv_cudafeatures2d4100 \
                -lopencv_cudafilters4100 \
                -lopencv_cudaimgproc4100 \
                -lopencv_cudalegacy4100 \
                -lopencv_cudaobjdetect4100 \
                -lopencv_cudaoptflow4100 \
                -lopencv_cudastereo4100 \
                -lopencv_cudawarping4100
    }
}

# QCustomPlot configuration
INCLUDEPATH += $$PWD/third_party/qcustomplot
SOURCES += third_party/qcustomplot/qcustomplot.cpp \
    CGParams.cpp
HEADERS += third_party/qcustomplot/qcustomplot.h

# CUDA Configuration
CUDA_DIR = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6"
INCLUDEPATH += $$CUDA_DIR/include

# You can make this application better by adding large file support:
DEFINES += _FILE_OFFSET_BITS=64

# Source files
SOURCES += \
    CLAHE.cpp \
    ThreadLogger.cpp \
    adjustments.cpp \
    control_panel.cpp \
    dark_line.cpp \
    darkline_pointer.cpp \
    display_window.cpp \
    histogram.cpp \
    image_label.cpp \
    image_processor.cpp \
    interlace.cpp \
    main.cpp \
    mainwindow.cpp \
    pointer_operations.cpp \
    zoom.cpp

HEADERS += \
    CLAHE.h \
    ThreadLogger.h \
    adjustments.h \
    control_panel.h \
    dark_line.h \
    darkline_pointer.h \
    display_window.h \
    histogram.h \
    image_label.h \
    image_processing_params.h \
    image_processor.h \
    interlace.h \
    mainwindow.h \
    pointer_operations.h \
    zoom.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Add optimization flags for better performance
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_LFLAGS_RELEASE += -O3

# Enable link time optimization
QMAKE_LFLAGS += -flto
QMAKE_CXXFLAGS += -flto

# If using MSVC
msvc {
    QMAKE_CXXFLAGS += /GL
    QMAKE_LFLAGS += /LTCG
} else {
    # 其他编译器保持使用 -flto
    QMAKE_LFLAGS += -flto
    QMAKE_CXXFLAGS += -flto
}

# CGProcessImage library configuration
win32 {
    CONFIG(release, debug|release) {
        LIBS += -L$$PWD/lib -lCGProcessImage
        PRE_TARGETDEPS += $$PWD/lib/CGProcessImage.lib
    } else {
        LIBS += -L$$PWD/lib -lCGProcessImage
        PRE_TARGETDEPS += $$PWD/lib/CGProcessImage.lib
    }
}

INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/include
