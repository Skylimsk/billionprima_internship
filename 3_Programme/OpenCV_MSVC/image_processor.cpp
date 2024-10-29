#ifdef _MSC_VER
#pragma warning(disable: 4068) // unknown pragma
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4244) // conversion from 'double' to 'float', possible loss of data
#pragma warning(disable: 4458) // declaration hides class member
#pragma warning(disable: 4005) // macro redefinition
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4702) // unreachable code
#pragma warning(disable: 4456) // declaration hides previous local declaration
#pragma warning(disable: 4389) // signed/unsigned mismatch
#pragma warning(disable: 4996) // This function or variable may be unsafe
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
#define _USE_MATH_DEFINES
#include <cmath>
#include "image_processor.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include "stb_image_write.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

ImageProcessor::ImageProcessor(QLabel* imageLabel)
    : imageLabel(imageLabel),
    regionSelected(false),
    rotationState(0),
    kernelSize(0) {
}

void ImageProcessor::saveCurrentState() {
    imageHistory.push(finalImage);
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
        finalImage = imageHistory.top();
        imageHistory.pop();

        ActionRecord currentAction = actionHistory.top();
        actionHistory.pop();

        // If there are more actions in history, get the previous one
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
    saveCurrentState();
    preProcessedImage = finalImage; // Save the state before applying CLAHE
    hasCLAHEBeenApplied = true;
    return claheProcessor.applyCLAHE(inputImage, clipLimit, tileSize);
}

cv::Mat ImageProcessor::applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize) {
    saveCurrentState();
    preProcessedImage = finalImage; // Save the state before applying CLAHE
    hasCLAHEBeenApplied = true;
    return claheProcessor.applyCLAHE_CPU(inputImage, clipLimit, tileSize);
}

void ImageProcessor::applyThresholdCLAHE_GPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize) {
    saveCurrentState();
    if (hasCLAHEBeenApplied) {
        claheProcessor.applyThresholdCLAHE_GPU(preProcessedImage, threshold, clipLimit, tileSize);
    } else {
        claheProcessor.applyThresholdCLAHE_GPU(finalImage, threshold, clipLimit, tileSize);
    }
    finalImage = matToVector(vectorToMat(finalImage));
    hasCLAHEBeenApplied = false;
}

void ImageProcessor::applyThresholdCLAHE_CPU(uint16_t threshold, double clipLimit, const cv::Size& tileSize) {
    saveCurrentState();
    if (hasCLAHEBeenApplied) {
        claheProcessor.applyThresholdCLAHE_CPU(preProcessedImage, threshold, clipLimit, tileSize);
    } else {
        claheProcessor.applyThresholdCLAHE_CPU(finalImage, threshold, clipLimit, tileSize);
    }
    finalImage = claheProcessor.matToVector(claheProcessor.vectorToMat(finalImage));
    hasCLAHEBeenApplied = false;
}

CLAHEProcessor::PerformanceMetrics ImageProcessor::getLastPerformanceMetrics() const {
    return claheProcessor.getLastPerformanceMetrics();
}

std::vector<ImageProcessor::DarkLine> ImageProcessor::detectDarkLines(
    uint16_t brightThreshold, uint16_t darkThreshold, int minLineLength) {

    saveCurrentState();
    std::vector<DarkLine> detectedLines;

    int height = finalImage.size();
    int width = finalImage[0].size();

    // Step 1: Find bright regions with improved detection
    auto brightRegions = findBrightRegions(brightThreshold);

    // Step 2: Create a binary mask for dark pixels
    std::vector<std::vector<bool>> darkPixels(height, std::vector<bool>(width, false));
    std::vector<std::vector<bool>> visited(height, std::vector<bool>(width, false));

    // Improved dark pixel detection with consideration for thicker lines
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (brightRegions[y][x] && finalImage[y][x] <= darkThreshold) {
                darkPixels[y][x] = true;
            }
        }
    }

    // Direction vectors for 8-connected neighborhood
    const int dx[] = {-1, 1, 0, 0, -1, -1, 1, 1};
    const int dy[] = {0, 0, -1, 1, -1, 1, -1, 1};

    // Step 3: Enhanced line detection algorithm
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (!darkPixels[y][x] || visited[y][x]) {
                continue;
            }

            // Initialize line properties
            std::vector<std::pair<int, int>> linePixels;
            std::queue<std::pair<int, int>> pixelQueue;

            pixelQueue.push({x, y});
            visited[y][x] = true;
            linePixels.push_back({x, y});

            // Breadth-first search for connected dark pixels
            while (!pixelQueue.empty()) {
                auto [curX, curY] = pixelQueue.front();
                pixelQueue.pop();

                for (int dir = 0; dir < 8; ++dir) {
                    int newX = curX + dx[dir];
                    int newY = curY + dy[dir];

                    if (newX >= 0 && newX < width && newY >= 0 && newY < height &&
                        !visited[newY][newX] && darkPixels[newY][newX]) {
                        visited[newY][newX] = true;
                        pixelQueue.push({newX, newY});
                        linePixels.push_back({newX, newY});
                    }
                }
            }

            // Process detected line segment if it meets minimum length requirement
            if (linePixels.size() >= static_cast<size_t>(std::max(1, minLineLength))) {
                // Find bounding box of the line
                int minX = width, minY = height, maxX = 0, maxY = 0;
                for (const auto& pixel : linePixels) {
                    minX = std::min(minX, pixel.first);
                    maxX = std::max(maxX, pixel.first);
                    minY = std::min(minY, pixel.second);
                    maxY = std::max(maxY, pixel.second);
                }

                // Calculate line thickness using improved method
                int thickness = calculateLineThickness(linePixels, minX, maxX, minY, maxY);

                // Create line segment
                detectedLines.push_back({
                    cv::Point(minX, minY),
                    cv::Point(maxX, maxY),
                    thickness
                });
            }
        }
    }

    // Refine and merge detected lines
    refineDarkLineDetection(detectedLines);

    return detectedLines;
}

