#ifdef _MSC_VER
#pragma warning(disable: 4068) // unknown pragma
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4244) // conversion from 'double' to 'float', possible loss of data
#pragma warning(disable: 4458) // declaration hides class member
#pragma warning(disable: 4005) // macro redefinition
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4702)
#endif

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <opencv2/opencv.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/cuda.hpp>
#include <QDebug>
#include <QRect>
#include <fstream>
#include <thread>
#include <algorithm>
#include <QtWidgets/QMessageBox>
#define _USE_MATH_DEFINES
#include <cmath>
#include "image_processor.h"
#include "adjustments.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include "stb_image_write.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

ImageProcessor::ImageProcessor(QLabel* imageLabel)
    : imageLabel(imageLabel)
    , regionSelected(false)
    , rotationState(0)
    , kernelSize(0)
    , hasCLAHEBeenApplied(false)
{
    // Initialize image display parameters with defaults
    m_imageDisplayParams.EnergyMode_Default = 2;
    m_imageDisplayParams.DualRowMode_Default = 0;
    m_imageDisplayParams.DualRowDirection_Default = 0;
    m_imageDisplayParams.LaneNumber_Default = "0101";
    m_imageDisplayParams.HighLowLayout_Default = 2;
    m_imageDisplayParams.DefaultSpeed_Default = 5;

    // Set current lane data to defaults
    m_imageDisplayParams.LaneData.EnergyMode = m_imageDisplayParams.EnergyMode_Default;
    m_imageDisplayParams.LaneData.DualRowMode = m_imageDisplayParams.DualRowMode_Default;
    m_imageDisplayParams.LaneData.DualRowDirection = m_imageDisplayParams.DualRowDirection_Default;
    m_imageDisplayParams.LaneData.LaneNumber = m_imageDisplayParams.LaneNumber_Default;
    m_imageDisplayParams.LaneData.HighLowLayout = m_imageDisplayParams.HighLowLayout_Default;
    m_imageDisplayParams.LaneData.DefaultSpeed = m_imageDisplayParams.DefaultSpeed_Default;

    // Initialize total lane count
    m_imageDisplayParams.TotalLane = 0;
}

void ImageProcessor::saveCurrentState() {
    ImageState currentState;
    currentState.image = finalImage;
    currentState.detectedLines = m_detectedLines;
    imageHistory.push(currentState);
}

QString ImageProcessor::getCurrentAction() const {
    if (!actionHistory.empty()) {
        return actionHistory.top().toString();
    }
    return "None";
}

ImageProcessor::ActionRecord ImageProcessor::getLastActionRecord() const {
    if (!actionHistory.empty()) {
        return actionHistory.top();
    }
    return {"None", ""};
}

void ImageProcessor::loadTxtImage(const std::string& txtFilePath) {
    std::ifstream inFile(txtFilePath);
    if (!inFile.is_open()) {
        std::cerr << "Error: Cannot open file " << txtFilePath << std::endl;
        return;
    }

    std::string line;
    imgData.clear();
    size_t maxWidth = 0;

    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<uint16_t> row;
        uint32_t value;
        while (ss >> value) {
            // Convert to 16-bit, clamping to maximum value
            row.push_back(static_cast<uint16_t>(std::min(value, static_cast<uint32_t>(65535))));
        }
        if (!row.empty()) {
            maxWidth = std::max(maxWidth, row.size());
            imgData.push_back(row);
        }
    }
    inFile.close();

    if (imgData.empty()) {
        std::cerr << "Error: No valid data found in the file." << std::endl;
        return;
    }

    // Pad all rows to the same width with the last value in each row
    for (auto& row : imgData) {
        if (row.size() < maxWidth) {
            row.resize(maxWidth, row.empty() ? 0 : row.back());
        }
    }

    originalImg = imgData;
    finalImage = imgData;

    saveCurrentState();

    // Reset selection-related variables
    selectedRegion = QRect();
    regionSelected = false;
}

void ImageProcessor::setLastAction(const QString& action, const QString& parameters) {
    actionHistory.push({action, parameters});
    lastAction = actionHistory.top().toString();
}

