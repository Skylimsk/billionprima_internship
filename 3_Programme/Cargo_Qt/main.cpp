#include <QApplication>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QLabel>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QMouseEvent>
#include <QRect>
#include <QGroupBox>
#include <QGridLayout>
#include <QScrollArea>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <QComboBox>
#include <stack>
#include <thread>
#include <random>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Struct to hold image processing parameters
struct ImageProcessingParams {
    float yStretchFactor = 1.0f;          // Stretch factor in Y-axis (default: 1.0)
    float contrastFactor = 1.09f;         // Contrast factor (default: 1.09)
    float sharpenStrength = 1.09f;        // Sharpening strength (default: 1.09)
    float gammaValue = 1.2f;              // Gamma correction value (default: 1.2)
    int linesToProcess = 200;             // Number of lines to process (default: 200)
    float darkPixelThreshold = 0.5f;      // Threshold for dark pixels (default: 0.5)
    uint16_t darkThreshold = 2048;        // Threshold for dark regions (default: 2048)
    float regionGamma = 1.0f;             // Gamma for selected region (default: 1.0)
    float regionSharpen = 1.0f;           // Sharpening for selected region (default: 1.0)
    float regionContrast = 1.0f;          // Contrast for selected region (default: 1.0)
    float distortionFactor = 1.0f;        // Distortion factor (default: 1.0)
    bool useSineDistortion = false;       // Toggle sine distortion effect
    int paddingSize = 1;                  // Padding size for image processing (default: 1)
    float scalingFactor = 1.06f;
};

// Global variables
ImageProcessingParams params;
std::vector<std::vector<uint16_t>> imgData, originalImg, finalImage;
std::stack<std::vector<std::vector<uint16_t>>> imageHistory;
QLabel* imageLabel = nullptr;
int rotationState = 0;                    // Track rotation state (0, 90, 180, 270 degrees)
QRect selectedRegion;
bool regionSelected = false;
float displayScale = 1.0f;
int kernelSize = 0;                       // Holds the kernel size for image filtering

// Function declarations
void processImage();
void updateImageDisplay();
void loadTxtImage(const QString& filePath);
void processXAxis(std::vector<std::vector<uint16_t>>& img);
void processYAxis(std::vector<std::vector<uint16_t>>& img, int linesToAvg, float darkPct, uint16_t darkThreshold);
void processBasedOnRegion(std::vector<std::vector<uint16_t>>& img);
std::vector<std::vector<uint16_t>> rotateImage(const std::vector<std::vector<uint16_t>>& image, int angle);
void sharpenImage(std::vector<std::vector<uint16_t>>& img, float sharpenStrength);
void adjustContrast(std::vector<std::vector<uint16_t>>& img, float contrastFactor);
void gammaCorrection(std::vector<std::vector<uint16_t>>& img, float gammaValue);
void stretchImageY(std::vector<std::vector<uint16_t>>& img, float yStretchFactor);
void stretchImageX(std::vector<std::vector<uint16_t>>& img, float xStretchFactor);
void processAndMergeImageParts();
void saveImageState();
void revertImage();
void adjustWindowSize();
void cropRegion(const QRect& region);
void applyMedianFilter(std::vector<std::vector<uint16_t>>& image, int kernelSize);
void applyHighPassFilter(std::vector<std::vector<uint16_t>>& image);
void saveImage(const QString& filePath);


enum DistortionDirection { LEFT, RIGHT, TOP, BOTTOM };
QLineEdit* LineToProcessEdit;
QLineEdit* DarkPixelThresholdEdit;
QLineEdit* DarkThresholdEdit;
QLineEdit* ScalingFactorEdit;
bool updatingFromSystem = false;

class ImageLabel : public QLabel {
public:
    ImageLabel(QWidget* parent = nullptr) : QLabel(parent), transform(QTransform()) {}

    // 设置图片变换（旋转或缩放等）
    void setImageTransform(const QTransform& newTransform) {
        transform = newTransform;
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        // 将鼠标位置转换为原始图像坐标
        QPointF scaledPoint = transform.inverted().map(event->pos());
        int imageWidth = static_cast<int>(finalImage[0].size());
        int imageHeight = static_cast<int>(finalImage.size());
        scaledPoint.setX(std::clamp(static_cast<int>(scaledPoint.x()), 0, imageWidth - 1));
        scaledPoint.setY(std::clamp(static_cast<int>(scaledPoint.y()), 0, imageHeight - 1));

        // 开始选择
        selectedRegion.setTopLeft(scaledPoint.toPoint());
        selectedRegion.setBottomRight(scaledPoint.toPoint());  // 将两个角都设置为相同的点
        regionSelected = false;  // 重置选择状态
        updateImageDisplay();  // 刷新显示以显示初始选择区域
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        // 将鼠标位置转换为原始图像坐标
        QPointF scaledPoint = transform.inverted().map(event->pos());
        int imageWidth = static_cast<int>(finalImage[0].size());
        int imageHeight = static_cast<int>(finalImage.size());
        scaledPoint.setX(std::clamp(static_cast<int>(scaledPoint.x()), 0, imageWidth - 1));
        scaledPoint.setY(std::clamp(static_cast<int>(scaledPoint.y()), 0, imageHeight - 1));

        // 更新选择矩形的右下角
        selectedRegion.setBottomRight(scaledPoint.toPoint());
        regionSelected = true;  // 表示正在选择区域
        updateImageDisplay();  // 刷新显示以显示更新的选择区域
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        // 将鼠标位置转换为原始图像坐标
        QPointF scaledPoint = transform.inverted().map(event->pos());
        int imageWidth = static_cast<int>(finalImage[0].size());
        int imageHeight = static_cast<int>(finalImage.size());
        scaledPoint.setX(std::clamp(static_cast<int>(scaledPoint.x()), 0, imageWidth - 1));
        scaledPoint.setY(std::clamp(static_cast<int>(scaledPoint.y()), 0, imageHeight - 1));

        // 确认选择矩形
        selectedRegion.setBottomRight(scaledPoint.toPoint());
        regionSelected = true;  // 表示已选择区域
        updateImageDisplay();  // 刷新显示以显示最终选择区域
    }

private:
    QTransform transform;  // 保存图片的变换信息
};


