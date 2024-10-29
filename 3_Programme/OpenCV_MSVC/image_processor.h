#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <vector>
#include <string>
#include <QRect>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <stack>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "image_processing_params.h"
#include "CLAHE.h"

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

    ImageProcessor(QLabel* imageLabel);

    void loadTxtImage(const std::string& txtFilePath);
    void saveImage(const QString& filePath);
    void cropRegion(const QRect& region);

    void processImage();
    void processYXAxis(std::vector<std::vector<uint16_t>>& image, int linesToAvgY, int linesToAvgX);
    void processAndMergeImageParts();
    void applyMedianFilter(std::vector<std::vector<uint16_t>>& image, int filterKernelSize);
    void applyHighPassFilter(std::vector<std::vector<uint16_t>>& image);

    void adjustContrast(std::vector<std::vector<uint16_t>>& img, float contrastFactor);
    void adjustGammaOverall(std::vector<std::vector<uint16_t>>& img, float gamma);
    void sharpenImage(std::vector<std::vector<uint16_t>>& img, float sharpenStrength);
    void adjustGammaForSelectedRegion(float gamma, const QRect& region);
    void applySharpenToRegion(float sharpenStrength, const QRect& region);
    void applyContrastToRegion(float contrastFactor, const QRect& region);
    std::vector<std::vector<uint16_t>> rotateImage(const std::vector<std::vector<uint16_t>>& image, int angle);
    void stretchImageY(std::vector<std::vector<uint16_t>>& img, float yStretchFactor);
    void stretchImageX(std::vector<std::vector<uint16_t>>& img, float xStretchFactor);
    std::vector<std::vector<uint16_t>> distortImage(const std::vector<std::vector<uint16_t>>& image, float distortionFactor, const std::string& direction);
    std::vector<std::vector<uint16_t>> addPadding(const std::vector<std::vector<uint16_t>>& image, int paddingSize);

    cv::Mat vectorToMat(const std::vector<std::vector<uint16_t>>& image);
    std::vector<std::vector<uint16_t>> matToVector(const cv::Mat& mat);

    const std::vector<std::vector<uint16_t>>& getFinalImage() const;
    void updateAndSaveFinalImage(const std::vector<std::vector<uint16_t>>& newImage);

    static void adjustGammaForRegion(std::vector<std::vector<uint16_t>>& img, float gamma, int startY, int endY, int startX, int endX);
    static void processSharpenChunk(int startY, int endY, int width, std::vector<std::vector<uint16_t>>& img, const std::vector<std::vector<uint16_t>>& tempImg, float sharpenStrength);
    static void processSharpenRegionChunk(int startY, int endY, int left, int right, std::vector<std::vector<uint16_t>>& img, float sharpenStrength);
    static void processContrastChunk(int startY, int endY, int left, int right, float contrastFactor, std::vector<std::vector<uint16_t>>& img);

    // CLAHE related functions
    cv::Mat applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    cv::Mat applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_GPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_CPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize);
    CLAHEProcessor::PerformanceMetrics getLastPerformanceMetrics() const;

    // Action record related functions
    void setLastAction(const QString& action, const QString& parameters = QString());
    QString revertImage();
    QString getCurrentAction() const;
    ActionRecord getLastActionRecord() const;

    struct DarkLine {
        cv::Point start;
        cv::Point end;
        int thickness;
    };

    std::vector<DarkLine> detectDarkLines(uint16_t brightThreshold = 60000,
                                          uint16_t darkThreshold = 5000,
                                          int minLineLength = 20);

    void removeDarkLines(const std::vector<DarkLine>& lines);
    void removeFromZeroX(const std::vector<DarkLine>& lines);

private:
    std::vector<std::vector<uint16_t>> imgData;
    std::vector<std::vector<uint16_t>> originalImg;
    std::vector<std::vector<uint16_t>> finalImage;
    std::stack<std::vector<std::vector<uint16_t>>> imageHistory;
    std::stack<ActionRecord> actionHistory;
    QString lastAction;

    QLabel* imageLabel;
    QRect selectedRegion;
    bool regionSelected;
    int rotationState;
    int kernelSize;

    void processRegion(const QRect& region, std::function<void(int, int)> operation);
    void saveImageState();
    void saveCurrentState();

    ImageProcessingParams params;
    CLAHEProcessor claheProcessor;

    std::vector<std::vector<uint16_t>> preProcessedImage; // Store state before CLAHE
    bool hasCLAHEBeenApplied;

    bool isInBrightRegion(const std::vector<std::vector<uint16_t>>& image,
                          int x, int y, uint16_t brightThreshold);
    std::vector<std::vector<bool>> findBrightRegions(uint16_t brightThreshold);
    void refineDarkLineDetection(std::vector<DarkLine>& lines);

    cv::Mat prepareImageForLineDetection(uint16_t brightThreshold, uint16_t darkThreshold);
    uint16_t interpolateValue(int x, int y, int margin = 5);

    int calculateLineThickness(const std::vector<std::pair<int, int>>& linePixels,
                               int minX, int maxX, int minY, int maxY);


};

#endif // IMAGE_PROCESSOR_H
