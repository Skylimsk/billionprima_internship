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
#include <QMessageBox>
#define _USE_MATH_DEFINES
#include <cmath>
#include "image_processor.h"
#include "adjustments.h"
#include "darkline_pointer.h"
#include "interlace.h"
#include "zoom.h"
#include <vector>
#include <utility>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#include "stb_image_write.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

ImageProcessor::ImageProcessor(QLabel* imageLabel)
    : imageLabel(imageLabel),
    m_imgData(nullptr),
    m_originalImg(nullptr),
    m_finalImage(nullptr),
    m_height(0),
    m_width(0),
    m_currentDarkLines(nullptr),
    regionSelected(false),
    rotationState(0),
    kernelSize(0) {
}

ImageProcessor::~ImageProcessor() {
    freeImage(m_imgData, m_height);
    freeImage(m_originalImg, m_height);
    freeImage(m_finalImage, m_height);
    if (m_currentDarkLines) {
        DarkLinePointerProcessor::destroyDarkLineArray(m_currentDarkLines);
    }
}

void ImageProcessor::saveCurrentState() {

    if (!validateState()) {
        qDebug() << "Cannot save invalid state";
        return;
    }

    ImageState currentState;
    currentState.image = cloneImage(m_finalImage, m_height, m_width);
    currentState.height = m_height;
    currentState.width = m_width;

    if (m_currentDarkLines) {
        currentState.darkLines = new DarkLineArray();
        DarkLinePointerProcessor::copyDarkLineArray(m_currentDarkLines, currentState.darkLines);
    }

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

double** ImageProcessor::matToDoublePtr(const cv::Mat& mat, int& height, int& width) {
    height = mat.rows;
    width = mat.cols;

    double** result = allocateImage(height, width);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            result[i][j] = static_cast<double>(mat.at<uint16_t>(i, j));
        }
    }
    return result;
}

void ImageProcessor::loadImage(const std::string& filePath) {
    try {
        cv::Mat img = cv::imread(filePath, cv::IMREAD_ANYDEPTH);
        if (img.empty()) {
            throw std::runtime_error("Failed to load image: " + filePath);
        }

        // Convert to 16-bit if necessary
        cv::Mat img16;
        if (img.depth() != CV_16U) {
            double scale = (img.depth() == CV_8U) ? 257.0 : 1.0;
            img.convertTo(img16, CV_16U, scale);
        } else {
            img16 = img;
        }

        // Handle multi-channel images by converting to grayscale
        cv::Mat grayscale;
        if (img16.channels() > 1) {
            cv::cvtColor(img16, grayscale, cv::COLOR_BGR2GRAY);
        } else {
            grayscale = img16;
        }

        // Free existing images
        freeImage(m_finalImage, m_height);
        freeImage(m_originalImg, m_height);

        // Store the truly original image first
        int origHeight, origWidth;
        m_originalImg = matToDoublePtr(grayscale, origHeight, origWidth);
        m_height = origHeight;
        m_width = origWidth;

        // Create working copy
        m_finalImage = cloneImage(m_originalImg, m_height, m_width);

        qDebug() << "Loaded image dimensions:" << m_width << "x" << m_height;

        // Clear history
        while (!imageHistory.empty()) imageHistory.pop();
        while (!actionHistory.empty()) actionHistory.pop();

    } catch (const std::exception& e) {
        qDebug() << "Error loading image:" << e.what();
        throw;
    }
}

