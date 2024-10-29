#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <QDebug>
#include <QRect>
#include <fstream>
#include <thread>
#include <algorithm>

#include "image_processor.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include "stb_image_write.h"
#pragma GCC diagnostic pop

ImageProcessor::ImageProcessor(QLabel* imageLabel)
    : imageLabel(imageLabel), regionSelected(false), rotationState(0), kernelSize(0) {
    // Initialize other members as needed
}

void ImageProcessor::saveCurrentState() {
    imageHistory.push(finalImage);
}

QString ImageProcessor::getCurrentAction() const {
    if (!actionHistory.empty()) {
        return actionHistory.top();
    }
    return "No actions";
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

void ImageProcessor::setLastAction(const QString& action) {
    actionHistory.push(action);
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
    if (!imageHistory.empty()) {
        finalImage = imageHistory.top();
        imageHistory.pop();

        if (!actionHistory.empty()) {
            actionHistory.pop(); // Remove the action we're reverting
        }

        return getCurrentAction(); // Return the new current action
    }
    return QString();  // Return an empty QString if no action to revert
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

    int height = image.size();
    int width = image[0].size();

    // Y-axis processing
    std::vector<float> referenceYMean(width, 0.0f);
    for (int y = 0; y < std::min(linesToAvgY, height); ++y) {
        for (int x = 0; x < width; ++x) {
            referenceYMean[x] += image[y][x];
        }
    }
    for (int x = 0; x < width; ++x) {
        referenceYMean[x] /= linesToAvgY;
    }

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float normalizedValue = static_cast<float>(image[y][x]) / referenceYMean[x];
            image[y][x] = static_cast<uint16_t>(std::min(normalizedValue * 65535.0f, 65535.0f));
        }
    }

    // X-axis processing
    std::vector<float> referenceXMean(height, 0.0f);
    for (int y = 0; y < height; ++y) {
        for (int x = width - linesToAvgX; x < width; ++x) {
            referenceXMean[y] += image[y][x];
        }
        referenceXMean[y] /= linesToAvgX;
    }

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < height; ++y) {
            float normalizedValue = static_cast<float>(image[y][x]) / referenceXMean[y];
            image[y][x] = static_cast<uint16_t>(std::min(normalizedValue * 65535.0f, 65535.0f));
        }
    }
}

