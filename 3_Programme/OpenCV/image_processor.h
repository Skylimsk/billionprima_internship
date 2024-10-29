#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <vector>
#include <string>
#include <QRect>
#include <QImage>
#include <QPixmap>
#include <QLabel>
#include <stack>
#include <opencv2/opencv.hpp>

#include "image_processing_params.h"

class ImageProcessor {
public:
    ImageProcessor(QLabel* imageLabel);

    void loadTxtImage(const std::string& txtFilePath);
    void saveImage(const QString& filePath);
    void cropRegion(const QRect& region);

    void updateImageDisplay();

    void processImage();
    void processYXAxis(std::vector<std::vector<uint16_t>>& image, int linesToAvgY, int linesToAvgX);
    void processAndMergeImageParts();
    void applyMedianFilter(std::vector<std::vector<uint16_t>>& image, int filterKernelSize);
    void applyHighPassFilter(std::vector<std::vector<uint16_t>>& image);
    cv::Mat applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);

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

    void setLastAction(const QString& action);
    QString revertImage();
    QString getCurrentAction() const;

private:
    std::vector<std::vector<uint16_t>> imgData;
    std::vector<std::vector<uint16_t>> originalImg;
    std::vector<std::vector<uint16_t>> finalImage;
    std::stack<std::vector<std::vector<uint16_t>>> imageHistory;
    QLabel* imageLabel;
    QRect selectedRegion;
    bool regionSelected;
    int rotationState;
    int kernelSize;

    void processRegion(const QRect& region, std::function<void(int, int)> operation);
    void saveImageState();
    void saveCurrentState();

    ImageProcessingParams params;

    std::stack<QString> actionHistory;
    QString lastAction;

    cv::Mat convertTo8Bit(const cv::Mat& input);
    cv::Mat convertTo16Bit(const cv::Mat& input);
};

#endif // IMAGE_PROCESSOR_H