// Helper function to check if a file is an image
bool ImageProcessor::isImageFile(const std::string& filePath) {
    std::string extension = filePath.substr(filePath.find_last_of(".") + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    const std::vector<std::string> imageExtensions = {
        "jpg", "jpeg", "png", "tiff", "tif", "bmp"
    };

    return std::find(imageExtensions.begin(), imageExtensions.end(), extension) != imageExtensions.end();
}

void ImageProcessor::setLastAction(const QString& action, const QString& parameters) {
    actionHistory.push({action, parameters});
    lastAction = actionHistory.top().toString();
}

CGData ImageProcessor::cropRegion(double** inputImage, int inputHeight, int inputWidth,
                                  int left, int top, int right, int bottom) {
    // Validate input parameters
    if (!inputImage || inputHeight <= 0 || inputWidth <= 0) {
        throw std::invalid_argument("Invalid input image");
    }

    // Normalize coordinates to ensure left <= right and top <= bottom
    int normalizedLeft = std::min(left, right);
    int normalizedRight = std::max(left, right);
    int normalizedTop = std::min(top, bottom);
    int normalizedBottom = std::max(top, bottom);

    // Clamp values to image boundaries
    normalizedLeft = std::max(0, normalizedLeft);
    normalizedTop = std::max(0, normalizedTop);
    normalizedRight = std::min(inputWidth, normalizedRight);
    normalizedBottom = std::min(inputHeight, normalizedBottom);

    // Calculate output dimensions
    int outputWidth = normalizedRight - normalizedLeft;
    int outputHeight = normalizedBottom - normalizedTop;

    // Validate crop region
    if (outputWidth <= 0 || outputHeight <= 0) {
        throw std::runtime_error("Invalid crop region dimensions");
    }

    CGData result;
    result.Row = outputHeight;
    result.Column = outputWidth;

    try {
        // Use the existing malloc2D template function
        malloc2D(result.Data, outputHeight, outputWidth);

        // Copy the cropped region
        for (int y = 0; y < outputHeight; y++) {
            for (int x = 0; x < outputWidth; x++) {
                result.Data[y][x] = inputImage[normalizedTop + y][normalizedLeft + x];
            }
        }

        // Calculate min and max values
        result.Min = UINT_MAX;
        result.Max = 0;
        for (int y = 0; y < outputHeight; y++) {
            for (int x = 0; x < outputWidth; x++) {
                unsigned int val = static_cast<unsigned int>(result.Data[y][x]);
                result.Min = std::min(result.Min, val);
                result.Max = std::max(result.Max, val);
            }
        }

        return result;

    } catch (const std::exception& e) {
        // Clean up on error
        if (result.Data) {
            for (int i = 0; i < result.Row; i++) {
                free(result.Data[i]);
            }
            free(result.Data);
        }
        throw std::runtime_error(std::string("Crop operation failed: ") + e.what());
    }
}

void ImageProcessor::processImage() {
    saveImageState();
    m_finalImage = cloneImage(m_originalImg, m_height, m_width);
}

void ImageProcessor::processYXAxis(double**& image, int height, int width, int linesToAvgY, int linesToAvgX) {
    saveCurrentState();
    if (!image || height <= 0 || width <= 0) {
        throw std::invalid_argument("Invalid input parameters for YX axis processing");
    }

    // Y-axis processing
    if (linesToAvgY > 0) {
        std::vector<double> referenceYMean(width, 0.0);

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < std::min(linesToAvgY, height); ++y) {
                referenceYMean[x] += image[y][x];
            }
            referenceYMean[x] = std::max(referenceYMean[x] / linesToAvgY, 1e-6);
        }

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double normalizedValue = image[y][x] / referenceYMean[x];
                normalizedValue = std::clamp(normalizedValue, 0.1, 10.0);
                image[y][x] = std::clamp(normalizedValue * 65535.0, 0.0, 65535.0);
            }
        }
    }

    // X-axis processing
    if (linesToAvgX > 0) {
        std::vector<double> referenceXMean(height, 0.0);

        for (int y = 0; y < height; ++y) {
            for (int x = width - linesToAvgX; x < width; ++x) {
                referenceXMean[y] += image[y][x];
            }
            referenceXMean[y] = std::max(referenceXMean[y] / linesToAvgX, 1e-6);
        }

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double normalizedValue = image[y][x] / referenceXMean[y];
                normalizedValue = std::clamp(normalizedValue, 0.1, 10.0);
                image[y][x] = std::clamp(normalizedValue * 65535.0, 0.0, 65535.0);
            }
        }
    }
}

void ImageProcessor::applyMedianFilter(double**& image, int height, int width, int filterKernelSize) {
    if (!image || height <= 0 || width <= 0 || filterKernelSize <= 0 || filterKernelSize % 2 == 0) {
        throw std::invalid_argument("Invalid input parameters for median filter");
    }

    // Allocate temporary buffer for output
    double** outputImage = nullptr;
    malloc2D(outputImage, height, width);

    int offset = filterKernelSize / 2;
    std::vector<double> window(filterKernelSize * filterKernelSize);

    try {
        // Process each pixel
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Handle border pixels
                if (y < offset || y >= height - offset || x < offset || x >= width - offset) {
                    outputImage[y][x] = image[y][x];
                    continue;
                }

                // Fill window with neighborhood values
                int idx = 0;
                for (int ky = -offset; ky <= offset; ++ky) {
                    for (int kx = -offset; kx <= offset; ++kx) {
                        window[idx++] = image[y + ky][x + kx];
                    }
                }

                // Sort window and take median
                std::sort(window.begin(), window.end());
                outputImage[y][x] = window[window.size() / 2];
            }
        }

        // Free input image and swap with output
        for (int i = 0; i < height; ++i) {
            free(image[i]);
        }
        free(image);
        image = outputImage;

    } catch (const std::exception& e) {
        // Clean up on error
        if (outputImage) {
            for (int i = 0; i < height; ++i) {
                free(outputImage[i]);
            }
            free(outputImage);
        }
        throw;
    }
}