void adjustWindowSize() {
    if (!finalImage.empty()) {
        int imageHeight = finalImage.size();
        int imageWidth = finalImage[0].size();

        // Calculate scaling factor to fit the image within the label while preserving aspect ratio
        double widthScale = static_cast<double>(imageLabel->width()) / imageWidth;
        double heightScale = static_cast<double>(imageLabel->height()) / imageHeight;
        displayScale = std::min(widthScale, heightScale); // Choose the smaller scale to fit in both dimensions

        // Resize QLabel to fit the image within its boundaries
        imageLabel->setFixedSize(static_cast<int>(imageWidth * displayScale), static_cast<int>(imageHeight * displayScale));
        updateImageDisplay(); // Update display to apply scaling
    }
}

// Function to load image data from a text file
void loadTxtImage(const std::string& txtFilePath) {
    std::ifstream inFile(txtFilePath);
    if (!inFile.is_open()) {
        std::cerr << "Error: Cannot open file " << txtFilePath << std::endl;
        return;
    }

    std::string line;
    imgData.clear();
    size_t maxWidth = 0;

    // Read data line by line
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<uint16_t> row;
        uint32_t value;

        while (ss >> value) {
            // Convert to 16-bit
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

    // Pad all rows to the same width with 0
    for (auto& row : imgData) {
        if (row.size() < maxWidth) {
            row.resize(maxWidth, 0);
        }
    }

    originalImg = imgData;
    finalImage = imgData;

    processImage();
    selectedRegion = QRect();
    regionSelected = false;
    updateImageDisplay();
}

void processImage() {
    saveImageState();
    finalImage = originalImg;

    updateImageDisplay();
}

// Function to update the image display
void updateImageDisplay() {
    if (!finalImage.empty()) {
        int height = finalImage.size();
        int width = finalImage[0].size();
        QImage image(width, height, QImage::Format_Grayscale16);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint16_t pixelValue = finalImage[y][x];
                image.setPixel(x, y, qRgb(pixelValue >> 8, pixelValue >> 8, pixelValue >> 8));
            }
        }

        QPixmap pixmap = QPixmap::fromImage(image);

        // Draw the selection box if a region is selected
        if (regionSelected) {
            QPainter painter(&pixmap);
            painter.setPen(QPen(Qt::red, 2));  // Red color, 2-pixel thickness
            painter.drawRect(selectedRegion);
        }

        // Set the pixmap to the QLabel without scaling
        imageLabel->setPixmap(pixmap);
        imageLabel->setFixedSize(pixmap.size());  // Adjust the QLabel size to match the image
    }
}

void saveImageState() {
    imageHistory.push(finalImage);
}

// Revert to the previous image state
void revertImage() {
    if (!imageHistory.empty()) {
        finalImage = imageHistory.top();
        imageHistory.pop();
        updateImageDisplay();
    } else {
        std::cout << "No more history to revert!" << std::endl;
    }
}

std::vector<std::vector<uint16_t>> rotateImage(const std::vector<std::vector<uint16_t>>& image, int angle) {
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
    // Perform rotation
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

void stretchImageY(std::vector<std::vector<uint16_t>>& img, float yStretchFactor) {
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
                (1 - weight) * img[yLow][x] + weight * img[yHigh][x]
                );
        }
    }

    img = std::move(stretchedImg);
}

void stretchImageX(std::vector<std::vector<uint16_t>>& img, float xStretchFactor) {
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
                (1 - weight) * img[y][xLow] + weight * img[y][xHigh]
                );
        }
    }

    img = std::move(stretchedImg);
}

void processXAxis(std::vector<std::vector<uint16_t>>& image, float adjustmentFactor = 0.5f) {
    int height = image.size();
    int width = image[0].size();

    for (int x = 0; x < width; ++x) {
        uint64_t columnSum = 0;
        uint16_t minPixel = 65535;
        uint16_t maxPixel = 0;

        for (int y = 0; y < height; ++y) {
            uint16_t pixelValue = image[y][x];
            columnSum += pixelValue;
            minPixel = std::min(minPixel, pixelValue);
            maxPixel = std::max(maxPixel, pixelValue);
        }

        if (maxPixel == minPixel) continue;

        float scale = 65535.0f / (maxPixel - minPixel);

        for (int y = 0; y < height; ++y) {
            float newValue = static_cast<float>(image[y][x] - minPixel) * scale;

            newValue *= adjustmentFactor;

            image[y][x] = static_cast<uint16_t>(std::fmin(std::fmax(newValue, 0.0f), 65535.0f));
        }
    }
}