void ImageProcessor::saveImage(const QString& filePath) {
    int width = finalImage[0].size();
    int height = finalImage.size();
    std::vector<uint8_t> outputBuffer(width * height * 3);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t pixelValue = finalImage[y][x] >> 8;
            outputBuffer[3 * (y * width + x)] = pixelValue;
            outputBuffer[3 * (y * width + x) + 1] = pixelValue;
            outputBuffer[3 * (y * width + x) + 2] = pixelValue;
        }
    }

    if (stbi_write_png(filePath.toStdString().c_str(), width, height, 3, outputBuffer.data(), width * 3)) {
        qDebug() << "Image saved successfully: " << filePath;
    } else {
        qDebug() << "Failed to save the image.";
    }
}

QString ImageProcessor::revertImage() {
    if (!imageHistory.empty() && !actionHistory.empty()) {
        ImageState prevState = imageHistory.top();
        finalImage = prevState.image;
        m_detectedLines = prevState.detectedLines;
        imageHistory.pop();

        ActionRecord currentAction = actionHistory.top();
        actionHistory.pop();

        // For calibration operations, ensure parameters are maintained
        if (currentAction.action.startsWith("Calibration")) {
            if (currentAction.parameters.contains("Y:") && !currentAction.parameters.contains("X:")) {
                // Y-axis only calibration
                int yParam = currentAction.parameters.mid(2).toInt();
                InterlaceProcessor::setCalibrationParams(yParam, 0);
            } else if (!currentAction.parameters.contains("Y:") && currentAction.parameters.contains("X:")) {
                // X-axis only calibration
                int xParam = currentAction.parameters.mid(2).toInt();
                InterlaceProcessor::setCalibrationParams(0, xParam);
            }
            // Both axes case is already handled by existing calibration params
        }

        if (!actionHistory.empty()) {
            lastAction = actionHistory.top().toString();
            return actionHistory.top().toString();
        } else {
            lastAction = "None";
            return "None";
        }
    }
    return QString();
}

void ImageProcessor::cropRegion(const QRect& region) {

    saveCurrentState();

    qDebug() << "Entering cropRegion function";
    qDebug() << "Selected region:" << region;
    qDebug() << "Current image size:" << finalImage.size() << "x" << (finalImage.empty() ? 0 : finalImage[0].size());

    if (finalImage.empty()) {
        qDebug() << "Error: finalImage is empty";
        return;
    }

    // Normalize the rectangle to ensure left <= right and top <= bottom
    int left = std::min(region.left(), region.right());
    int top = std::min(region.top(), region.bottom());
    int right = std::max(region.left(), region.right());
    int bottom = std::max(region.top(), region.bottom());

    // Clamp values to image boundaries
    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(static_cast<int>(finalImage[0].size()), right);
    bottom = std::min(static_cast<int>(finalImage.size()), bottom);

    QRect adjustedRegion(left, top, right - left, bottom - top);
    qDebug() << "Adjusted crop region:" << adjustedRegion;

    if (adjustedRegion.width() <= 0 || adjustedRegion.height() <= 0) {
        qDebug() << "Invalid crop region, skipping crop";
        return;
    }

    size_t newWidth = adjustedRegion.width();
    size_t newHeight = adjustedRegion.height();
    std::vector<std::vector<uint16_t>> croppedImage(newHeight, std::vector<uint16_t>(newWidth));

    for (size_t y = 0; y < newHeight; ++y) {
        for (size_t x = 0; x < newWidth; ++x) {
            croppedImage[y][x] = finalImage[top + y][left + x];
        }
    }

    saveImageState(); // Save the current state before modifying
    finalImage = croppedImage;
    qDebug() << "New image size after crop:" << finalImage.size() << "x" << finalImage[0].size();
}

void ImageProcessor::processImage() {
    saveImageState();
    finalImage = originalImg;
}