void ImageProcessor::processAndMergeImageParts() {

    saveCurrentState();

    int height = originalImg.size();
    int width = originalImg[0].size();
    int quarterWidth = width / 4;

    // Split image into four parts using std::copy for efficiency
    std::vector<std::vector<uint16_t>> leftLeft(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> leftRight(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightLeft(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightRight(height, std::vector<uint16_t>(quarterWidth));

    for (int y = 0; y < height; ++y) {
        std::copy(originalImg[y].begin(), originalImg[y].begin() + quarterWidth, leftLeft[y].begin());
        std::copy(originalImg[y].begin() + quarterWidth, originalImg[y].begin() + 2 * quarterWidth, leftRight[y].begin());
        std::copy(originalImg[y].begin() + 2 * quarterWidth, originalImg[y].begin() + 3 * quarterWidth, rightLeft[y].begin());
        std::copy(originalImg[y].begin() + 3 * quarterWidth, originalImg[y].end(), rightRight[y].begin());
    }



    // Process each part for noise detection and removal
    auto processPart = [&](std::vector<std::vector<uint16_t>>& part, const std::vector<std::vector<uint16_t>>& ) {
        // Apply stretch based on rotation state
        if (rotationState == 1 || rotationState == 3) {
            stretchImageX(part, params.yStretchFactor);  // Stretch along X-axis for 90 or 270-degree rotations
        } else {
            stretchImageY(part, params.yStretchFactor);  // Stretch along Y-axis otherwise
        }
    };

    // Process the split parts using neighbouring parts
    processPart(leftLeft, leftRight);
    processPart(leftRight, rightLeft);
    processPart(rightLeft, rightRight);
    processPart(rightRight, leftLeft);

    // Combine parts using a weighted average to preserve details
    auto weightedAverageImages = [](const std::vector<std::vector<uint16_t>>& img1,
                                    const std::vector<std::vector<uint16_t>>& img2,
                                    const std::vector<std::vector<uint16_t>>& img3,
                                    const std::vector<std::vector<uint16_t>>& img4) {
        int height = img1.size();
        int width = img1[0].size();
        std::vector<std::vector<uint16_t>> result(height, std::vector<uint16_t>(width));

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                result[y][x] = static_cast<uint16_t>((
                                                         static_cast<uint32_t>(img1[y][x]) +
                                                         static_cast<uint32_t>(img2[y][x]) +
                                                         static_cast<uint32_t>(img3[y][x]) +
                                                         static_cast<uint32_t>(img4[y][x])) / 4);  // Weighted average of all four parts
            }
        }

        return result;
    };

    // Merge the processed parts into the final image
    finalImage = weightedAverageImages(leftLeft, leftRight, rightLeft, rightRight);

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
    cv::Mat resultImage;
    cv::Mat inputImage8bit = convertTo8Bit(inputImage);

    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
    clahe->apply(inputImage8bit, resultImage);

    return convertTo16Bit(resultImage);
}

cv::Mat ImageProcessor::convertTo8Bit(const cv::Mat& input) {
    cv::Mat output;
    input.convertTo(output, CV_8UC1, 255.0 / 65535.0);
    return output;
}

cv::Mat ImageProcessor::convertTo16Bit(const cv::Mat& input) {
    cv::Mat output;
    input.convertTo(output, CV_16UC1, 65535.0 / 255.0);
    return output;
}

// Function implementations
void ImageProcessor::adjustGammaForRegion(std::vector<std::vector<uint16_t>>& img, float gamma, int startY, int endY, int startX, int endX) {
    float invGamma = 1.0f / gamma;
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            float normalized = img[y][x] / 65535.0f; // Normalize the pixel value
            float corrected = std::pow(normalized, invGamma); // Apply Gamma correction
            img[y][x] = static_cast<uint16_t>(corrected * 65535.0f); // Convert back to pixel value
        }
    }
}

void ImageProcessor::processContrastChunk(int top, int bottom, int left, int right, float contrastFactor, std::vector<std::vector<uint16_t>>& img) {
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;
    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            float pixel = static_cast<float>(img[y][x]);
            img[y][x] = static_cast<uint16_t>(std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0f, MAX_PIXEL_VALUE));
        }
    }
}

void ImageProcessor::processSharpenRegionChunk(int startY, int endY, int left, int right, std::vector<std::vector<uint16_t>>& img, float sharpenStrength) {
    for (int y = startY; y < endY; ++y) {
        for (int x = left; x < right; ++x) {
            img[y][x] = std::clamp(static_cast<int>(img[y][x] * sharpenStrength), 0, 65535);
        }
    }
}

void ImageProcessor::processSharpenChunk(int startY, int endY, int width, std::vector<std::vector<uint16_t>>& img, const std::vector<std::vector<uint16_t>>& tempImg, float sharpenStrength) {
    float kernel[3][3] = {
        { 0, -1 * sharpenStrength,  0 },
        { -1 * sharpenStrength, 1 + 4 * sharpenStrength, -1 * sharpenStrength },
        { 0, -1 * sharpenStrength,  0 }
    };
    for (std::vector<std::vector<uint16_t>>::size_type y = startY; y < static_cast<std::vector<std::vector<uint16_t>>::size_type>(endY); ++y) {
        for (std::vector<uint16_t>::size_type x = 1; x < static_cast<std::vector<uint16_t>::size_type>(width) - 1; ++x) {
            // Apply convolution only if within bounds
            if (y > 0 && y < tempImg.size() - 1 && x > 0 && x < tempImg[0].size() - 1) {
                float newPixelValue = 0;
                // Apply kernel to neighbours
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        newPixelValue += kernel[ky + 1][kx + 1] * tempImg[y + ky][x + kx];
                    }
                }
                // Clamp the new value and assign to the output image
                img[y][x] = static_cast<uint16_t>(std::clamp(newPixelValue, 0.0f, 65535.0f));
            }
        }
    }
}