void ImageProcessor::applyHighPassFilter(double**& image, int height, int width) {
    if (!image || height <= 0 || width <= 0) {
        throw std::invalid_argument("Invalid input parameters for high pass filter");
    }

    // Allocate output buffer
    double** outputImage = nullptr;
    malloc2D(outputImage, height, width);

    // Define high-pass filter kernel
    const int kernelSize = 3;
    const double kernel[3][3] = {
        {0, -1, 0},
        {-1, 5, -1},
        {0, -1, 0}
    };

    const float scalingFactor = 1.5f;
    const int halfSize = kernelSize / 2;

    try {
        // Process each pixel
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Handle border pixels
                if (y < halfSize || y >= height - halfSize || x < halfSize || x >= width - halfSize) {
                    outputImage[y][x] = image[y][x];
                    continue;
                }

                double newValue = 0.0;
                for (int ky = -halfSize; ky <= halfSize; ++ky) {
                    for (int kx = -halfSize; kx <= halfSize; ++kx) {
                        newValue += image[y + ky][x + kx] * kernel[ky + halfSize][kx + halfSize];
                    }
                }

                newValue *= scalingFactor;
                outputImage[y][x] = std::clamp(newValue, 0.0, 65535.0);
            }
        }

        // Free input image and swap with output
        for (int i = 0; i < height; ++i) {
            free(image[i]);
        }
        free(image);
        image = outputImage;

    } catch (const std::exception& e) {
        // Clean up on error
        if (outputImage) {
            for (int i = 0; i < height; ++i) {
                free(outputImage[i]);
            }
            free(outputImage);
        }
        throw;
    }
}

CLAHEProcessor::PerformanceMetrics ImageProcessor::getLastPerformanceMetrics() const {
    return claheProcessor.getLastPerformanceMetrics();
}

void ImageProcessor::rotateImage(int angle) {
    qDebug() << "\n=== Starting Rotation Operation ===";
    qDebug() << "Current history size:" << imageHistory.size();

    if (!m_finalImage) {
        qDebug() << "No image to rotate";
        return;
    }

    saveCurrentState();  // This should increment history
    qDebug() << "History size after saving state:" << imageHistory.size();

    int newHeight, newWidth;
    if (angle == 90 || angle == 270) {
        newHeight = m_width;
        newWidth = m_height;
    } else {
        newHeight = m_height;
        newWidth = m_width;
    }

    double** rotated = new double*[newHeight];
    for(int i = 0; i < newHeight; i++) {
        rotated[i] = new double[newWidth];
    }

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int newX, newY;
            if (angle == 90) {
                newX = y;
                newY = m_width - x - 1;
            } else if (angle == 180) {
                newX = m_width - x - 1;
                newY = m_height - y - 1;
            } else if (angle == 270) {
                newX = m_height - y - 1;
                newY = x;
            } else {
                newX = x;
                newY = y;
            }
            rotated[newY][newX] = m_finalImage[y][x];
        }
    }

    // Free old image
    for(int i = 0; i < m_height; i++) {
        delete[] m_finalImage[i];
    }
    delete[] m_finalImage;

    m_finalImage = rotated;
    m_height = newHeight;
    m_width = newWidth;
}


void ImageProcessor::stretchImageY(double**& image, int& height, int width, float yStretchFactor) {
    saveCurrentState();
    if (!image || height <= 0 || width <= 0 || yStretchFactor <= 0) {
        throw std::invalid_argument("Invalid input parameters for Y-stretch");
    }

    int newHeight = static_cast<int>(height * yStretchFactor);

    // Allocate memory for stretched image
    double** stretchedImage = nullptr;
    malloc2D(stretchedImage, newHeight, width);

    try {
        // Perform vertical stretching
        for (int y = 0; y < newHeight; ++y) {
            float srcY = y / yStretchFactor;
            int yLow = static_cast<int>(srcY);
            int yHigh = std::min(yLow + 1, height - 1);
            float weight = srcY - yLow;

            for (int x = 0; x < width; ++x) {
                stretchedImage[y][x] = (1 - weight) * image[yLow][x] + weight * image[yHigh][x];
            }
        }

        // Free original image
        for (int i = 0; i < height; ++i) {
            free(image[i]);
        }
        free(image);

        // Update image pointer and height
        image = stretchedImage;
        height = newHeight;

    } catch (const std::exception& e) {
        // Clean up on error
        if (stretchedImage) {
            for (int i = 0; i < newHeight; ++i) {
                free(stretchedImage[i]);
            }
            free(stretchedImage);
        }
        throw;
    }
}