void ImageProcessor::processYXAxis(std::vector<std::vector<uint16_t>>& image, int linesToAvgY, int linesToAvgX) {
    saveCurrentState();

    // Store calibration parameters
    if (linesToAvgY > 0 && linesToAvgX == 0) {
        InterlaceProcessor::setCalibrationParams(linesToAvgY, 0);  // Y-axis only
    } else if (linesToAvgY == 0 && linesToAvgX > 0) {
        InterlaceProcessor::setCalibrationParams(0, linesToAvgX);  // X-axis only
    } else {
        InterlaceProcessor::setCalibrationParams(linesToAvgY, linesToAvgX);  // Both axes
    }

    int height = image.size();
    int width = image[0].size();

    // Y-axis processing only if Y lines are specified
    if (linesToAvgY > 0) {
        std::vector<float> referenceYMean(width, 0.0f);

        // Calculate reference means for Y-axis
        for (int y = 0; y < std::min(linesToAvgY, height); ++y) {
            for (int x = 0; x < width; ++x) {
                referenceYMean[x] += static_cast<float>(image[y][x]);
            }
        }

        // Normalize reference means and add epsilon to avoid division by zero
        const float epsilon = 1e-6f;
        for (int x = 0; x < width; ++x) {
            referenceYMean[x] = std::max(referenceYMean[x] / linesToAvgY, epsilon);
        }

        // Apply Y-axis normalization with clamping
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float normalizedValue = static_cast<float>(image[y][x]) / referenceYMean[x];
                // Clamp the normalized value to a reasonable range (e.g., 0.1 to 10.0)
                normalizedValue = std::clamp(normalizedValue, 0.1f, 10.0f);
                // Scale to 16-bit range and clamp again
                float scaledValue = normalizedValue * 65535.0f;
                image[y][x] = static_cast<uint16_t>(std::clamp(scaledValue, 0.0f, 65535.0f));
            }
        }
    }

    // X-axis processing only if X lines are specified
    if (linesToAvgX > 0) {
        std::vector<float> referenceXMean(height, 0.0f);

        // Calculate reference means for X-axis
        for (int y = 0; y < height; ++y) {
            for (int x = width - linesToAvgX; x < width; ++x) {
                referenceXMean[y] += static_cast<float>(image[y][x]);
            }
        }

        // Normalize reference means and add epsilon to avoid division by zero
        const float epsilon = 1e-6f;
        for (int y = 0; y < height; ++y) {
            referenceXMean[y] = std::max(referenceXMean[y] / linesToAvgX, epsilon);
        }

        // Apply X-axis normalization with clamping
        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                float normalizedValue = static_cast<float>(image[y][x]) / referenceXMean[y];
                // Clamp the normalized value to a reasonable range (e.g., 0.1 to 10.0)
                normalizedValue = std::clamp(normalizedValue, 0.1f, 10.0f);
                // Scale to 16-bit range and clamp again
                float scaledValue = normalizedValue * 65535.0f;
                image[y][x] = static_cast<uint16_t>(std::clamp(scaledValue, 0.0f, 65535.0f));
            }
        }
    }
}

void ImageProcessor::applyMedianFilter(std::vector<std::vector<uint16_t>>& image, int filterKernelSize) {

    saveCurrentState();

    kernelSize = filterKernelSize;

    int height = image.size();
    int width = image[0].size();
    std::vector<std::vector<uint16_t>> outputImage = image;

    int offset = kernelSize / 2;
    for (int y = offset; y < height - offset; ++y) {
        for (int x = offset; x < width - offset; ++x) {
            std::vector<uint16_t> window;
            for (int ky = -offset; ky <= offset; ++ky) {
                for (int kx = -offset; kx <= offset; ++kx) {
                    window.push_back(image[y + ky][x + kx]);
                }
            }
            std::sort(window.begin(), window.end());
            outputImage[y][x] = window[window.size() / 2];
        }
    }
    image = outputImage;
}