void processYAxis(std::vector<std::vector<uint16_t>>& image, int dynamicLinesToProcess, float darkPixelThreshold, uint16_t darkThreshold, float scalingFactor, int iterations = 3) {
    int height = image.size();
    int width = image[0].size();

    for (int iteration = 0; iteration < iterations; ++iteration) {
        // Dynamically determine dynamicLinesToProcess by finding the darkest region based on overall image brightness
        dynamicLinesToProcess = std::max(10, height / 10); // Minimum of 10 lines or 10% of the height
        dynamicLinesToProcess = std::min(dynamicLinesToProcess, height);

        // Dynamically adjust darkThreshold and darkPixelThreshold based on overall image brightness
        uint64_t totalPixelValue = 0;
        uint64_t totalPixelCount = static_cast<uint64_t>(height) * width;
        for (const auto& row : image) {
            totalPixelValue += std::accumulate(row.begin(), row.end(), static_cast<uint64_t>(0));
        }
        float averagePixelValue = static_cast<float>(totalPixelValue) / totalPixelCount;

        darkThreshold = static_cast<uint16_t>(std::max(1024.0f, averagePixelValue * 0.5f)); // Adjust darkThreshold dynamically
        darkPixelThreshold = std::min(0.8f, averagePixelValue / 65535.0f * 1.5f);            // Adjust darkPixelThreshold dynamically

        // Update input fields to reflect dynamic changes
        updatingFromSystem = true;
        LineToProcessEdit->setText(QString::number(dynamicLinesToProcess));
        DarkPixelThresholdEdit->setText(QString::number(darkPixelThreshold));
        DarkThresholdEdit->setText(QString::number(darkThreshold));
        ScalingFactorEdit->setText(QString::number(scalingFactor, 'f', 3));
        updatingFromSystem = false;

        // Debugging outputs for dynamic parameters
        std::cout << "Iteration: " << iteration + 1 << " of " << iterations << std::endl;
        std::cout << "Dynamic lines to process: " << dynamicLinesToProcess << std::endl;
        std::cout << "Dark threshold (dynamic): " << darkThreshold << std::endl;
        std::cout << "Dark pixel threshold (dynamic): " << darkPixelThreshold << std::endl;
        std::cout << "Scaling factor: " << scalingFactor << std::endl;

        std::vector<std::pair<int, float>> lineDarkRatios;
        for (int y = 0; y < height; ++y) {
            int darkPixelCount = std::count_if(image[y].begin(), image[y].end(), [&](uint16_t pixel) { return pixel < darkThreshold; });
            float darkPixelRatio = static_cast<float>(darkPixelCount) / width;
            lineDarkRatios.emplace_back(y, darkPixelRatio);
        }

        // Sort lines by dark pixel ratio in descending order
        std::sort(lineDarkRatios.begin(), lineDarkRatios.end(), [](const std::pair<int, float>& a, const std::pair<int, float>& b) {
            return a.second > b.second;
        });

        // Select the top N darkest lines
        dynamicLinesToProcess = std::min(dynamicLinesToProcess, static_cast<int>(lineDarkRatios.size()));
        std::vector<int> darkestLines;
        for (int i = 0; i < dynamicLinesToProcess; ++i) {
            darkestLines.push_back(lineDarkRatios[i].first);
        }

        // Calculate the locally weighted average pixel value for the selected darkest lines
        std::vector<uint16_t> avgLine(width, 0);
        float totalWeight = 0.0f;
        for (int y : darkestLines) {
            float weight = lineDarkRatios[y].second; // Use dark pixel ratio as weight
            totalWeight += weight;
            for (int x = 0; x < width; ++x) {
                avgLine[x] += static_cast<uint16_t>(image[y][x] * weight);
            }
        }
        for (int x = 0; x < width; ++x) {
            avgLine[x] = static_cast<uint16_t>(avgLine[x] / totalWeight);
        }

        // Process each line of the image with selective enhancement
        for (int y = 0; y < height; ++y) {
            int darkPixelCount = std::count_if(image[y].begin(), image[y].end(), [&](uint16_t pixel) { return pixel < darkThreshold; });
            float darkPixelRatio = static_cast<float>(darkPixelCount) / width;

            float adaptiveScalingFactor = scalingFactor; // Keep scalingFactor fixed

            if (darkPixelRatio > darkPixelThreshold) {
                // Apply stronger enhancement for lines with high dark pixel ratio
                std::transform(avgLine.begin(), avgLine.end(), image[y].begin(), [&](uint16_t avgPixel) {
                    return static_cast<uint16_t>(std::min(static_cast<float>(avgPixel) * adaptiveScalingFactor, 65535.0f));
                });
            } else {
                // Apply lighter enhancement for lines with lower dark pixel ratio
                std::transform(image[y].begin(), image[y].end(), image[y].begin(), [&](uint16_t pixel) {
                    return static_cast<uint16_t>(std::min(static_cast<float>(pixel) * adaptiveScalingFactor, 65535.0f));
                });
            }
        }
    }
}
void processAndMergeImageParts() {
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

    // Noise detection and removal function
    auto isNoise = [](const std::vector<std::vector<uint16_t>>& imgPart, uint16_t threshold = 50) {
        int noiseCount = 0;
        int totalPixels = imgPart.size() * imgPart[0].size();

        for (const auto& row : imgPart) {
            for (uint16_t pixel : row) {
                if (pixel < threshold) {  // Pixels below the threshold are considered noise
                    noiseCount++;
                }
            }
        }

        // If more than 70% of pixels are noise, mark the part as noisy
        return (noiseCount > totalPixels * 0.7);
    };

    // Remove noise using neighbouring part pixels or average filtering
    auto removeNoise = [&](std::vector<std::vector<uint16_t>>& imgPart, const std::vector<std::vector<uint16_t>>& neighborPart) {
        for (int y = 0; y < imgPart.size(); ++y) {
            for (int x = 0; x < imgPart[0].size(); ++x) {
                if (imgPart[y][x] < 50) {  // If pixel is considered noise
                    imgPart[y][x] = static_cast<uint16_t>((static_cast<uint32_t>(imgPart[y][x]) + static_cast<uint32_t>(neighborPart[y][x])) / 2);  // Replace with average of current and neighbour pixel to reduce sharp lines
                }
            }
        }
    };

    // Process each part for noise detection and removal
    auto processPart = [&](std::vector<std::vector<uint16_t>>& part, const std::vector<std::vector<uint16_t>>& neighbor) {
        if (isNoise(part)) {
            std::cerr << "Detected noise, removing..." << std::endl;
            removeNoise(part, neighbor);
        } else {
            // Apply stretch based on rotation state
            if (rotationState == 1 || rotationState == 3) {
                stretchImageX(part, params.yStretchFactor);  // Stretch along X-axis for 90 or 270-degree rotations
            } else {
                stretchImageY(part, params.yStretchFactor);  // Stretch along Y-axis otherwise
            }
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

    // Update the image display
    updateImageDisplay();
}


void cropRegion(const QRect& region) {
    if (!regionSelected) return;

    int left = std::min(region.left(), region.right());
    int top = std::min(region.top(), region.bottom());
    int right = std::max(region.left(), region.right());
    int bottom = std::max(region.top(), region.bottom());

    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(static_cast<int>(finalImage[0].size()), right);
    bottom = std::min(static_cast<int>(finalImage.size()), bottom);

    if (left >= right || top >= bottom) {
        return;
    }

    size_t newWidth = right - left;
    size_t newHeight = bottom - top;
    std::vector<std::vector<uint16_t>> croppedImage(newHeight, std::vector<uint16_t>(newWidth));

    for (size_t y = 0; y < newHeight; ++y) {
        for (size_t x = 0; x < newWidth; ++x) {
            croppedImage[y][x] = finalImage[top + y][left + x];
        }
    }
    finalImage = croppedImage;
    regionSelected = false;
    updateImageDisplay();
}

// Helper function to process contrast in a specific range
void processContrastChunk(int top, int bottom, int left, int right, float contrastFactor, std::vector<std::vector<uint16_t>>& img) {
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;

    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            float pixel = static_cast<float>(img[y][x]);
            img[y][x] = static_cast<uint16_t>(std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0f, MAX_PIXEL_VALUE));
        }
    }
}