int ImageProcessor::calculateLineThickness(
    const std::vector<std::pair<int, int>>& linePixels,
    int minX, int maxX, int minY, int maxY) {

    int dx = maxX - minX;
    int dy = maxY - minY;

    if (dx > dy) {
        // Primarily horizontal line
        std::vector<int> heightProfile(dx + 1, 0);
        for (const auto& pixel : linePixels) {
            int x = pixel.first - minX;
            if (x >= 0 && x < heightProfile.size()) {
                heightProfile[x]++;
            }
        }
        return *std::max_element(heightProfile.begin(), heightProfile.end());
    } else {
        // Primarily vertical line
        std::vector<int> widthProfile(dy + 1, 0);
        for (const auto& pixel : linePixels) {
            int y = pixel.second - minY;
            if (y >= 0 && y < widthProfile.size()) {
                widthProfile[y]++;
            }
        }
        return *std::max_element(widthProfile.begin(), widthProfile.end());
    }
}

bool ImageProcessor::isInBrightRegion(const std::vector<std::vector<uint16_t>>& image,
                                      int x, int y, uint16_t brightThreshold) {
    const int windowSize = 5; // 考虑周围区域
    int brightPixels = 0;
    int totalPixels = 0;

    for (int dy = -windowSize/2; dy <= windowSize/2; ++dy) {
        for (int dx = -windowSize/2; dx <= windowSize/2; ++dx) {
            int newX = x + dx;
            int newY = y + dy;

            if (newX >= 0 && newX < image[0].size() &&
                newY >= 0 && newY < image.size()) {
                if (image[newY][newX] >= brightThreshold) {
                    brightPixels++;
                }
                totalPixels++;
            }
        }
    }

    // 如果周围70%以上的像素都是亮的，则认为在白色区域内
    return (static_cast<float>(brightPixels) / totalPixels) >= 0.7;
}