void ImageProcessor::applyHighPassFilter(std::vector<std::vector<uint16_t>>& image) {

    saveCurrentState();

    std::vector<std::vector<int>> kernel = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
    };

    int height = image.size();
    int width = image[0].size();
    std::vector<std::vector<uint16_t>> outputImage = image;

    int kernelSize = 3;
    int halfSize = kernelSize / 2;

    float scalingFactor = 1.5f;

    for (int y = halfSize; y < height - halfSize; ++y) {
        for (int x = halfSize; x < width - halfSize; ++x) {
            int newValue = 0;

            for (int ky = -halfSize; ky <= halfSize; ++ky) {
                for (int kx = -halfSize; kx <= halfSize; ++kx) {
                    newValue += image[y + ky][x + kx] * kernel[ky + halfSize][kx + halfSize];
                }
            }

            newValue = static_cast<int>(newValue * scalingFactor);
            outputImage[y][x] = static_cast<uint16_t>(std::clamp(newValue, 0, 65535));
        }
    }

    image = outputImage;
}

cv::Mat ImageProcessor::applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize) {
    saveCurrentState();
    preProcessedImage = finalImage;  // 保存当前图像状态，而不是输入图像
    hasCLAHEBeenApplied = true;
    cv::Mat result = claheProcessor.applyCLAHE(inputImage, clipLimit, tileSize);
    finalImage = matToVector(result);  // 更新 finalImage 为 CLAHE 处理后的结果
    return result;
}

cv::Mat ImageProcessor::applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize) {
    saveCurrentState();
    preProcessedImage = finalImage;  // 保存当前图像状态，而不是输入图像
    hasCLAHEBeenApplied = true;
    cv::Mat result = claheProcessor.applyCLAHE_CPU(inputImage, clipLimit, tileSize);
    finalImage = matToVector(result);  // 更新 finalImage 为 CLAHE 处理后的结果
    return result;
}


void ImageProcessor::applyThresholdCLAHE_GPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize) {
    saveCurrentState();
    if (hasCLAHEBeenApplied) {

        double enhancedClipLimit = clipLimit * 2.0;  // 增加 clipLimit 使效果更明显
        std::vector<std::vector<uint16_t>> processImage = finalImage;
        claheProcessor.applyThresholdCLAHE_GPU(processImage, threshold, enhancedClipLimit, tileSize);
        finalImage = processImage;
    } else {

        claheProcessor.applyThresholdCLAHE_GPU(finalImage, threshold, clipLimit, tileSize);
    }
    hasCLAHEBeenApplied = false;
}

void ImageProcessor::applyThresholdCLAHE_CPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize) {
    saveCurrentState();
    if (hasCLAHEBeenApplied) {

        double enhancedClipLimit = clipLimit * 2.0;
        std::vector<std::vector<uint16_t>> processImage = finalImage;
        claheProcessor.applyThresholdCLAHE_CPU(processImage, threshold, enhancedClipLimit, tileSize);
        finalImage = processImage;
    } else {
        claheProcessor.applyThresholdCLAHE_CPU(finalImage, threshold, clipLimit, tileSize);
    }
    hasCLAHEBeenApplied = false;
}

CLAHEProcessor::PerformanceMetrics ImageProcessor::getLastPerformanceMetrics() const {
    return claheProcessor.getLastPerformanceMetrics();
}

std::vector<std::vector<uint16_t>> ImageProcessor::rotateImage(const std::vector<std::vector<uint16_t>>& image, int angle) {
    saveCurrentState();
    int height = image.size();
    int width = image[0].size();
    int newHeight, newWidth;

    if (angle == 90 || angle == 270) {
        newHeight = width;
        newWidth = height;
    } else {
        newHeight = height;
        newWidth = width;
    }
    std::vector<std::vector<uint16_t>> rotatedImage(newHeight, std::vector<uint16_t>(newWidth, 0));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int newX, newY;
            if (angle == 90) {
                newX = y;
                newY = width - x - 1;
            } else if (angle == 180) {
                newX = width - x - 1;
                newY = height - y - 1;
            } else if (angle == 270) {
                newX = height - y - 1;
                newY = x;
            } else {
                newX = x;
                newY = y;
            }
            rotatedImage[newY][newX] = image[y][x];
        }
    }
    return rotatedImage;
}