void applyContrastToRegion(const QRect& region) {
    if (!regionSelected || finalImage.empty()) return;

    int left = std::min(region.left(), region.right());
    int right = std::max(region.left(), region.right());
    int top = std::min(region.top(), region.bottom());
    int bottom = std::max(region.top(), region.bottom());

    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(static_cast<int>(finalImage[0].size()), right);
    bottom = std::min(static_cast<int>(finalImage.size()), bottom);

    if (left >= right || top >= bottom) return;

    saveImageState();

    // Define number of threads and divide the work
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = (bottom - top) / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = top + i * chunkHeight;
        int endY = (i == numThreads - 1) ? bottom : startY + chunkHeight;

        // Each thread handles a horizontal slice of the image
        threads.emplace_back(processContrastChunk, startY, endY, left, right, params.regionContrast, std::ref(finalImage));
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    updateImageDisplay();
}

void adjustContrast(std::vector<std::vector<uint16_t>>& img, float contrastFactor) {
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;
    int height = img.size();
    int width = img[0].size();

    // Define number of threads and divide the work
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;

        // Each thread handles a slice of the image
        threads.emplace_back([=, &img]() {
            for (int y = startY; y < endY; ++y) {
                for (int x = 0; x < width; ++x) {
                    float pixel = static_cast<float>(img[y][x]);
                    img[y][x] = static_cast<uint16_t>(std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0f, MAX_PIXEL_VALUE));
                }
            }
        });
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// Function to apply gamma correction to a portion of the image (used for both overall and region)
void adjustGammaForRegion(std::vector<std::vector<uint16_t>>& img, float gamma, int startY, int endY, int startX, int endX) {
    float invGamma = 1.0f / gamma;
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            float normalized = img[y][x] / 65535.0f; // Normalize the pixel value
            float corrected = std::pow(normalized, invGamma); // Apply Gamma correction
            img[y][x] = static_cast<uint16_t>(corrected * 65535.0f); // Convert back to pixel value
        }
    }
}

