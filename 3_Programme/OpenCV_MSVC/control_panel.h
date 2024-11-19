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
#include "histogram.h"
#include "zoom.h"
#include "darkline_pointer.h"
#include "display_window.h"

class ControlPanel : public QWidget {
    Q_OBJECT

signals:
    void fileLoaded(bool loaded);

public:
    ControlPanel(ImageProcessor& imageProcessor, ImageLabel* imageLabel, QWidget* parent = nullptr);
    ~ControlPanel();
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

    // Helper functions for dark line dialog handling
    QString generateDarkLineInfo(const DarkLineArray* lines);
    void handleRemoveLinesDialog();
    QGroupBox* createRemovalTypeBox(int inObjectCount, int isolatedCount);
    QGroupBox* createMethodSelectionBox();
    QGroupBox* createLineSelectionBox(const DarkLineArray* lines);
    void connectDialogControls(
        QRadioButton* inObjectRadio,
        QRadioButton* isolatedRadio,
        QRadioButton* neighborValuesRadio,
        QRadioButton* stitchRadio,
        QPushButton* selectAllButton,
        QListWidget* lineList,
        QGroupBox* methodBox,
        QGroupBox* lineSelectionBox);
    void updateDialogVisibility(
        QRadioButton* inObjectRadio,
        QGroupBox* methodBox,
        QGroupBox* lineSelectionBox,
        QPushButton* selectAllButton,
        QRadioButton* neighborValuesRadio);
    void handleDialogAccepted(
        QRadioButton* inObjectRadio,
        QRadioButton* isolatedRadio,
        QRadioButton* neighborValuesRadio,
        QRadioButton* stitchRadio,
        QListWidget* lineList,
        const DarkLineArray* initialLines);
    void updateLineList(QRadioButton* inObjectRadio, QListWidget* lineList);

    // 2D Pointer specific methods
    void resetDetectedLinesPointer();
    void updateImageDisplayPointer();
    void updateDarkLineInfoDisplayPointer();
    void handleNeighborValuesRemoval(
        DarkLineArray* lines,
        const std::vector<std::pair<int, int>>& lineIndices,
        bool isInObject);
    void handleIsolatedLinesRemoval();
    void handleDirectStitchRemoval(
        const DarkLineArray* lines,
        std::vector<std::pair<int, int>>& lineIndices,
        bool isInObject);
    QString generateRemovalSummary(
        const DarkLineArray* initialLines,
        const DarkLineArray* finalLines,
        const std::vector<std::pair<int, int>>& removedIndices,
        bool removedInObject,
        const QString& methodStr);

    // Helper functions
    void validateImageData(const ImageData& imageData);
    std::vector<std::vector<uint16_t>> createVectorFromImageData(const ImageData& imageData);
    void convertRowToUint16(const double* sourceRow, std::vector<uint16_t>& destRow, int cols);
    void drawLineLabelWithCount(QPainter& painter,
                                const ImageProcessor::DarkLine& line,
                                int count,
                                float zoomLevel,
                                const QSize& imageSize);
    void drawLineLabelWithCountPointer(QPainter& painter,
                                       const DarkLine& line,
                                       int count,
                                       float zoomLevel,
                                       const QSize& imageSize);
    void drawLabelCommon(QPainter& painter,
                         const QString& labelText,
                         int labelX,
                         int labelY,
                         int labelMargin,
                         float zoomLevel,
                         const QSize& imageSize);
    void countLinesInArray(const DarkLineArray* array, int& inObjectCount, int& isolatedCount);
    bool isVectorMethodActive() const { return !m_detectedLines.empty(); }
    bool isPointerMethodActive() const { return m_detectedLinesPointer != nullptr && m_detectedLinesPointer->rows > 0; }
    void clearAllDetectionResults();

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

    // Constants
    static constexpr int DEFAULT_LABEL_MARGIN = 5;
    static constexpr int DEFAULT_LABEL_SPACING = 30;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr float DEFAULT_ZOOM_STEP = 0.2f;

    std::unique_ptr<DisplayWindow> m_lowEnergyWindow;
    std::unique_ptr<DisplayWindow> m_highEnergyWindow;
    std::unique_ptr<DisplayWindow> m_finalWindow;

    bool m_showDisplayWindows = false;

    // Dark line detection results storage
    std::vector<ImageProcessor::DarkLine> m_detectedLines;     // For original implementation
    DarkLineArray* m_detectedLinesPointer;                    // For 2D pointer implementation
};

#endif // CONTROL_PANEL_H