void ImageProcessor::stretchImageY(std::vector<std::vector<uint16_t>>& img, float yStretchFactor) {
    saveCurrentState();
    if (yStretchFactor <= 0) return;

    int origHeight = img.size();
    int width = img[0].size();
    int newHeight = static_cast<int>(origHeight * yStretchFactor);

    std::vector<std::vector<uint16_t>> stretchedImg(newHeight, std::vector<uint16_t>(width));

    for (int y = 0; y < newHeight; ++y) {
        float srcY = y / yStretchFactor;
        int yLow = static_cast<int>(srcY);
        int yHigh = std::min(yLow + 1, origHeight - 1);
        float weight = srcY - yLow;

        for (int x = 0; x < width; ++x) {
            stretchedImg[y][x] = static_cast<uint16_t>(
                (1 - weight) * img[yLow][x] + weight * img[yHigh][x]);
        }
    }

    img = std::move(stretchedImg);
}

void ImageProcessor::stretchImageX(std::vector<std::vector<uint16_t>>& img, float xStretchFactor) {
    saveCurrentState();
    if (xStretchFactor <= 0) return;

    int height = img.size();
    int origWidth = img[0].size();
    int newWidth = static_cast<int>(origWidth * xStretchFactor);

    std::vector<std::vector<uint16_t>> stretchedImg(height, std::vector<uint16_t>(newWidth));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            float srcX = x / xStretchFactor;
            int xLow = static_cast<int>(srcX);
            int xHigh = std::min(xLow + 1, origWidth - 1);
            float weight = srcX - xLow;

            stretchedImg[y][x] = static_cast<uint16_t>(
                (1 - weight) * img[y][xLow] + weight * img[y][xHigh]);
        }
    }

    img = std::move(stretchedImg);
}

std::vector<std::vector<uint16_t>> ImageProcessor::distortImage(const std::vector<std::vector<uint16_t>>& image, float distortionFactor, const std::string& direction) {
    saveCurrentState();
    int height = image.size();
    int width = image[0].size();
    int centerX = width / 2;
    int centerY = height / 2;

    std::vector<std::vector<uint16_t>> distortedImage = image;

    if (direction == "Left") {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < centerX; ++x) {
                int newX = static_cast<int>(x + distortionFactor * std::sin(M_PI * x / centerX));
                newX = std::min(std::max(newX, 0), centerX - 1);
                distortedImage[y][x] = image[y][newX];
            }
        }

    } else if (direction == "Right") {
        for (int y = 0; y < height; ++y) {
            for (int x = centerX; x < width; ++x) {
                int newX = static_cast<int>(x + distortionFactor * std::sin(M_PI * (x - centerX) / (width - centerX)));
                newX = std::min(std::max(newX, centerX), width - 1);
                distortedImage[y][newX] = image[y][x];
            }
        }

    } else if (direction == "Top") {
        for (int y = centerY - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                int newY = static_cast<int>(y + distortionFactor * std::sin(M_PI * (y - centerY) / centerY));
                newY = std::min(std::max(newY, 0), centerY - 1);
                distortedImage[newY][x] = image[y][x];
            }
        }

    } else if (direction == "Bottom") {
        for (int y = centerY; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int newY = static_cast<int>(y + distortionFactor * std::sin(M_PI * (y - centerY) / (height - centerY)));
                newY = std::min(std::max(newY, centerY), height - 1);
                distortedImage[newY][x] = image[y][x];
            }
        }
    }

    return distortedImage;
}

std::vector<std::vector<uint16_t>> ImageProcessor::addPadding(const std::vector<std::vector<uint16_t>>& image, int paddingSize) {
    saveCurrentState();
    int height = image.size();
    int width = image[0].size();

    int newHeight = height + 2 * paddingSize;
    int newWidth = width + 2 * paddingSize;

    std::vector<std::vector<uint16_t>> paddedImage(newHeight, std::vector<uint16_t>(newWidth, 65535));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            paddedImage[y + paddingSize][x + paddingSize] = image[y][x];
        }
    }
    return paddedImage;
}

cv::Mat ImageProcessor::vectorToMat(const std::vector<std::vector<uint16_t>>& image) {
    int height = image.size();
    int width = image[0].size();
    cv::Mat mat(height, width, CV_16UC1);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            mat.at<uint16_t>(y, x) = image[y][x];
        }
    }

    return mat;
}

