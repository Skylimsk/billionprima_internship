QT       += core gui printsupport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
CONFIG += qt

# Path configurations
OPENCV_DIR = D:/opencv_build/install
OPENCV_BIN_PATH = $$OPENCV_DIR/x64/vc17/bin
TBB_DIR = "C:/Program Files (x86)/Intel/oneAPI/tbb/latest"
CUDA_DIR = "C:/Program Files/NVIDIA GPU Computing Toolkit/CUDA/v12.6"

# Include paths
INCLUDEPATH += $$OPENCV_DIR/include
INCLUDEPATH += $$TBB_DIR/include
INCLUDEPATH += $$CUDA_DIR/include
INCLUDEPATH += $$PWD/third_party/qcustomplot
INCLUDEPATH += $$PWD/include

# Environment setup
QMAKE_EXTRA_TARGETS += first
first.target = first
first.commands = set PATH=$$OPENCV_BIN_PATH;$$TBB_DIR/redist/intel64/vc14;$(PATH)

# OpenCV configuration for debug and release
win32 {
    CONFIG(debug, debug|release) {
        # OpenCV libs for debug
        LIBS += -L$$OPENCV_DIR/x64/vc17/lib \
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

        # DLL Copy Commands
        OPENCV_DLL_PATH = $$OPENCV_DIR/x64/vc17/bin
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
        # OpenCV libs for release
        LIBS += -L$$OPENCV_DIR/x64/vc17/lib \
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

        # TBB lib for release
        LIBS += -L$$TBB_DIR/lib/intel64/vc14 -ltbb12
    }
}

# QCustomPlot configuration
SOURCES += third_party/qcustomplot/qcustomplot.cpp \
    moc_image_processor.cpp
HEADERS += third_party/qcustomplot/qcustomplot.h

# Source files
SOURCES += \
    CLAHE.cpp \
    ThreadLogger.cpp \
    adjustments.cpp \
    control_panel.cpp \
    darkline_pointer.cpp \
    display_window.cpp \
    histogram.cpp \
    image_label.cpp \
    image_processor.cpp \
    interlace.cpp \
    main.cpp \
    mainwindow.cpp \
    pointer_operations.cpp \
    zoom.cpp \
    CGParams.cpp

HEADERS += \
    CLAHE.h \
    ThreadLogger.h \
    adjustments.h \
    control_panel.h \
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

FORMS += mainwindow.ui

# Compiler and linker flags
DEFINES += _FILE_OFFSET_BITS=64

# Optimization flags
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
QMAKE_LFLAGS_RELEASE += -O3

# Enable OpenMP support
msvc {
    QMAKE_CXXFLAGS += /GL /openmp
    QMAKE_LFLAGS += /LTCG
} else {
    QMAKE_CXXFLAGS += -flto -fopenmp
    QMAKE_LFLAGS += -flto
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

DEPENDPATH += $$PWD/include

# Deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