void ImageProcessor::stretchImageX(double**& image, int height, int& width, float xStretchFactor) {
    saveCurrentState();
    if (!image || height <= 0 || width <= 0 || xStretchFactor <= 0) {
        throw std::invalid_argument("Invalid input parameters for X-stretch");
    }

    int newWidth = static_cast<int>(width * xStretchFactor);

    // Allocate memory for stretched image
    double** stretchedImage = nullptr;
    malloc2D(stretchedImage, height, newWidth);

    try {
        // Perform horizontal stretching
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                float srcX = x / xStretchFactor;
                int xLow = static_cast<int>(srcX);
                int xHigh = std::min(xLow + 1, width - 1);
                float weight = srcX - xLow;

                stretchedImage[y][x] = (1 - weight) * image[y][xLow] + weight * image[y][xHigh];
            }
        }

        // Free original image
        for (int i = 0; i < height; ++i) {
            free(image[i]);
        }
        free(image);

        // Update image pointer and width
        image = stretchedImage;
        width = newWidth;

    } catch (const std::exception& e) {
        // Clean up on error
        if (stretchedImage) {
            for (int i = 0; i < height; ++i) {
                free(stretchedImage[i]);
            }
            free(stretchedImage);
        }
        throw;
    }
}

void ImageProcessor::distortImage(double**& image, int height, int width, float distortionFactor, const std::string& direction) {
    saveCurrentState();
    if (!image || height <= 0 || width <= 0) {
        throw std::invalid_argument("Invalid input parameters for distortion");
    }

    // Allocate memory for distorted image
    double** distortedImage = nullptr;
    malloc2D(distortedImage, height, width);

    try {
        int centerX = width / 2;
        int centerY = height / 2;

        // Copy original image first
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                distortedImage[y][x] = image[y][x];
            }
        }

        // Apply distortion based on direction
        if (direction == "Left") {
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < centerX; ++x) {
                    int newX = static_cast<int>(x + distortionFactor * std::sin(M_PI * x / centerX));
                    newX = std::min(std::max(newX, 0), centerX - 1);
                    distortedImage[y][x] = image[y][newX];
                }
            }
        }
        else if (direction == "Right") {
            for (int y = 0; y < height; ++y) {
                for (int x = centerX; x < width; ++x) {
                    int newX = static_cast<int>(x + distortionFactor * std::sin(M_PI * (x - centerX) / (width - centerX)));
                    newX = std::min(std::max(newX, centerX), width - 1);
                    distortedImage[y][newX] = image[y][x];
                }
            }
        }
        else if (direction == "Top") {
            for (int y = centerY - 1; y >= 0; --y) {
                for (int x = 0; x < width; ++x) {
                    int newY = static_cast<int>(y + distortionFactor * std::sin(M_PI * (y - centerY) / centerY));
                    newY = std::min(std::max(newY, 0), centerY - 1);
                    distortedImage[newY][x] = image[y][x];
                }
            }
        }
        else if (direction == "Bottom") {
            for (int y = centerY; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    int newY = static_cast<int>(y + distortionFactor * std::sin(M_PI * (y - centerY) / (height - centerY)));
                    newY = std::min(std::max(newY, centerY), height - 1);
                    distortedImage[newY][x] = image[y][x];
                }
            }
        }

        // Free original image
        for (int i = 0; i < height; ++i) {
            free(image[i]);
        }
        free(image);

        // Update image pointer
        image = distortedImage;

    } catch (const std::exception& e) {
        // Clean up on error
        if (distortedImage) {
            for (int i = 0; i < height; ++i) {
                free(distortedImage[i]);
            }
            free(distortedImage);
        }
        throw;
    }
}