std::vector<std::vector<uint16_t>> ImageProcessor::matToVector(const cv::Mat& mat) {
    int height = mat.rows;
    int width = mat.cols;
    std::vector<std::vector<uint16_t>> image(height, std::vector<uint16_t>(width));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            image[y][x] = mat.at<uint16_t>(y, x);
        }
    }

    return image;
}

const std::vector<std::vector<uint16_t>>& ImageProcessor::getFinalImage() const {
    return finalImage;
}

void ImageProcessor::saveImageState() {
    ImageState currentState;
    currentState.image = finalImage;
    currentState.detectedLines = m_detectedLines;
    imageHistory.push(currentState);
}

void ImageProcessor::updateAndSaveFinalImage(const std::vector<std::vector<uint16_t>>& newImage) {
    saveCurrentState();
    finalImage = newImage;
}

std::vector<ImageProcessor::DarkLine> ImageProcessor::detectDarkLines() {
    m_lastRemovedLines.clear(); // Clear previous removal history when detecting new lines
    m_detectedLines = DarkLineProcessor::detectDarkLines(finalImage);
    return m_detectedLines;
}

void ImageProcessor::removeDarkLines(const std::vector<DarkLine>& lines) {
    if (lines.empty()) return;
    saveCurrentState();
    // Now we need to pass the line width when calling findReplacementValue
    DarkLineProcessor::removeDarkLines(finalImage, lines);
    clearDetectedLines();
}

void ImageProcessor::removeAllDarkLines() {
    if (m_detectedLines.empty()) {
        m_detectedLines = detectDarkLines();
    }
    saveCurrentState();

    DarkLineProcessor::removeDarkLinesSelective(finalImage, m_detectedLines, true, true);

    m_lastRemovedLines.clear();
    for (size_t i = 0; i < m_detectedLines.size(); ++i) {
        m_lastRemovedLines.push_back(i + 1);
    }

    clearDetectedLines();
}

void ImageProcessor::removeInObjectDarkLines() {
    if (m_detectedLines.empty()) {
        m_detectedLines = detectDarkLines();
    }
    saveCurrentState();

    // Process only in-object lines
    DarkLineProcessor::removeDarkLinesSelective(finalImage, m_detectedLines, true, false);

    // Track removed line indices
    m_lastRemovedLines.clear();
    for (size_t i = 0; i < m_detectedLines.size(); ++i) {
        if (m_detectedLines[i].inObject) {
            m_lastRemovedLines.push_back(i + 1);
        }
    }

    clearDetectedLines();
}

void ImageProcessor::removeIsolatedDarkLines() {
    if (m_detectedLines.empty()) {
        m_detectedLines = detectDarkLines();
    }
    saveCurrentState();

    // Process only isolated lines
    DarkLineProcessor::removeDarkLinesSelective(finalImage, m_detectedLines, false, true);

    // Track removed line indices
    m_lastRemovedLines.clear();
    for (size_t i = 0; i < m_detectedLines.size(); ++i) {
        if (!m_detectedLines[i].inObject) {
            m_lastRemovedLines.push_back(i + 1);
        }
    }

    clearDetectedLines();
}

void ImageProcessor::removeDarkLinesSequential(
    const std::vector<DarkLine>& selectedLines,
    bool removeInObject,
    bool removeIsolated,
    LineRemovalMethod method) {

    if (selectedLines.empty()) return;

    saveCurrentState();

    DarkLineProcessor::RemovalMethod processorMethod =
        (method == LineRemovalMethod::DIRECT_STITCH) ?
            DarkLineProcessor::RemovalMethod::DIRECT_STITCH :
            DarkLineProcessor::RemovalMethod::NEIGHBOR_VALUES;

    // 处理选中的线条
    DarkLineProcessor::removeDarkLinesSequential(
        finalImage,
        selectedLines,
        removeInObject,
        removeIsolated,
        processorMethod);

    // 更新移除的线条记录
    m_lastRemovedLines.clear();
    for (size_t i = 0; i < selectedLines.size(); ++i) {
        m_lastRemovedLines.push_back(i + 1);
    }

    clearDetectedLines();
}