// Function to apply gamma correction to the entire image using multithreading
void adjustGammaOverall(std::vector<std::vector<uint16_t>>& img, float gamma) {
    int height = img.size();
    int width = img[0].size();

    // Use multithreading to process the image
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int blockHeight = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * blockHeight;
        int endY = (i == numThreads - 1) ? height : startY + blockHeight;
        threads.emplace_back(adjustGammaForRegion, std::ref(img), gamma, startY, endY, 0, width);
    }

    // Join all threads
    for (auto& thread : threads) {
        thread.join();
    }
}

void adjustGammaForSelectedRegion(std::vector<std::vector<uint16_t>>& img, float gamma, int regionStartX, int regionEndX, int regionStartY, int regionEndY) {
    // Ensure the image is not empty
    if (img.empty()) return;

    // Calculate and clamp the region boundaries
    int left = std::min(regionStartX, regionEndX);
    int right = std::max(regionStartX, regionEndX);
    int top = std::min(regionStartY, regionEndY);
    int bottom = std::max(regionStartY, regionEndY);

    // Clamp region boundaries to ensure they are within image size limits
    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(static_cast<int>(img[0].size()), right);
    bottom = std::min(static_cast<int>(img.size()), bottom);

    // Check if the region is valid (return if invalid)
    if (left >= right || top >= bottom) return;

    // Apply gamma correction to the specified region
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int regionHeight = bottom - top;
    int blockHeight = regionHeight / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = top + i * blockHeight;
        int endY = (i == numThreads - 1) ? bottom : startY + blockHeight;
        threads.emplace_back(adjustGammaForRegion, std::ref(img), gamma, startY, endY, left, right);
    }

    // Join threads
    for (auto& thread : threads) {
        thread.join();
    }
}

