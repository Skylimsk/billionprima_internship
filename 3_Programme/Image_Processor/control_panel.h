#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QFontMetrics>
#include <QRadioButton>
#include <QListWidget>
#include <QGroupBox>
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
#include "graph_processor.h"
#include "zoom.h"
#include "darkline_pointer.h"
#include "display_window.h"
#include "DataCalibration.h"
#include "CGParams.h"
#include "CGProcessImage.h"
#include "graph_3d_processor.h"

class ControlPanel : public QWidget {
    Q_OBJECT

signals:
    void fileLoaded(bool loaded);

public:

    std::mutex m_detectedLinesMutex;

    struct SafeDarkLine {
        bool isValid = false;
        double x = 0;
        double y = 0;
        double width = 0;
        bool isVertical = false;
        bool inObject = false;
        double startX = 0;
        double startY = 0;
        double endX = 0;
        double endY = 0;
    };

    ControlPanel(ImageProcessor& imageProcessor, ImageLabel* imageLabel, QWidget* parent = nullptr);
    ~ControlPanel();
    void updatePixelInfo(const QPoint& pos);
    void updateLastAction(const QString& action, const QString& parameters = QString());

    ImageProcessor& getImageProcessor() { return m_imageProcessor; }
    DarkLineArray* getDetectedLinesPointer() const { return m_detectedLinesPointer; }
    void resetDetectedLinesPointer();
    void updateDarkLineInfoDisplayPointer();
    QLabel* getDarkLineInfoLabel() { return m_darkLineInfoLabel; }

    // Image Display Methods
    void updateImageDisplay(double** image = nullptr, int height = 0, int width = 0);

    DarkLineArray* getDetectedLinesPointer() {
        std::lock_guard<std::mutex> lock(m_detectedLinesMutex);
        return m_detectedLinesPointer;
    }

    void setDetectedLinesPointer(DarkLineArray* pointer) {
        std::lock_guard<std::mutex> lock(m_detectedLinesMutex);
        if (m_detectedLinesPointer) {
            DarkLinePointerProcessor::destroyDarkLineArray(m_detectedLinesPointer);
        }
        m_detectedLinesPointer = pointer;
    }

    ImageData convertToImageData(double** data, int rows, int cols);
    double** convertFromImageData(const ImageData& imageData);

    QLabel* m_imageSizeLabel;

protected:
    struct LineVisualProperties {
        QColor color;
        float penWidth;
        QRect boundingRect;
        QString label;
        QPointF labelPosition;
    };

private:

    enum class CalibrationMode {
        BOTH_AXES,
        Y_AXIS_ONLY,
        X_AXIS_ONLY
    };

    // Setup Methods
    void setupPixelInfoLabel();
    void setupFileOperations();
    void setupPreProcessingOperations();
    void setupBasicOperations();
    void setupGraph();
    void setupFilteringOperations();
    void setupAdvancedOperations();
    void setupCombinedAdjustments();
    void setupBlackLineDetection();
    void setupZoomControls();
    void setupResetOperations();

    // Event Handlers
    QString handleRevert();
    void enableButtons(bool enable);

    // Drawing Methods
    void drawLineLabelWithCountPointer(QPainter& painter, const DarkLine& line, int count, float zoomLevel, const QSize& imageSize);
    void drawLabelCommon(QPainter& painter, const QString& labelText, int labelX, int labelY, int labelMargin, float zoomLevel, const QSize& imageSize);

    // Zoom Methods
    void toggleZoomMode(bool active);
    bool checkZoomMode();
    QSize calculateZoomedSize(const QSize& originalSize, float zoomLevel) const;

    // Helper Methods
    void validateImageData(const ImageData& imageData);
    void processCalibration(int linesToProcessY, int linesToProcessX, CalibrationMode mode);
    std::pair<double, bool> showInputDialog(
        const QString& title,
        const QString& label,
        double defaultValue,
        double min,
        double max);
    void createGroupBox(const QString& title, const std::vector<std::pair<QString, std::variant<std::function<void()>, QPushButton*>>>& buttons);
    void updateDarkLineInfoDisplay();
    void updateCalibrationButtonText();
    void resetAllParameters();
    void clearAllDetectionResults();
    void resetDetectedLines();
    void updateLastActionLabelSize();

    // Member Variables
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
    QLabel* m_fileTypeLabel;

    double m_lastGpuTime;
    double m_lastCpuTime;
    bool m_hasCpuClaheTime;
    bool m_hasGpuClaheTime;

    GraphProcessor* m_histogram;
    std::vector<QPushButton*> m_allButtons;
    QGroupBox* m_zoomControlsGroup;

    std::unique_ptr<Graph3DProcessor> m_graph3DProcessor;

    // Button Members
    QPushButton* m_fixZoomButton;
    QPushButton* m_zoomButton;
    QPushButton* m_zoomInButton;
    QPushButton* m_zoomOutButton;
    QPushButton* m_resetZoomButton;
    QPushButton* m_calibrationButton;
    QPushButton* m_resetCalibrationButton;

    QMessageBox* m_zoomWarningBox;

    // Constants
    static constexpr int DEFAULT_LABEL_MARGIN = 5;
    static constexpr int DEFAULT_LABEL_SPACING = 30;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr float DEFAULT_ZOOM_STEP = 0.2f;

    // Display Windows
    std::unique_ptr<DisplayWindow> m_lowEnergyWindow;
    std::unique_ptr<DisplayWindow> m_highEnergyWindow;
    std::unique_ptr<DisplayWindow> m_finalWindow;
    bool m_showDisplayWindows = false;

    // Dark Line Detection Results
    DarkLineArray* m_detectedLinesPointer;

    // Operation Buttons
    QPushButton* m_pointerDetectBtn = nullptr;
    QPushButton* m_pointerRemoveBtn = nullptr;
    QPushButton* m_pointerResetBtn = nullptr;
    QPushButton* m_vectorDetectBtn = nullptr;
    QPushButton* m_vectorRemoveBtn = nullptr;
    QPushButton* m_vectorResetBtn = nullptr;

    // Performance Metrics
    double m_fileLoadTime = 0.0;
    double m_histogramTime = 0.0;

    QPixmap m_currentPixmap;

    bool initializeImageData(ImageData& imageData, int& height, int& width);
    static void cleanupImageData(ImageData& imageData);
    bool processDetectedLines(DarkLineArray* outLines, bool success);
    void updateDarkLineDisplay(const QString& detectionInfo);

    void handleDetectionError(const std::exception& e, DarkLineArray* outLines = nullptr);

    static void cleanupImageArray(double** array, int rows);

    struct ImageDataGuard {
        ImageData& data;
        ImageDataGuard(ImageData& d) : data(d) {}
        ~ImageDataGuard() { cleanupImageData(data); }
    };

    QString getTimingString(double processingTime) {
        return QString("\nProcessing Time: %1 ms").arg(processingTime, 0, 'f', 2);
    }
};

#endif // CONTROL_PANEL_H