void ImageProcessor::removeDarkLinesSelective(
    bool removeInObject,
    bool removeIsolated,
    LineRemovalMethod method) {

    if (m_detectedLines.empty()) {
        m_detectedLines = detectDarkLines();
    }
    saveCurrentState();

    DarkLineProcessor::RemovalMethod processorMethod =
        (method == LineRemovalMethod::DIRECT_STITCH) ?
            DarkLineProcessor::RemovalMethod::DIRECT_STITCH :
            DarkLineProcessor::RemovalMethod::NEIGHBOR_VALUES;

    DarkLineProcessor::removeDarkLinesSelective(
        finalImage,
        m_detectedLines,
        removeInObject,
        removeIsolated,
        processorMethod);

    // 更新移除的线条记录
    m_lastRemovedLines.clear();
    for (size_t i = 0; i < m_detectedLines.size(); ++i) {
        bool shouldRemove = (m_detectedLines[i].inObject && removeInObject) ||
                            (!m_detectedLines[i].inObject && removeIsolated);
        if (shouldRemove) {
            m_lastRemovedLines.push_back(i + 1);
        }
    }

    clearDetectedLines();
}

InterlaceProcessor::InterlacedResult ImageProcessor::processEnhancedInterlacedSections(
    InterlaceProcessor::StartPoint lowEnergyStart,
    InterlaceProcessor::StartPoint highEnergyStart,
    InterlaceProcessor::MergeMethod mergeMethod) {

    saveCurrentState();

    auto result = InterlaceProcessor::processEnhancedInterlacedSections(
        finalImage,
        lowEnergyStart,
        highEnergyStart,
        mergeMethod
        );

    // Update final image with the combined and calibrated result
    finalImage = result.combinedImage;

    return result;
}

void ImageProcessor::resetToOriginal() {
    if (!originalImg.empty()) {
        finalImage = originalImg;
        while (!imageHistory.empty()) imageHistory.pop();
        while (!actionHistory.empty()) actionHistory.pop();
    }
}

void ImageProcessor::clearImage() {
    originalImg.clear();
    finalImage.clear();
    while (!imageHistory.empty()) imageHistory.pop();
    while (!actionHistory.empty()) actionHistory.pop();
}

void ImageProcessor::applyEdgeEnhancement(float strength) {
    if (finalImage.empty() || finalImage[0].empty()) return;

    saveCurrentState();

    int height = finalImage.size();
    int width = finalImage[0].size();

    // Create Sobel kernels
    std::vector<std::vector<int>> sobelX = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    std::vector<std::vector<int>> sobelY = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    // Create temporary image for edge detection
    std::vector<std::vector<uint16_t>> edgeImage(height, std::vector<uint16_t>(width));

    // Apply Sobel operators
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            int gx = 0;
            int gy = 0;

            // Apply kernels
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int pixelValue = finalImage[y + i][x + j];
                    gx += pixelValue * sobelX[i + 1][j + 1];
                    gy += pixelValue * sobelY[i + 1][j + 1];
                }
            }

            // Calculate gradient magnitude
            float magnitude = std::sqrt(gx * gx + gy * gy);
            edgeImage[y][x] = static_cast<uint16_t>(std::min(magnitude, 65535.0f));
        }
    }

    // Enhance edges by adding weighted edge information to original image
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float enhancedValue = finalImage[y][x] + strength * edgeImage[y][x];
            finalImage[y][x] = static_cast<uint16_t>(std::min(enhancedValue, 65535.0f));
        }
    }
}

InterlaceProcessor::InterlacedResult ImageProcessor::processEnhancedInterlacedSections(
    InterlaceProcessor::StartPoint lowEnergyStart,
    InterlaceProcessor::StartPoint highEnergyStart,
    const InterlaceProcessor::MergeParams& mergeParams)  // Change from MergeMethod to MergeParams
{
    saveCurrentState();

    return InterlaceProcessor::processEnhancedInterlacedSections(
        finalImage,
        lowEnergyStart,
        highEnergyStart,
        mergeParams
        );
}