void ImageProcessor::addPadding(double**& image, int& height, int& width, int paddingSize) {
    saveCurrentState();
    if (!image || height <= 0 || width <= 0 || paddingSize < 0) {
        throw std::invalid_argument("Invalid input parameters for padding");
    }

    int newHeight = height + 2 * paddingSize;
    int newWidth = width + 2 * paddingSize;

    // Allocate memory for padded image
    double** paddedImage = nullptr;
    malloc2D(paddedImage, newHeight, newWidth);

    try {
        // Fill padding with maximum value (65535.0)
        for (int y = 0; y < newHeight; ++y) {
            for (int x = 0; x < newWidth; ++x) {
                paddedImage[y][x] = 65535.0;
            }
        }

        // Copy original image to center
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                paddedImage[y + paddingSize][x + paddingSize] = image[y][x];
            }
        }

        // Free original image
        for (int i = 0; i < height; ++i) {
            free(image[i]);
        }
        free(image);

        // Update image pointer and dimensions
        image = paddedImage;
        height = newHeight;
        width = newWidth;

    } catch (const std::exception& e) {
        // Clean up on error
        if (paddedImage) {
            for (int i = 0; i < newHeight; ++i) {
                free(paddedImage[i]);
            }
            free(paddedImage);
        }
        throw;
    }
}

void ImageProcessor::saveImageState() {
    ImageState currentState;
    currentState.image = cloneImage(m_finalImage, m_height, m_width);
    currentState.height = m_height;
    currentState.width = m_width;
    currentState.darkLines = m_currentDarkLines ? new DarkLineArray(*m_currentDarkLines) : nullptr;
    imageHistory.push(currentState);
}

void ImageProcessor::updateAndSaveFinalImage(double** newImage, int height, int width, bool saveCurrentStateFlag) {
    if (saveCurrentStateFlag) {
        saveCurrentState();
    }

    // Free existing image
    if (m_finalImage) {
        for (int i = 0; i < m_height; i++) {
            delete[] m_finalImage[i];
        }
        delete[] m_finalImage;
    }

    // Allocate and copy new image
    m_height = height;
    m_width = width;
    m_finalImage = new double*[height];
    for (int i = 0; i < height; i++) {
        m_finalImage[i] = new double[width];
        std::copy(newImage[i], newImage[i] + width, m_finalImage[i]);
    }
}

void ImageProcessor::clearImage() {
    freeImage(m_imgData, m_height);
    freeImage(m_originalImg, m_height);
    freeImage(m_finalImage, m_height);

    if (m_currentDarkLines) {
        DarkLinePointerProcessor::destroyDarkLineArray(m_currentDarkLines);
        m_currentDarkLines = nullptr;
    }

    m_height = 0;
    m_width = 0;

    // Clear all history states
    while (!imageHistory.empty()) {
        imageHistory.pop();
    }
    while (!actionHistory.empty()) {
        actionHistory.pop();
    }
}

void ImageProcessor::resetToOriginal() {
    if (m_originalImg) {
        // Free current working image
        freeImage(m_finalImage, m_height);

        // Create fresh copy from original
        m_finalImage = cloneImage(m_originalImg, m_height, m_width);

        // Clear any dark lines
        if (m_currentDarkLines) {
            DarkLinePointerProcessor::destroyDarkLineArray(m_currentDarkLines);
            m_currentDarkLines = nullptr;
        }

        // Clear all history states
        while (!imageHistory.empty()) {
            imageHistory.pop();
        }
        while (!actionHistory.empty()) {
            actionHistory.pop();
        }

        qDebug() << "Image reset to original state and history cleared";
    } else {
        qDebug() << "No original image available for reset";
    }
}

void ImageProcessor::applyEdgeEnhancement(double**& image, int height, int width, float strength) {
    if (!image || height <= 0 || width <= 0) {
        throw std::invalid_argument("Invalid input parameters for edge enhancement");
    }

    // Allocate temporary edge image
    double** edgeImage = nullptr;
    double** outputImage = nullptr;
    malloc2D(edgeImage, height, width);
    malloc2D(outputImage, height, width);

    // Define Sobel kernels
    const double sobelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    const double sobelY[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    try {
        // Apply Sobel operators
        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                double gx = 0.0, gy = 0.0;

                // Apply kernels
                for (int i = -1; i <= 1; ++i) {
                    for (int j = -1; j <= 1; ++j) {
                        double pixelValue = image[y + i][x + j];
                        gx += pixelValue * sobelX[i + 1][j + 1];
                        gy += pixelValue * sobelY[i + 1][j + 1];
                    }
                }

                // Calculate gradient magnitude
                edgeImage[y][x] = std::sqrt(gx * gx + gy * gy);
            }
        }

        // Enhance edges
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double enhancedValue = image[y][x] + strength * edgeImage[y][x];
                outputImage[y][x] = std::clamp(enhancedValue, 0.0, 65535.0);
            }
        }

        // Clean up temporary edge image
        for (int i = 0; i < height; ++i) {
            free(edgeImage[i]);
        }
        free(edgeImage);

        // Free input image and swap with output
        for (int i = 0; i < height; ++i) {
            free(image[i]);
        }
        free(image);
        image = outputImage;

    } catch (const std::exception& e) {
        // Clean up on error
        if (edgeImage) {
            for (int i = 0; i < height; ++i) {
                free(edgeImage[i]);
            }
            free(edgeImage);
        }
        if (outputImage) {
            for (int i = 0; i < height; ++i) {
                free(outputImage[i]);
            }
            free(outputImage);
        }
        throw;
    }
}