// Helper function to apply sharpening to a chunk of the image, with boundary checks
void processSharpenChunk(int startY, int endY, int width, std::vector<std::vector<uint16_t>>& img, const std::vector<std::vector<uint16_t>>& tempImg, float sharpenStrength) {
    float kernel[3][3] = {
        { 0, -1 * sharpenStrength,  0 },
        { -1 * sharpenStrength, 1 + 4 * sharpenStrength, -1 * sharpenStrength },
        { 0, -1 * sharpenStrength,  0 }
    };

    for (int y = startY; y < endY; ++y) {
        for (int x = 1; x < width - 1; ++x) {
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

void sharpenImage(std::vector<std::vector<uint16_t>>& img, float sharpenStrength) {
    int height = img.size();
    int width = img[0].size();
    std::vector<std::vector<uint16_t>> tempImg = img;  // Make a copy of the original image for convolution

    // Define number of threads and divide the work
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;

        // Each thread handles a slice of the image
        threads.emplace_back(processSharpenChunk, startY, endY, width, std::ref(img), std::ref(tempImg), sharpenStrength);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void processSharpenRegionChunk(int startY, int endY, int left, int right, std::vector<std::vector<uint16_t>>& img, float sharpenStrength) {
    for (int y = startY; y < endY; ++y) {
        for (int x = left; x < right; ++x) {
            img[y][x] = std::clamp(static_cast<int>(img[y][x] * sharpenStrength), 0, 65535);
        }
    }
}

void applySharpenToRegion(const QRect& region) {
    if (!regionSelected || finalImage.empty()) return;

    int left = std::min(region.left(), region.right());
    int right = std::max(region.left(), region.right());
    int top = std::min(region.top(), region.bottom());
    int bottom = std::max(region.top(), region.bottom());

    left = std::max(0, left);
    top = std::max(0, top);
    right = std::min(static_cast<int>(finalImage[0].size()), right);
    bottom = std::min(static_cast<int>(finalImage.size()), bottom);

    if (left >= right || top >= bottom) return;

    saveImageState();

    // Define number of threads and divide the work
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = (bottom - top) / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = top + i * chunkHeight;
        int endY = (i == numThreads - 1) ? bottom : startY + chunkHeight;

        // Each thread handles a slice of the region
        threads.emplace_back(processSharpenRegionChunk, startY, endY, left, right, std::ref(finalImage), params.regionSharpen);
    }

    // Wait for all threads to finish
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    updateImageDisplay();
}

void saveImage(const QString& filePath) {
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
        std::cout << "Image saved successfully: " << filePath.toStdString() << std::endl;
    } else {
        std::cerr << "Failed to save the image." << std::endl;
    }
}

void applyMedianFilter(std::vector<std::vector<uint16_t>>& image, int filterKernelSize) {
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

void applyHighPassFilter(std::vector<std::vector<uint16_t>>& image) {

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

    float scalingFactor = 1.5f;  // Set the scaling factor to 1.5

    // Apply the high-pass filter
    for (int y = halfSize; y < height - halfSize; ++y) {
        for (int x = halfSize; x < width - halfSize; ++x) {
            int newValue = 0;

            // Convolve the kernel with the image
            for (int ky = -halfSize; ky <= halfSize; ++ky) {
                for (int kx = -halfSize; kx <= halfSize; ++kx) {
                    newValue += image[y + ky][x + kx] * kernel[ky + halfSize][kx + halfSize];
                }
            }

            // Apply scaling factor to amplify the high-pass filter effect
            newValue = static_cast<int>(newValue * scalingFactor);

            // Clamp the pixel value to be within the range of 0-65535
            outputImage[y][x] = static_cast<uint16_t>(std::clamp(newValue, 0, 65535));
        }
    }

    // Replace the original image with the sharpened one
    image = outputImage;
}


std::vector<std::vector<uint16_t>> distortImage(const std::vector<std::vector<uint16_t>>& image, float distortionFactor, const std::string& direction) {
    int height = image.size();
    int width = image[0].size();
    int centerX = width / 2;
    int centerY = height / 2;

    std::vector<std::vector<uint16_t>> distortedImage = image;

    if (direction == "Left") {
        // Distort left half
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < centerX; ++x) {
                int newX = static_cast<int>(x + distortionFactor * std::sin(M_PI * x / centerX));
                newX = std::min(std::max(newX, 0), centerX - 1);
                distortedImage[y][x] = image[y][newX];
            }
        }

    } else if (direction == "Right") {
        // Distort right half
        for (int y = 0; y < height; ++y) {
            for (int x = centerX; x < width; ++x) {
                int newX = static_cast<int>(x + distortionFactor * std::sin(M_PI * (x - centerX) / (width - centerX)));
                newX = std::min(std::max(newX, centerX), width - 1);
                distortedImage[y][newX] = image[y][x];
            }
        }

    } else if (direction == "Top") {
        // Distort top half
        for (int y = centerY - 1; y >= 0; --y) {
            for (int x = 0; x < width; ++x) {
                int newY = static_cast<int>(y + distortionFactor * std::sin(M_PI * (y - centerY) / centerY));
                newY = std::min(std::max(newY, 0), centerY - 1);
                distortedImage[newY][x] = image[y][x];
            }
        }

    } else if (direction == "Bottom") {
        // Distort bottom half
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


std::vector<std::vector<uint16_t>> addPadding(const std::vector<std::vector<uint16_t>>& image, int paddingSize) {
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

void setupControlPanel(QWidget* window) {

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    int taskbarHeight = 40;
    int availableHeight = screenHeight - taskbarHeight;

    int windowWidth = screenWidth * 0.9;
    int windowHeight = availableHeight * 0.9;
    window->resize(windowWidth, windowHeight);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(1);
    mainLayout->setContentsMargins(2, 2, 2, 2);

    QWidget* controlPanelWidget = new QWidget;
    controlPanelWidget->setFixedSize(windowWidth, 250);

    QGridLayout* controlPanelLayout = new QGridLayout(controlPanelWidget);
    controlPanelLayout->setHorizontalSpacing(5);
    controlPanelLayout->setVerticalSpacing(1);
    controlPanelLayout->setContentsMargins(2, 2, 2, 2);

    QSize buttonSize(110, 35);

    auto createButton = [&](const QString& buttonName, std::function<void()> onClick, int row, int column) {
        QPushButton* button = new QPushButton(buttonName);
        button->setFixedSize(buttonSize);
        controlPanelLayout->addWidget(button, row, column);
        QObject::connect(button, &QPushButton::clicked, [=]() {
            onClick();
            std::cout << buttonName.toStdString() << " pressed." << std::endl; // Print button name
        });
    };

    createButton("Browse", [&]() {
        QString fileName = QFileDialog::getOpenFileName(nullptr, "Open Text File", "", "Text Files (*.txt)");
        if (!fileName.isEmpty()) {
            loadTxtImage(fileName.toStdString());
            adjustWindowSize();
        }
    }, 0, 0);


    createButton("Crop", [&]() {
        cropRegion(selectedRegion);
        adjustWindowSize();
    }, 0, 1);

    createButton("Save", [&]() {
        QString filePath = QFileDialog::getSaveFileName(nullptr, "Save Image", "", "PNG Files (*.png)");
        if (!filePath.isEmpty()) {
            saveImage(filePath);
        }
    }, 0, 2);

    createButton("Revert", [&]() {
        revertImage();
        adjustWindowSize();
    }, 0, 3);

    createButton("Rotate CW", [&]() {
        saveImageState();
        finalImage = rotateImage(finalImage, 90);
        rotationState = (rotationState + 1) % 4;
        adjustWindowSize();
        updateImageDisplay();
    }, 0, 4);

    createButton("Rotate CCW", [&]() {
        saveImageState();
        finalImage = rotateImage(finalImage, 270);
        rotationState = (rotationState + 3) % 4;
        adjustWindowSize();
        updateImageDisplay();
    }, 0, 5);

    createButton("Process X-axis", [&]() {
        saveImageState();
        processXAxis(finalImage, 0.5f);
        adjustWindowSize();
        updateImageDisplay();
    }, 0, 6);

    createButton("Process Y-axis", [&]() {
        saveImageState();
        processYAxis(finalImage, params.linesToProcess, params.darkPixelThreshold, params.darkThreshold, params.scalingFactor);
        adjustWindowSize();
        updateImageDisplay();
    }, 0, 7);

    controlPanelLayout->addWidget(new QLabel("Line to Process:"), 0, 8);
    LineToProcessEdit = new QLineEdit(QString::number(params.linesToProcess));
    LineToProcessEdit->setFixedSize(50, 30);
    LineToProcessEdit->setValidator(new QDoubleValidator(1, 1000, 2));
    controlPanelLayout->addWidget(LineToProcessEdit, 0, 9);

    QObject::connect(LineToProcessEdit, &QLineEdit::textChanged, [=](const QString& text) {
        if (!updatingFromSystem) {
            params.linesToProcess = text.toFloat();
        }
    });

    controlPanelLayout->addWidget(new QLabel("Dark Pixel Threshold:"), 0, 10);
    DarkPixelThresholdEdit = new QLineEdit(QString::number(params.darkPixelThreshold));
    DarkPixelThresholdEdit->setFixedSize(50, 30);
    DarkPixelThresholdEdit->setValidator(new QDoubleValidator(0.1, 10, 2));
    controlPanelLayout->addWidget(DarkPixelThresholdEdit, 0, 11);

    QObject::connect(DarkPixelThresholdEdit, &QLineEdit::textChanged, [=](const QString& text) {
        if (!updatingFromSystem) {
            params.darkPixelThreshold = text.toFloat();
        }
    });

    controlPanelLayout->addWidget(new QLabel("Dark Threshold:"), 0, 12);
    DarkThresholdEdit = new QLineEdit(QString::number(params.darkThreshold));
    DarkThresholdEdit->setFixedSize(50, 30);
    DarkThresholdEdit->setValidator(new QDoubleValidator(1, 65535, 2));
    controlPanelLayout->addWidget(DarkThresholdEdit, 0, 13);

    QObject::connect(DarkThresholdEdit, &QLineEdit::textChanged, [=](const QString& text) {
        if (!updatingFromSystem) {
            params.darkThreshold = text.toUInt();
        }
    });

    controlPanelLayout->addWidget(new QLabel("Scaling Factor:"), 0, 14);
    ScalingFactorEdit = new QLineEdit(QString::number(params.scalingFactor));
    ScalingFactorEdit->setFixedSize(50, 30);
    ScalingFactorEdit->setValidator(new QDoubleValidator(0.1, 10, 3));
    controlPanelLayout->addWidget(ScalingFactorEdit, 0, 15);

    QObject::connect(ScalingFactorEdit, &QLineEdit::textChanged, [=](const QString& text) {
        if (!updatingFromSystem) {
            params.scalingFactor = text.toFloat();
        }
    });

    createButton("Stretch", [&]() {

        saveImageState();
        if (rotationState == 1 || rotationState == 3) {
            stretchImageX(finalImage, params.yStretchFactor);
        } else {
            stretchImageY(finalImage, params.yStretchFactor);
        }
        adjustWindowSize();
        updateImageDisplay();
    }, 1, 0);

    controlPanelLayout->addWidget(new QLabel("Stretch Factor:"), 1, 1);
    QLineEdit* stretchFactorLineEdit = new QLineEdit(QString::number(params.yStretchFactor));
    stretchFactorLineEdit->setFixedSize(50, 30);
    stretchFactorLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2));
    controlPanelLayout->addWidget(stretchFactorLineEdit, 1, 2);

    QObject::connect(stretchFactorLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.yStretchFactor = text.toFloat();
    });

    createButton("Split & Merge", [&]() {
        processAndMergeImageParts();
        adjustWindowSize();
        updateImageDisplay();
    }, 1, 3);

    createButton("Median Filter", [&]() {
        int filterKernelSize = 3;
        applyMedianFilter(finalImage, filterKernelSize);
        kernelSize = filterKernelSize;
        updateImageDisplay();
    }, 1, 4);

    createButton("High-Pass Filter", [&]() {
        applyHighPassFilter(finalImage);
        updateImageDisplay();
    }, 1, 5);

    QComboBox* distortionDirectionComboBox = new QComboBox();
    distortionDirectionComboBox->addItem("Left");
    distortionDirectionComboBox->addItem("Right");
    distortionDirectionComboBox->addItem("Top");
    distortionDirectionComboBox->addItem("Bottom");
    controlPanelLayout->addWidget(distortionDirectionComboBox, 1, 9);

    createButton("Apply Distortion", [=]() {

        QString selectedDirection = distortionDirectionComboBox->currentText();

        // Apply distortion based on the selected direction
        finalImage = distortImage(finalImage, params.distortionFactor, selectedDirection.toStdString());

        // Refresh image display
        updateImageDisplay();
    }, 1, 6);

    controlPanelLayout->addWidget(new QLabel("Distortion Factor:"), 1, 7);

    // Set the minimum value to 1 and maximum value to 100 using QDoubleValidator
    QLineEdit* distortionFactorLineEdit = new QLineEdit(QString::number(params.distortionFactor));
    distortionFactorLineEdit->setFixedSize(50, 30);
    distortionFactorLineEdit->setValidator(new QDoubleValidator(1.0, 100.0, 2)); // Min value: 1, Max value: 100
    controlPanelLayout->addWidget(distortionFactorLineEdit, 1, 8);

    // Ensure the user can only input values between 1 and 100
    QObject::connect(distortionFactorLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        bool ok;
        double inputValue = text.toDouble(&ok);

        // If the value is not valid, reset it to the closest valid value
        if (ok) {
            if (inputValue > 100.0) {
                distortionFactorLineEdit->setText("100");  // Set to maximum 100
            } else if (inputValue < 1.0) {
                distortionFactorLineEdit->setText("1");  // Set to minimum 1
            }
        } else {
            distortionFactorLineEdit->clear();  // Clear the field if the input is not a valid number
        }

        // Update the params.distortionFactor
        params.distortionFactor = distortionFactorLineEdit->text().toDouble();
    });

    createButton("Padding", [&]() {
        finalImage = addPadding(finalImage, params.paddingSize);
        updateImageDisplay();
    }, 1, 10);


    controlPanelLayout->addWidget(new QLabel("Padding Size:"), 1, 11);

    QLineEdit* paddingSizeEdit = new QLineEdit(QString::number(params.paddingSize));
    paddingSizeEdit->setFixedSize(50, 30);
    paddingSizeEdit->setValidator(new QIntValidator(1, 1000));
    controlPanelLayout->addWidget(paddingSizeEdit, 1, 12);

    QObject::connect(paddingSizeEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.paddingSize = text.toInt();
    });

    // Overall gamma control
    createButton("Overall Gamma", [&]() {
        saveImageState();  // 保存当前图像状态，便于撤销操作
        adjustGammaOverall(finalImage, params.gammaValue);  // 调用新的多线程整体Gamma校正函数
        adjustWindowSize();  // 调整窗口大小以适应新图像
        updateImageDisplay();  // 更新图像显示
    }, 2, 0);

    QLineEdit* overallGammaLineEdit = new QLineEdit(QString::number(params.gammaValue));
    overallGammaLineEdit->setFixedSize(50, 30);
    overallGammaLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2));  // 限制Gamma值在0.1到10之间
    controlPanelLayout->addWidget(overallGammaLineEdit, 2, 1);

    QObject::connect(overallGammaLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.gammaValue = text.toFloat();  // 更新Gamma值参数
    });
    // Overall sharpen control
    createButton("Overall Sharpen", [&]() {
        saveImageState();
        sharpenImage(finalImage, params.sharpenStrength);
        adjustWindowSize();
        updateImageDisplay();
    }, 2, 2);

    QLineEdit* overallSharpenLineEdit = new QLineEdit(QString::number(params.sharpenStrength));
    overallSharpenLineEdit->setFixedSize(50, 30);
    overallSharpenLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2));
    controlPanelLayout->addWidget(overallSharpenLineEdit, 2, 3);

    QObject::connect(overallSharpenLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.sharpenStrength = text.toFloat();
    });

    // Overall contrast control
    createButton("Overall Contrast", [&]() {
        saveImageState();
        adjustContrast(finalImage, params.contrastFactor);
        adjustWindowSize();
        updateImageDisplay();
    }, 2, 4);

    QLineEdit* overallContrastLineEdit = new QLineEdit(QString::number(params.contrastFactor));
    overallContrastLineEdit->setFixedSize(50, 30);
    overallContrastLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2));
    controlPanelLayout->addWidget(overallContrastLineEdit, 2, 5);

    QObject::connect(overallContrastLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.contrastFactor = text.toFloat();
    });

    // Regional gamma control
    createButton("Region Gamma", [&]() {
        saveImageState();  // 保存当前图像状态
        adjustGammaForSelectedRegion(finalImage, params.regionGamma, selectedRegion.left(), selectedRegion.right(), selectedRegion.top(), selectedRegion.bottom());  // 使用 QRect 的公共成员函数
        adjustWindowSize();  // 调整窗口大小以适应新图像
        updateImageDisplay();  // 更新图像显示
    }, 2, 6);


    QLineEdit* regionGammaLineEdit = new QLineEdit(QString::number(params.regionGamma));
    regionGammaLineEdit->setFixedSize(50, 30);
    regionGammaLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2));  // 限制Gamma值在0.1到10之间
    controlPanelLayout->addWidget(regionGammaLineEdit, 2, 7);

    QObject::connect(regionGammaLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.regionGamma = text.toFloat();  // 更新区域Gamma值参数
    });

    // Region sharpen control
    createButton("Region Sharpen", [&]() {
        applySharpenToRegion(selectedRegion);
        adjustWindowSize();
    }, 2, 8);

    QLineEdit* regionSharpenLineEdit = new QLineEdit(QString::number(params.regionSharpen));
    regionSharpenLineEdit->setFixedSize(50, 30);
    regionSharpenLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2));
    controlPanelLayout->addWidget(regionSharpenLineEdit, 2, 9);

    QObject::connect(regionSharpenLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.regionSharpen = text.toFloat();
    });

    // Region contrast control
    createButton("Region Contrast", [&]() {
        applyContrastToRegion(selectedRegion);
        adjustWindowSize();
    }, 2, 10);

    QLineEdit* regionContrastLineEdit = new QLineEdit(QString::number(params.regionContrast));
    regionContrastLineEdit->setFixedSize(50, 30);
    regionContrastLineEdit->setValidator(new QDoubleValidator(0.1, 10.0, 2));
    controlPanelLayout->addWidget(regionContrastLineEdit, 2, 11);

    QObject::connect(regionContrastLineEdit, &QLineEdit::textChanged, [=](const QString& text) {
        params.regionContrast = text.toFloat();
    });


    mainLayout->addWidget(controlPanelWidget, 0, Qt::AlignTop);

    imageLabel = new ImageLabel();
    imageLabel->setAlignment(Qt::AlignLeft);
    imageLabel->setFrameStyle(QFrame::Box | QFrame::Plain);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(scrollArea, 1);

    window->setLayout(mainLayout);
}

// Main function
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QWidget window;
    window.setWindowTitle("Image Processor with Region Selection");
    window.resize(1600, 800);
    setupControlPanel(&window);

    window.show();
    return app.exec();
}
