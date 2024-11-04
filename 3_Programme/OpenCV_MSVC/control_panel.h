// control_panel.h
#ifndef CONTROL_PANEL_H
#define CONTROL_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <functional>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "image_processor.h"
#include "image_label.h"
#include "histogram.h"

class ControlPanel : public QWidget {
    Q_OBJECT

public:
    ControlPanel(ImageProcessor& imageProcessor, ImageLabel* imageLabel, QWidget* parent = nullptr);
    void updatePixelInfo(const QPoint& pos);
    void updateLastAction(const QString& action, const QString& parameters = QString());

private:
    void setupPixelInfoLabel();
    void setupFileOperations();
    void setupBasicOperations();
    void setupFilteringOperations();
    void setupAdvancedOperations();
    void setupGlobalAdjustments();
    void setupRegionalAdjustments();
    void setupCLAHEOperations();
    void setupBlackLineDetection();
    void handleRevert();

    std::pair<double, bool> showInputDialog(const QString& title, const QString& label, double defaultValue, double min, double max);
    void createGroupBox(const QString& title, const std::vector<std::pair<QString, std::function<void()>>>& buttons);
    void updateImageDisplay();
    void resetDetectedLines();

    QVBoxLayout* m_mainLayout;
    QVBoxLayout* m_scrollLayout;
    QScrollArea* m_scrollArea;
    ImageProcessor& m_imageProcessor;
    ImageLabel* m_imageLabel;
    QLabel* m_pixelInfoLabel;
    QLabel* m_lastActionLabel;
    QLabel* m_lastActionParamsLabel;  // Added this declaration
    QLabel* m_gpuTimingLabel;
    QLabel* m_cpuTimingLabel;
    double m_lastGpuTime;
    double m_lastCpuTime;
    bool m_hasCpuClaheTime;
    bool m_hasGpuClaheTime;

    Histogram* m_histogram;

    std::vector<ImageProcessor::DarkLine> m_detectedLines;
};

#endif // CONTROL_PANEL_H