void ImageProcessor::adjustContrast(std::vector<std::vector<uint16_t>>& img, float contrastFactor) {
    saveCurrentState();
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;
    int height = img.size();
    int width = img[0].size();

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;

        threads.emplace_back([=, &img]() {
            for (int y = startY; y < endY; ++y) {
                for (int x = 0; x < width; ++x) {
                    float pixel = static_cast<float>(img[y][x]);
                    img[y][x] = static_cast<uint16_t>(std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0f, MAX_PIXEL_VALUE));
                }
            }
        });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ImageProcessor::adjustGammaOverall(std::vector<std::vector<uint16_t>>& img, float gamma) {
    saveCurrentState();
    int height = img.size();
    int width = img[0].size();

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int blockHeight = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * blockHeight;
        int endY = (i == numThreads - 1) ? height : startY + blockHeight;
        threads.emplace_back(adjustGammaForRegion, std::ref(img), gamma, startY, endY, 0, width);
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void ImageProcessor::sharpenImage(std::vector<std::vector<uint16_t>>& img, float sharpenStrength) {
    saveCurrentState();
    int height = img.size();
    int width = img[0].size();
    std::vector<std::vector<uint16_t>> tempImg = img;

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;
        threads.emplace_back(processSharpenChunk, startY, endY, width, std::ref(img), std::ref(tempImg), sharpenStrength);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void ImageProcessor::processRegion(const QRect& region, std::function<void(int, int)> operation) {
    QRect normalizedRegion = region.normalized();
    int left = std::max(0, normalizedRegion.left());
    int top = std::max(0, normalizedRegion.top());
    int right = std::min(static_cast<int>(finalImage[0].size()), normalizedRegion.right() + 1);
    int bottom = std::min(static_cast<int>(finalImage.size()), normalizedRegion.bottom() + 1);

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            operation(x, y);
        }
    }
}

void ImageProcessor::adjustGammaForSelectedRegion(float gamma, const QRect& region) {
    saveCurrentState();
    float invGamma = 1.0f / gamma;
    processRegion(region, [this, invGamma](int x, int y) {
        float normalized = finalImage[y][x] / 65535.0f;
        float corrected = std::pow(normalized, invGamma);
        finalImage[y][x] = static_cast<uint16_t>(corrected * 65535.0f);
    });
}

void ImageProcessor::applySharpenToRegion(float sharpenStrength, const QRect& region) {
    saveCurrentState();
    std::vector<std::vector<uint16_t>> tempImage = finalImage;
    processRegion(region, [this, &tempImage, sharpenStrength](int x, int y) {
        auto width = static_cast<int>(finalImage[0].size());
        auto height = static_cast<int>(finalImage.size());
        if (x > 0 && x < width - 1 && y > 0 && y < height - 1) {
            int sum = 5 * tempImage[y][x] - tempImage[y-1][x] - tempImage[y+1][x] - tempImage[y][x-1] - tempImage[y][x+1];
            finalImage[y][x] = static_cast<uint16_t>(std::clamp(static_cast<float>(sum) * sharpenStrength + static_cast<float>(tempImage[y][x]), 0.0f, 65535.0f));
        }
    });
}

void ImageProcessor::applyContrastToRegion(float contrastFactor, const QRect& region) {
    saveCurrentState();
    float midGray = 32767.5f;

    processRegion(region, [this, contrastFactor, midGray](int x, int y) {
        float pixel = static_cast<float>(finalImage[y][x]);
        finalImage[y][x] = static_cast<uint16_t>(std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0f, 65535.0f));
    });
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
            stretchedImg[y][x] = static_cast<uint16_t>((1 - weight) * img[yLow][x] + weight * img[yHigh][x]);
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

            stretchedImg[y][x] = static_cast<uint16_t>((1 - weight) * img[y][xLow] + weight * img[y][xHigh]);
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
    imageHistory.push(finalImage);
}

void ImageProcessor::updateAndSaveFinalImage(const std::vector<std::vector<uint16_t>>& newImage) {
    saveImageState();
    finalImage = newImage;
}
