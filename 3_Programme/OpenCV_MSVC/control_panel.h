#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <functional>
#include <variant>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "image_processor.h"
#include "image_label.h"
#include "histogram.h"
#include "zoom.h"
#include "darkline_pointer.h"

class ControlPanel : public QWidget {
    Q_OBJECT

signals:
    void fileLoaded(bool loaded);

public:
    ControlPanel(ImageProcessor& imageProcessor, ImageLabel* imageLabel, QWidget* parent = nullptr);
    void updatePixelInfo(const QPoint& pos);
    void updateLastAction(const QString& action, const QString& parameters = QString());

private:
    // Type definitions for clarity
    using DarkLinePtrType = DarkLinePtr;
    using DarkLinePtrArrayType = DarkLinePtrArray;

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
    void handleRevert();
    void enableButtons(bool enable);

    // Conversion functions for DarkLinePointerProcessor integration
    DarkLineImageData convertToImageData(const std::vector<std::vector<uint16_t>>& image);
    std::vector<std::vector<uint16_t>> convertFromImageData(const DarkLineImageData& imageData);

    // Zoom-related methods
    void setupZoomControls();
    void toggleZoomMode(bool active);
    bool checkZoomMode();

    std::pair<double, bool> showInputDialog(const QString& title, const QString& label, double defaultValue, double min, double max);
    void createGroupBox(const QString& title,
                        const std::vector<std::pair<QString, std::variant<std::function<void()>, QPushButton*>>>& buttons);
    void updateImageDisplay();
    void updateDarkLineInfoDisplay();

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
    double m_lastGpuTime;
    double m_lastCpuTime;
    bool m_hasCpuClaheTime;
    bool m_hasGpuClaheTime;

    Histogram* m_histogram;
    std::vector<QPushButton*> m_allButtons;
    QGroupBox* m_zoomControlsGroup;
    QPushButton* m_fixZoomButton;
    QMessageBox* m_zoomWarningBox;

    // Dark Line related members
    std::vector<ImageProcessor::DarkLine> m_detectedLines;  // Using original DarkLine from ImageProcessor
    QLabel* m_darkLineInfoLabel;
    QLabel* m_imageSizeLabel;

    void updateLineInfo(const QString& info);
    void resetDetectedLines();

    // Calibration related members
    QPushButton* m_calibrationButton;
    QPushButton* m_resetCalibrationButton;
    void updateCalibrationButtonText();

    // Helper drawing functions
    void drawLineLabel(QPainter& painter, const QString& text, const QPointF& pos, const ZoomManager& zoomManager);

    // Zoom related members
    QPushButton* m_zoomInButton;
    QPushButton* m_zoomOutButton;
    QPushButton* m_resetZoomButton;
    QPushButton* m_zoomButton;

};

#endif // CONTROL_PANEL_H