std::vector<std::vector<bool>> ImageProcessor::findBrightRegions(uint16_t brightThreshold) {
    int height = finalImage.size();
    int width = finalImage[0].size();
    std::vector<std::vector<bool>> brightRegions(height, std::vector<bool>(width, false));

    // 使用多线程处理
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int rowsPerThread = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * rowsPerThread;
        int endY = (i == numThreads - 1) ? height : startY + rowsPerThread;

        threads.emplace_back([this, &brightRegions, startY, endY, width, brightThreshold]() {
            for (int y = startY; y < endY; ++y) {
                for (int x = 0; x < width; ++x) {
                    brightRegions[y][x] = isInBrightRegion(finalImage, x, y, brightThreshold);
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return brightRegions;
}

void ImageProcessor::refineDarkLineDetection(std::vector<DarkLine>& lines) {
    if (lines.empty()) return;

    std::vector<DarkLine> mergedLines;
    std::vector<bool> merged(lines.size(), false);

    // Sort lines by position for more consistent merging
    std::sort(lines.begin(), lines.end(), [](const DarkLine& a, const DarkLine& b) {
        return a.start.x < b.start.x || (a.start.x == b.start.x && a.start.y < b.start.y);
    });

    // Enhanced merging criteria
    auto shouldMergeLines = [](const DarkLine& line1, const DarkLine& line2, int maxGap) {
        // Calculate endpoints distance
        double startDist = std::sqrt(std::pow(line1.start.x - line2.start.x, 2) +
                                     std::pow(line1.start.y - line2.start.y, 2));
        double endDist = std::sqrt(std::pow(line1.end.x - line2.end.x, 2) +
                                   std::pow(line1.end.y - line2.end.y, 2));

        // Calculate line angles
        double angle1 = std::atan2(line1.end.y - line1.start.y, line1.end.x - line1.start.x);
        double angle2 = std::atan2(line2.end.y - line2.start.y, line2.end.x - line2.start.x);
        double angleDiff = std::abs(angle1 - angle2);

        // Normalize angle difference to [0, π/2]
        angleDiff = std::min(angleDiff, M_PI - angleDiff);

        // Maximum allowed angle difference (30 degrees in radians)
        const double MAX_ANGLE_DIFF = M_PI / 6;

        return (startDist < maxGap || endDist < maxGap) && angleDiff < MAX_ANGLE_DIFF;
    };

    for (size_t i = 0; i < lines.size(); ++i) {
        if (merged[i]) continue;

        DarkLine currentLine = lines[i];
        bool hasMerged;

        do {
            hasMerged = false;
            for (size_t j = i + 1; j < lines.size(); ++j) {
                if (merged[j]) continue;

                int maxGap = std::max(currentLine.thickness, lines[j].thickness) * 2;
                if (shouldMergeLines(currentLine, lines[j], maxGap)) {
                    // Merge lines
                    currentLine.start.x = std::min(currentLine.start.x, lines[j].start.x);
                    currentLine.start.y = std::min(currentLine.start.y, lines[j].start.y);
                    currentLine.end.x = std::max(currentLine.end.x, lines[j].end.x);
                    currentLine.end.y = std::max(currentLine.end.y, lines[j].end.y);
                    currentLine.thickness = std::max(currentLine.thickness, lines[j].thickness);

                    merged[j] = true;
                    hasMerged = true;
                }
            }
        } while (hasMerged);

        mergedLines.push_back(currentLine);
    }

    lines = mergedLines;
}

void ImageProcessor::removeDarkLines(const std::vector<DarkLine>& lines) {
    saveCurrentState();

    for (const auto& line : lines) {
        // Calculate the line vector
        double dx = line.end.x - line.start.x;
        double dy = line.end.y - line.start.y;
        double length = std::sqrt(dx * dx + dy * dy);

        // Normalize direction vector
        double dirX = dx / length;
        double dirY = dy / length;

        // Calculate perpendicular vector
        double perpX = -dirY;
        double perpY = dirX;

        // Increased sampling density for better interpolation
        double step = 0.2;  // Smaller step size for more precise removal

        for (double t = 0; t <= length; t += step) {
            int x = static_cast<int>(line.start.x + t * dirX);
            int y = static_cast<int>(line.start.y + t * dirY);

            // Extend removal width based on line thickness
            int removalWidth = line.thickness * 1.5;  // Slightly wider than detected thickness

            for (int offset = -removalWidth; offset <= removalWidth; ++offset) {
                int px = static_cast<int>(x + offset * perpX);
                int py = static_cast<int>(y + offset * perpY);

                if (px >= 0 && px < finalImage[0].size() && py >= 0 && py < finalImage.size()) {
                    // Use enhanced interpolation for pixel replacement
                    finalImage[py][px] = interpolateValue(px, py, std::max(5, line.thickness));
                }
            }
        }
    }
}

uint16_t ImageProcessor::interpolateValue(int x, int y, int margin) {
    std::vector<uint16_t> values;
    int width = finalImage[0].size();
    int height = finalImage.size();

    // Gather values from a wider area for better interpolation
    for (int dy = -margin; dy <= margin; ++dy) {
        for (int dx = -margin; dx <= margin; ++dx) {
            if (dx == 0 && dy == 0) continue;

            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                values.push_back(finalImage[ny][nx]);
            }
        }
    }

    if (values.empty()) {
        return 65535;  // Default to white if no valid values found
    }

    // Use weighted median for more robust interpolation
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

void ImageProcessor::removeFromZeroX(const std::vector<DarkLine>& lines) {
    if (lines.empty()) return;

    saveCurrentState();

    size_t height = finalImage.size();
    size_t width = finalImage[0].size();

    // Create a mask for pixels to be interpolated
    std::vector<std::vector<bool>> maskToInterpolate(height, std::vector<bool>(width, false));

    // Only process lines that start from x=0
    for (const auto& line : lines) {
        if (line.start.x == 0) {  // Only process lines starting from x=0
            double vectorX = line.end.x - line.start.x;
            double vectorY = line.end.y - line.start.y;
            double length = std::sqrt(vectorX * vectorX + vectorY * vectorY);

            if (length < 1e-6) continue;

            double dirX = vectorX / length;
            double dirY = vectorY / length;
            double perpX = -dirY;
            double perpY = dirX;

            double step = 0.2;  // Small step size for precise removal

            for (double t = 0; t <= length; t += step) {
                int x = static_cast<int>(line.start.x + t * dirX);
                int y = static_cast<int>(line.start.y + t * dirY);

                int removalWidth = static_cast<int>(line.thickness * 1.5);
                for (int offset = -removalWidth; offset <= removalWidth; ++offset) {
                    int px = static_cast<int>(x + offset * perpX);
                    int py = static_cast<int>(y + offset * perpY);

                    if (px >= 0 && static_cast<size_t>(px) < width &&
                        py >= 0 && static_cast<size_t>(py) < height) {
                        maskToInterpolate[py][px] = true;
                    }
                }
            }
        }
    }

    // Perform interpolation for all marked pixels
    std::vector<std::vector<uint16_t>> tempImage = finalImage;
    for (size_t y = 0; y < height; ++y) {
        for (size_t x = 0; x < width; ++x) {
            if (maskToInterpolate[y][x]) {
                tempImage[y][x] = interpolateValue(x, y, 7);
            }
        }
    }

    finalImage = std::move(tempImage);
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
