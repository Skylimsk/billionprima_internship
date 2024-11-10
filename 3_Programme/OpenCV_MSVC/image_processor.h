#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <vector>
#include <string>
#include <QRect>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <stack>
#include <memory>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "image_processing_params.h"
#include "CLAHE.h"
#include "dark_line.h"
#include "adjustments.h"
#include "interlace.h"
//#include "display_window.h"

class ImageProcessor {
public:
    struct ActionRecord {
        QString action;
        QString parameters;

        QString toString() const {
            if (parameters.isEmpty()) {
                return action;
            }
            return action + " - " + parameters;
        }
    };

    using DarkLine = DarkLineProcessor::DarkLine;

    struct ImageState {
        std::vector<std::vector<uint16_t>> image;
        std::vector<DarkLine> detectedLines;
    };

    enum class SplitMode {
        ALL_PARTS,
        LEFT_MOST,    // First two quarters (LeftLeft & LeftRight)
        RIGHT_MOST    // Last two quarters (RightLeft & RightRight)
    };

    enum class LineRemovalMethod {
        NEIGHBOR_VALUES,
        DIRECT_STITCH
    };

    ImageProcessor(QLabel* imageLabel);

    // Basic Operations
    void loadTxtImage(const std::string& txtFilePath);
    void saveImage(const QString& filePath);
    void cropRegion(const QRect& region);
    void processImage();
    QString revertImage();

    // Image Processing Functions
    void processYXAxis(std::vector<std::vector<uint16_t>>& image, int linesToAvgY, int linesToAvgX);
    void applyMedianFilter(std::vector<std::vector<uint16_t>>& image, int filterKernelSize);
    void applyHighPassFilter(std::vector<std::vector<uint16_t>>& image);

    // Adjustment Methods
    void adjustContrast(float contrastFactor) {
        saveCurrentState();
        ImageAdjustments::adjustContrast(finalImage, contrastFactor);
    }

    void adjustGammaOverall(float gamma) {
        saveCurrentState();
        ImageAdjustments::adjustGammaOverall(finalImage, gamma);
    }

    void sharpenImage(float sharpenStrength) {
        saveCurrentState();
        ImageAdjustments::sharpenImage(finalImage, sharpenStrength);
    }

    void adjustGammaForSelectedRegion(float gamma, const QRect& region) {
        saveCurrentState();
        ImageAdjustments::adjustGammaForSelectedRegion(finalImage, gamma, region);
    }

    void applySharpenToRegion(float sharpenStrength, const QRect& region) {
        saveCurrentState();
        ImageAdjustments::applySharpenToRegion(finalImage, sharpenStrength, region);
    }

    void applyContrastToRegion(float contrastFactor, const QRect& region) {
        saveCurrentState();
        ImageAdjustments::applyContrastToRegion(finalImage, contrastFactor, region);
    }

    // Transformation Functions
    std::vector<std::vector<uint16_t>> rotateImage(const std::vector<std::vector<uint16_t>>& image, int angle);
    void stretchImageY(std::vector<std::vector<uint16_t>>& img, float stretchFactor);
    void stretchImageX(std::vector<std::vector<uint16_t>>& img, float stretchFactor);
    std::vector<std::vector<uint16_t>> distortImage(const std::vector<std::vector<uint16_t>>& image, float distortionFactor, const std::string& direction);
    std::vector<std::vector<uint16_t>> addPadding(const std::vector<std::vector<uint16_t>>& image, int paddingSize);

    // Conversion Functions
    cv::Mat vectorToMat(const std::vector<std::vector<uint16_t>>& image);
    std::vector<std::vector<uint16_t>> matToVector(const cv::Mat& mat);

    // CLAHE Functions
    cv::Mat applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    cv::Mat applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_GPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_CPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize);
    CLAHEProcessor::PerformanceMetrics getLastPerformanceMetrics() const;

    // Dark Line Processing
    std::vector<DarkLine> detectDarkLines();
    void removeDarkLines(const std::vector<DarkLine>& lines);
    void removeAllDarkLines();
    void removeInObjectDarkLines();
    void removeIsolatedDarkLines();
    void removeDarkLinesSelective(bool removeInObject, bool removeIsolated, LineRemovalMethod method = LineRemovalMethod::NEIGHBOR_VALUES);
    void removeDarkLinesSequential(const std::vector<DarkLine>& selectedLines, bool removeInObject, bool removeIsolated, LineRemovalMethod method);

    // State Management
    void saveCurrentState();
    void setLastAction(const QString& action, const QString& parameters = QString());
    QString getCurrentAction() const;
    ActionRecord getLastActionRecord() const;
    void clearDetectedLines() { m_detectedLines.clear(); }
    const std::vector<DarkLine>& getDetectedLines() const { return m_detectedLines; }
    const std::vector<size_t>& getLastRemovedLines() const { return m_lastRemovedLines; }

    // Getters and Setters
    const std::vector<std::vector<uint16_t>>& getFinalImage() const;
    void updateAndSaveFinalImage(const std::vector<std::vector<uint16_t>>& newImage);

    // Zoom Functions
    void setZoomLevel(float level);
    float getZoomLevel() const { return currentZoomLevel; }
    void zoomIn() { setZoomLevel(currentZoomLevel * ZOOM_STEP); }
    void zoomOut() { setZoomLevel(currentZoomLevel / ZOOM_STEP); }
    void resetZoom() { setZoomLevel(1.0f); }
    QSize getZoomedImageDimensions() const;

    InterlaceProcessor::InterlacedResult processEnhancedInterlacedSections(
        InterlaceProcessor::StartPoint lowEnergyStart,
        InterlaceProcessor::StartPoint highEnergyStart,
        InterlaceProcessor::MergeMethod mergeMethod
        );

private:
    std::vector<std::vector<uint16_t>> imgData;
    std::vector<std::vector<uint16_t>> originalImg;
    std::vector<std::vector<uint16_t>> finalImage;
    std::stack<ImageState> imageHistory;
    std::stack<ActionRecord> actionHistory;
    QString lastAction;

    QLabel* imageLabel;
    QRect selectedRegion;
    bool regionSelected;
    int rotationState;
    int kernelSize;

    void saveImageState();

    ImageProcessingParams params;
    CLAHEProcessor claheProcessor;

    std::vector<std::vector<uint16_t>> preProcessedImage;
    bool hasCLAHEBeenApplied;

    // Zoom related
    float currentZoomLevel;
    const float MIN_ZOOM_LEVEL = 0.1f;
    const float MAX_ZOOM_LEVEL = 10.0f;
    const float ZOOM_STEP = 1.2f;

    std::vector<DarkLine> m_detectedLines;
    std::vector<size_t> m_lastRemovedLines;

};

#endif // IMAGE_PROCESSOR_H
