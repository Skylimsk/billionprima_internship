#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QFontMetrics>
#include <functional>
#include <variant>
#include <vector>
#include <memory>
#include <algorithm>
#include <cmath>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "image_processor.h"
#include "image_label.h"
#include "histogram.h"
#include "zoom.h"
#include "darkline_pointer.h"

// Forward declarations
struct DarkLinePtr;
struct DarkLinePtrArray;

class ControlPanel : public QWidget {
    Q_OBJECT

signals:
    void fileLoaded(bool loaded);

public:
    ControlPanel(ImageProcessor& imageProcessor, ImageLabel* imageLabel, QWidget* parent = nullptr);
    void updatePixelInfo(const QPoint& pos);
    void updateLastAction(const QString& action, const QString& parameters = QString());

protected:
    // Helper struct for line visualization
    struct LineVisualProperties {
        QColor color;
        float penWidth;
        QRect boundingRect;
        QString label;
        QPointF labelPosition;
    };

private:
    // Type definitions and structures
    struct DarkLineImageData {
        double** data;
        int rows;
        int cols;

        DarkLineImageData() : data(nullptr), rows(0), cols(0) {}

        ~DarkLineImageData() {
            if (data) {
                for (int i = 0; i < rows; ++i) {
                    delete[] data[i];
                }
                delete[] data;
            }
        }
    };

    struct DarkLinePtr {
        int x;
        int y;
        int width;
        bool isVertical;
        bool inObject;
        int startX;
        int startY;
        int endX;
        int endY;
    };

    struct DarkLinePtrArray {
        DarkLinePtr* lines;
        size_t count;
        size_t capacity;
    };

    // Setup functions
    void setupPixelInfoLabel();
    void setupFileOperations();
    void setupPreProcessingOperations();
    void setupBasicOperations();
    void setupFilteringOperations();
    void setupAdvancedOperations();
    void setupGlobalAdjustments();
    void setupRegionalAdjustments();
    void setupCLAHEOperations();
    void setupBlackLineDetection();
    void setupPointerProcessing();
    void setupZoomControls();
    void handleRevert();
    void enableButtons(bool enable);

    // Conversion functions
    ImageData convertToImageData(const std::vector<std::vector<uint16_t>>& image);
    std::vector<std::vector<uint16_t>> convertFromImageData(const ImageData& imageData);

    // Drawing helper functions
    LineVisualProperties calculateLineProperties(const ImageProcessor::DarkLine& line,
                                                 size_t index,
                                                 float zoomLevel,
                                                 const QSize& imageSize);
    void drawLineWithLabel(QPainter& painter,
                           const LineVisualProperties& props,
                           float zoomLevel);
    void validateCoordinates(const ImageProcessor::DarkLine& line,
                             int width,
                             int height);
    void drawLineLabel(QPainter& painter,
                       const QString& text,
                       const QPointF& pos,
                       const ZoomManager& zoomManager);

    // Zoom-related methods
    void toggleZoomMode(bool active);
    bool checkZoomMode();
    QSize calculateZoomedSize(const QSize& originalSize, float zoomLevel) const;

    // Memory management helpers
    template<typename T>
    std::unique_ptr<T[]> createUniqueArray(size_t size);
    void cleanupImageData(DarkLineImageData& imageData);

    // UI helper functions
    std::pair<double, bool> showInputDialog(const QString& title,
                                            const QString& label,
                                            double defaultValue,
                                            double min,
                                            double max);
    void createGroupBox(const QString& title,
                        const std::vector<std::pair<QString,
                                                    std::variant<std::function<void()>,
                                                                 QPushButton*>>>& buttons);
    void updateImageDisplay();
    void updateDarkLineInfoDisplay();
    void updateCalibrationButtonText();
    void updateLineInfo(const QString& info);
    void resetDetectedLines();
    void processDetectedLines(const DarkLinePtrArray* lines);

    // Member variables
    QVBoxLayout* m_mainLayout;
    QVBoxLayout* m_scrollLayout;
    QScrollArea* m_scrollArea;
    ImageProcessor& m_imageProcessor;
    ImageLabel* m_imageLabel;
    QLabel* m_pixelInfoLabel;
    QLabel* m_lastActionLabel;
    QLabel* m_lastActionParamsLabel;
    QLabel* m_gpuTimingLabel;
    QLabel* m_cpuTimingLabel;
    QLabel* m_darkLineInfoLabel;
    QLabel* m_imageSizeLabel;

    double m_lastGpuTime;
    double m_lastCpuTime;
    bool m_hasCpuClaheTime;
    bool m_hasGpuClaheTime;

    Histogram* m_histogram;
    std::vector<QPushButton*> m_allButtons;
    QGroupBox* m_zoomControlsGroup;

    // Buttons
    QPushButton* m_fixZoomButton;
    QPushButton* m_zoomButton;
    QPushButton* m_zoomInButton;
    QPushButton* m_zoomOutButton;
    QPushButton* m_resetZoomButton;
    QPushButton* m_calibrationButton;
    QPushButton* m_resetCalibrationButton;

    QMessageBox* m_zoomWarningBox;
    std::vector<ImageProcessor::DarkLine> m_detectedLines;

    // Constants
    static constexpr int DEFAULT_LABEL_MARGIN = 5;
    static constexpr int DEFAULT_LABEL_SPACING = 30;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr float DEFAULT_ZOOM_STEP = 0.2f;

    DarkLinePointerProcessor::DarkLine convertToDarkLinePointer(const ImageProcessor::DarkLine& line) {
        DarkLinePointerProcessor::DarkLine pointerLine;
        pointerLine.x = line.x;
        pointerLine.y = line.y;
        pointerLine.startX = line.startX;
        pointerLine.startY = line.startY;
        pointerLine.endX = line.endX;
        pointerLine.endY = line.endY;
        pointerLine.width = line.width;
        pointerLine.isVertical = line.isVertical;
        pointerLine.inObject = line.inObject;
        return pointerLine;
    }

    // Convert vector of ImageProcessor::DarkLine to vector of DarkLinePointerProcessor::DarkLine
    std::vector<DarkLinePointerProcessor::DarkLine> convertToDarkLinePointerVector(
        const std::vector<ImageProcessor::DarkLine>& lines) {
        std::vector<DarkLinePointerProcessor::DarkLine> pointerLines;
        pointerLines.reserve(lines.size());
        for (const auto& line : lines) {
            pointerLines.push_back(convertToDarkLinePointer(line));
        }
        return pointerLines;
    }
};

#endif // CONTROL_PANEL_H