void ImageProcessor::clearDetectedLines() {
    if (m_currentDarkLines) {
        DarkLinePointerProcessor::destroyDarkLineArray(m_currentDarkLines);
        m_currentDarkLines = nullptr;
    }
}

double** ImageProcessor::cloneImage(double** src, int height, int width) {
    if (!src || height <= 0 || width <= 0) return nullptr;

    double** dest = new double*[height];
    for (int i = 0; i < height; i++) {
        dest[i] = new double[width];
        memcpy(dest[i], src[i], width * sizeof(double));
    }
    return dest;
}

void ImageProcessor::freeImage(double**& image, int height) {
    if (image) {
        for (int i = 0; i < height; i++) {
            delete[] image[i];
        }
        delete[] image;
        image = nullptr;
    }
}

double** ImageProcessor::allocateImage(int height, int width) {
    if (height <= 0 || width <= 0) return nullptr;

    double** image = new double*[height];
    for (int i = 0; i < height; i++) {
        image[i] = new double[width];
    }
    return image;
}

bool ImageProcessor::validateState() const {
    if (!m_finalImage || m_height <= 0 || m_width <= 0) {
        qDebug() << "Invalid state detected";
        return false;
    }
    return true;
}

void ImageProcessor::undo() {
    qDebug() << "\n=== Attempting Undo Operation in ImageProcessor ===";
    qDebug() << "Current history size:" << imageHistory.size();
    qDebug() << "Current image dimensions:" << m_width << "x" << m_height;

    if (imageHistory.empty()) {
        qDebug() << "No history available to undo";
        return;
    }

    try {
        // Get the last state from history
        ImageState lastState = imageHistory.top();  // Create a copy instead of moving
        imageHistory.pop();

        qDebug() << "Restoring state with dimensions:" << lastState.width << "x" << lastState.height;
        qDebug() << "Remaining history size:" << imageHistory.size();

        // Free current image if it exists
        if (m_finalImage) {
            qDebug() << "Freeing current image";
            for (int i = 0; i < m_height; i++) {
                if (m_finalImage[i]) {  // Add null check for each row
                    delete[] m_finalImage[i];
                }
            }
            delete[] m_finalImage;
            m_finalImage = nullptr;  // Set to nullptr after deletion
        }

        // Free current dark lines if they exist
        if (m_currentDarkLines) {
            qDebug() << "Freeing current dark lines";
            DarkLinePointerProcessor::destroyDarkLineArray(m_currentDarkLines);
            m_currentDarkLines = nullptr;  // Set to nullptr after deletion
        }

        // Create a deep copy of the previous state's image
        m_finalImage = allocateImage(lastState.height, lastState.width);
        for (int i = 0; i < lastState.height; i++) {
            std::memcpy(m_finalImage[i], lastState.image[i], lastState.width * sizeof(double));
        }

        // Update dimensions
        m_height = lastState.height;
        m_width = lastState.width;

        // Handle dark lines (if they exist)
        if (lastState.darkLines) {
            m_currentDarkLines = new DarkLineArray();
            DarkLinePointerProcessor::copyDarkLineArray(lastState.darkLines, m_currentDarkLines);
        } else {
            m_currentDarkLines = nullptr;
        }

        qDebug() << "State restored - New dimensions:" << m_width << "x" << m_height;

        // Update action history if available
        if (!actionHistory.empty()) {
            actionHistory.pop();
            qDebug() << "Action history updated";
        }

        qDebug() << "Undo operation completed successfully";
    }
    catch (const std::exception& e) {
        qDebug() << "Exception during undo operation:" << e.what();
        // Ensure cleanup in case of exception
        if (m_finalImage == nullptr) {
            m_height = 0;
            m_width = 0;
        }
        throw;
    }
}
