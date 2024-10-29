#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <QApplication>
#include <QPushButton>
#include <QFileDialog>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QTransform>       // Rotation
#include <QMouseEvent>

#include <QSlider>          // Slider
#include "stb_image_write.h"

// Function to save PNG
void savePNG(const std::string& pngFilePath, const std::vector<std::vector<uint16_t>>& image, int width, int height) {
    std::vector<uint8_t> imageData(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t pixel = static_cast<uint8_t>(image[y][x] >> 8);  // Shift right by 8 bits to reduce 16-bit to 8-bit
            imageData[3 * (y * width + x) + 0] = pixel; // R
            imageData[3 * (y * width + x) + 1] = pixel; // G
            imageData[3 * (y * width + x) + 2] = pixel; // B
        }
    }
    if (stbi_write_png(pngFilePath.c_str(), width, height, 3, imageData.data(), width * 3)) {
        std::cout << "PNG file created successfully: " << pngFilePath << std::endl;
    } else {
        std::cerr << "Error creating PNG file: " << pngFilePath << std::endl;
    }
}

// Function to save PNG with transparency
void savePNGWithTransparency(const std::string& pngFilePath, const std::vector<std::vector<uint16_t>>& image, int width, int height) {
    std::vector<uint8_t> imageData(width * height * 4);  // RGBA format requires 4 bytes per pixel

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t pixelValue = image[y][x];

            if (pixelValue == 0) {
                // Transparent pixel (set alpha to 0)
                imageData[4 * (y * width + x) + 0] = 0;  // R
                imageData[4 * (y * width + x) + 1] = 0;  // G
                imageData[4 * (y * width + x) + 2] = 0;  // B
                imageData[4 * (y * width + x) + 3] = 0;  // A (fully transparent)
            } else {
                uint8_t pixel = static_cast<uint8_t>(pixelValue >> 8);  // Shift right by 8 bits to reduce 16-bit to 8-bit
                imageData[4 * (y * width + x) + 0] = pixel;  // R
                imageData[4 * (y * width + x) + 1] = pixel;  // G
                imageData[4 * (y * width + x) + 2] = pixel;  // B
                imageData[4 * (y * width + x) + 3] = 255;    // A (fully opaque)
            }
        }
    }

    if (stbi_write_png(pngFilePath.c_str(), width, height, 4, imageData.data(), width * 4)) {
        std::cout << "PNG file with transparency created successfully: " << pngFilePath << std::endl;
    } else {
        std::cerr << "Error creating PNG file with transparency: " << pngFilePath << std::endl;
    }
}

// Function to rotate the image
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

// Function to apply a Gaussian blur (low-pass filter)
std::vector<std::vector<uint16_t>> applyGaussianBlur(const std::vector<std::vector<uint16_t>>& image, int kernelSize) {
    int height = image.size();
    int width = image[0].size();
    // Create a Gaussian kernel (simplified for this example)
    std::vector<std::vector<float>> kernel(kernelSize, std::vector<float>(kernelSize));
    float sigma = kernelSize / 5.0f;
    float sum = 0.0f;

    int halfKernel = kernelSize / 2;

    for (int i = -halfKernel; i <= halfKernel; ++i) {
        for (int j = -halfKernel; j <= halfKernel; ++j) {
            kernel[i + halfKernel][j + halfKernel] = std::exp(-(i * i + j * j) / (2 * sigma * sigma));
            sum += kernel[i + halfKernel][j + halfKernel];
        }
    }

    // Normalize the kernel
    for (int i = 0; i < kernelSize; ++i) {
        for (int j = 0; j < kernelSize; ++j) {
            kernel[i][j] /= sum;
        }
    }

    // Apply the Gaussian kernel (convolution)
    std::vector<std::vector<uint16_t>> blurredImage(height, std::vector<uint16_t>(width, 0));

    for (int y = halfKernel; y < height - halfKernel; ++y) {
        for (int x = halfKernel; x < width - halfKernel; ++x) {
            float pixelSum = 0.0f;
            for (int ky = -halfKernel; ky <= halfKernel; ++ky) {
                for (int kx = -halfKernel; kx <= halfKernel; ++kx) {
                    pixelSum += kernel[ky + halfKernel][kx + halfKernel] * image[y + ky][x + kx];
                }
            }
            blurredImage[y][x] = static_cast<uint16_t>(pixelSum);
        }
    }
    return blurredImage;
}

// High-pass filter based sharpening
std::vector<std::vector<uint16_t>> applyHighPassSharpening(const std::vector<std::vector<uint16_t>>& image, int kernelSize, float alpha) {
    int height = image.size();
    int width = image[0].size();

    // Apply Gaussian blur to get the low-frequency component
    std::vector<std::vector<uint16_t>> blurredImage = applyGaussianBlur(image, kernelSize);

    // Create high-pass filter by subtracting the blurred image from the original image
    std::vector<std::vector<int16_t>> highPassImage(height, std::vector<int16_t>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            highPassImage[y][x] = static_cast<int16_t>(image[y][x] - blurredImage[y][x]);
        }
    }

    // Sharpen the image by adding the high-pass filtered image back to the original image
    std::vector<std::vector<uint16_t>> sharpenedImage(height, std::vector<uint16_t>(width, 0));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int32_t sharpenedValue = static_cast<int32_t>(image[y][x] + alpha * highPassImage[y][x]);
            // Clamp the result to stay within valid 16-bit range
            sharpenedImage[y][x] = static_cast<uint16_t>(std::fmin(std::fmax(sharpenedValue, 0), 65535));
        }
    }
    return sharpenedImage;
}

// Bilinear interpolation resize function
std::vector<std::vector<uint16_t>> resizeImageBilinear(const std::vector<std::vector<uint16_t>>& image, int newHeight, int newWidth) {
    int oldHeight = image.size();
    int oldWidth = image[0].size();

    std::vector<std::vector<uint16_t>> resizedImage(newHeight, std::vector<uint16_t>(newWidth));

    float x_ratio = static_cast<float>(oldWidth) / newWidth;
    float y_ratio = static_cast<float>(oldHeight) / newHeight;

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {
            int x_l = static_cast<int>(x_ratio * x);
            int y_l = static_cast<int>(y_ratio * y);
            int x_h = std::min(x_l + 1, oldWidth - 1);
            int y_h = std::min(y_l + 1, oldHeight - 1);

            float x_weight = (x_ratio * x) - x_l;
            float y_weight = (y_ratio * y) - y_l;

            uint16_t a = image[y_l][x_l];
            uint16_t b = image[y_l][x_h];
            uint16_t c = image[y_h][x_l];
            uint16_t d = image[y_h][x_h];

            resizedImage[y][x] = static_cast<uint16_t>(
                a * (1 - x_weight) * (1 - y_weight) +
                b * x_weight * (1 - y_weight) +
                c * (1 - x_weight) * y_weight +
                d * x_weight * y_weight
                );
        }
    }
    return resizedImage;
}

// Combine parts using weighted average instead of max to preserve details
void mergeParts(const std::vector<std::vector<uint16_t>>& lowLowImage,
                const std::vector<std::vector<uint16_t>>& lowHighImage,
                const std::vector<std::vector<uint16_t>>& highLowImage,
                const std::vector<std::vector<uint16_t>>& highHighImage,
                std::vector<std::vector<uint16_t>>& mergedImage, int height, int width) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            mergedImage[y][x] = static_cast<uint16_t>(
                0.25 * lowLowImage[y][x] +
                0.25 * lowHighImage[y][x] +
                0.25 * highLowImage[y][x] +
                0.25 * highHighImage[y][x]);
        }
    }
}

// Function to apply contrast stretching
void contrastStretching(std::vector<std::vector<uint16_t>>& image, int linesToProcess) {
    int height = image.size();
    int width = image[0].size();
    uint16_t minPixel = 65535;
    uint16_t maxPixel = 0;

    // Find minimum and maximum pixel values
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width - linesToProcess; ++x) {
            minPixel = std::min(minPixel, image[y][x]);
            maxPixel = std::max(maxPixel, image[y][x]);
        }
    }

    // Apply contrast stretching formula: (pixel - min) * (65535 / (max - min))
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width - linesToProcess; ++x) {
            image[y][x] = static_cast<uint16_t>(((image[y][x] - minPixel) * 65535.0f) / (maxPixel - minPixel));
        }
    }
}

// Function for X and Y axis processing
void processXAxis(std::vector<std::vector<uint16_t>>& image) {
    int height = image.size();
    int width = image[0].size();

    for (int x = 0; x < width; ++x) {
        uint64_t columnSum = 0;
        for (int y = 0; y < height; ++y) {
            columnSum += image[y][x];
        }
        uint64_t columnAverage = std::max(columnSum / height, 1ULL);  // Ensure columnAverage is at least 1 to avoid divide-by-zero

        for (int y = 0; y < height; ++y) {
            image[y][x] = static_cast<uint16_t>(std::fmin(image[y][x] * (65535.0 / columnAverage), 65535.0f));
        }
    }
}

void processYAxis(std::vector<std::vector<uint16_t>>& image, int linesToProcess, float darkPixelThreshold = 0.5, uint16_t darkThreshold = 2048, float scalingFactor = 1.06) {
    int height = image.size();
    int width = image[0].size();

    int lineStart = 0;
    int lineEnd = std::min(linesToProcess, height);
    std::vector<uint16_t> avgLine(width, 0);

    // Calculate average pixel values for the first N lines
    for (int y = lineStart; y < lineEnd; ++y) {
        for (int x = 0; x < width; ++x) {
            avgLine[x] += image[y][x];
        }
    }
    for (int x = 0; x < width; ++x) {
        avgLine[x] /= (lineEnd - lineStart); // Calculate average pixel value per column
    }

    for (int y = 0; y < height; ++y) {
        // Count the number of dark pixels in this row
        int darkPixelCount = std::count_if(image[y].begin(), image[y].end(), [&](uint16_t pixel) { return pixel < darkThreshold; });

        // If the ratio of dark pixels exceeds the threshold, apply scaling
        float darkPixelRatio = static_cast<float>(darkPixelCount) / width;
        if (darkPixelRatio > darkPixelThreshold) {
            for (int x = 0; x < width; ++x) {
                image[y][x] = static_cast<uint16_t>(std::fmin(avgLine[x] * scalingFactor, 65535.0));
            }
        } else {
            for (int x = 0; x < width; ++x) {
                image[y][x] = static_cast<uint16_t>(std::fmin(image[y][x] * scalingFactor, 65535.0));
            }
        }
    }
}

void gammaCorrectionRegion(std::vector<std::vector<uint16_t>>& image, int xStart, int yStart, int xEnd, int yEnd, float gamma = 1.2f) {
    const float invGamma = 1.0f / gamma;
    const float maxVal = 65535.0f;

    for (int y = yStart; y <= yEnd; ++y) {
        for (int x = xStart; x <= xEnd; ++x) {
            float normalizedPixel = static_cast<float>(image[y][x]) / maxVal;
            image[y][x] = static_cast<uint16_t>(std::pow(normalizedPixel, invGamma) * maxVal);
        }
    }
}

// Main function for splitting, merging, applying effects, and resizing
void createCalibratedMergedPNG(const std::string& txtFilePath, const std::string& mergedPngFilePath, int linesToProcess, int desiredHeight, int desiredWidth, std::vector<std::vector<uint16_t>>& imageData) {
    std::ifstream inFile(txtFilePath);
    if (!inFile) {
        std::cerr << "Error opening text file: " << txtFilePath << std::endl;
        return;
    }
    std::vector<std::vector<uint16_t>> data;
    std::string line;

    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<uint16_t> row;
        uint16_t value;

        while (ss >> value) {
            row.push_back(value);
        }
        data.push_back(row);
    }
    inFile.close();

    if (data.empty()) {
        std::cerr << "Error: No data found in the file." << std::endl;
        return;
    }

    int dataHeight = data.size();
    int dataWidth = data[0].size();
    int partWidth = dataWidth / 4;  // 4 parts: low-low, low-high, high-low, high-high

    // Process X and Y axis
    processXAxis(data);
    processYAxis(data, linesToProcess);

    // Split into 4 parts
    std::vector<std::vector<uint16_t>> lowLowImage(dataHeight, std::vector<uint16_t>(partWidth, 0));
    std::vector<std::vector<uint16_t>> lowHighImage(dataHeight, std::vector<uint16_t>(partWidth, 0));
    std::vector<std::vector<uint16_t>> highLowImage(dataHeight, std::vector<uint16_t>(partWidth, 0));
    std::vector<std::vector<uint16_t>> highHighImage(dataHeight, std::vector<uint16_t>(partWidth, 0));

    for (int y = 0; y < dataHeight; ++y) {
        std::copy(data[y].begin(), data[y].begin() + partWidth, lowLowImage[y].begin());
        std::copy(data[y].begin() + partWidth, data[y].begin() + 2 * partWidth, lowHighImage[y].begin());
        std::copy(data[y].begin() + 2 * partWidth, data[y].begin() + 3 * partWidth, highLowImage[y].begin());
        std::copy(data[y].begin() + 3 * partWidth, data[y].end(), highHighImage[y].begin());
    }
    // Merge parts into a new image
    std::vector<std::vector<uint16_t>> mergedImage(dataHeight, std::vector<uint16_t>(partWidth, 0));
    mergeParts(lowLowImage, lowHighImage, highLowImage, highHighImage, mergedImage, dataHeight, partWidth);

    // Apply contrast stretching
    contrastStretching(mergedImage, linesToProcess);

    // Resize using bilinear interpolation
    if (desiredHeight > dataHeight || desiredWidth != partWidth) {
        mergedImage = resizeImageBilinear(mergedImage, desiredHeight, desiredWidth);
    }
    imageData = mergedImage; // Save the image data
    // Save the final PNG
    savePNG(mergedPngFilePath, mergedImage, desiredWidth, desiredHeight);
}

class ImageProcessor : public QWidget {
    Q_OBJECT

public:
    ImageProcessor(QWidget *parent = nullptr) : QWidget(parent), isSelecting(false), darkPixelThreshold(2048) {
        QPushButton *browseBtn = new QPushButton("Browse File", this);
        QPushButton *resizeBtn = new QPushButton("Resize Image", this);
        QPushButton *contrastBtn = new QPushButton("Apply Contrast Stretching", this);
        QPushButton *sharpenBtn = new QPushButton("Apply Sharpening", this);
        QPushButton *highPassBtn = new QPushButton("Apply High-pass Filtering", this);
        QPushButton *gammaBtn = new QPushButton("Apply Gamma Correction", this);
        QPushButton *rotateBtn = new QPushButton("Rotate Image", this);
        QPushButton *applyRegionContrastBtn = new QPushButton("Apply Contrast Stretching to Selected Region", this);

        QLabel *thresholdLabel = new QLabel("Dark Pixel Threshold:", this); // Add a label for the slider
        thresholdSlider = new QSlider(Qt::Horizontal, this); // Add a horizontal slider for threshold adjustment
        thresholdSlider->setRange(0, 65535); // Set the slider range
        thresholdSlider->setValue(darkPixelThreshold); // Set the initial value to 2048

        QLabel *scalingFactorLabel = new QLabel("Contrast Scaling Factor:", this);
        scalingFactorSlider = new QSlider(Qt::Horizontal, this);
        scalingFactorSlider->setRange(0, 100); // Set the range from 0% to 100%
        scalingFactorSlider->setValue(80);     // Set the initial value to 80%

        QLabel *gammaValueLabel = new QLabel("Gamma Value:", this); // Label for the slider
        gammaSlider = new QSlider(Qt::Horizontal, this);   // Slider for gamma value
        gammaSlider->setRange(50, 200);   // Set range from 0.5 to 2.0 (50-200 scale)
        gammaSlider->setValue(120);       // Set the default gamma value to 1.2 (120 scale)

        pictureLabel = new QLabel(this);
        pictureLabel->setMouseTracking(true);
        pictureLabel->setAlignment(Qt::AlignCenter);
        pictureLabel->installEventFilter(this);  // Install event filter to track mouse events

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QHBoxLayout *sliderLayout = new QHBoxLayout();

        btnLayout->addWidget(browseBtn);
        btnLayout->addWidget(resizeBtn);
        btnLayout->addWidget(contrastBtn);
        btnLayout->addWidget(sharpenBtn);
        btnLayout->addWidget(highPassBtn);
        btnLayout->addWidget(gammaBtn);
        btnLayout->addWidget(rotateBtn);
        btnLayout->addWidget(applyRegionContrastBtn);

        sliderLayout->addWidget(thresholdLabel);
        sliderLayout->addWidget(thresholdSlider);

        sliderLayout->addWidget(scalingFactorLabel);
        sliderLayout->addWidget(scalingFactorSlider);

        sliderLayout->addWidget(gammaValueLabel);
        sliderLayout->addWidget(gammaSlider);

        mainLayout->addLayout(btnLayout);
        mainLayout->addLayout(sliderLayout); // Add the slider layout to the main layout
        mainLayout->addWidget(pictureLabel);

        connect(resizeBtn, &QPushButton::clicked, this, &ImageProcessor::onApplyResize);
        connect(contrastBtn, &QPushButton::clicked, this, &ImageProcessor::onApplyContrast);
        connect(sharpenBtn, &QPushButton::clicked, this, &ImageProcessor::onApplySharpening);
        connect(highPassBtn, &QPushButton::clicked, this, &ImageProcessor::onApplyHighPassFiltering);
        connect(gammaBtn, &QPushButton::clicked, this, &ImageProcessor::onApplyGammaCorrection);
        connect(rotateBtn, &QPushButton::clicked, this, &ImageProcessor::onApplyRotation);
        connect(browseBtn, &QPushButton::clicked, this, &ImageProcessor::onBrowseFile);
        connect(applyRegionContrastBtn, &QPushButton::clicked, this, &ImageProcessor::onApplyRegionContrast);
        connect(thresholdSlider, &QSlider::valueChanged, this, &ImageProcessor::onThresholdChanged);

        // Connect gamma slider to label update
        connect(gammaSlider, &QSlider::valueChanged, [=](int value) {
            gammaValueLabel->setText(QString("Gamma Value: %1").arg(value / 100.0f));  // Display the gamma value as a float
        });

        this->setMaximumSize(QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX)); // Remove limits
        this->setMinimumSize(QSize(800, 600)); // Set only minimum size

        setLayout(mainLayout);
    }

protected:
    void drawSelectionRectangle() {
        // Ensure the original pixmap is valid
        if (originalPixmap.isNull()) {
            qWarning() << "Original pixmap is null. Cannot draw selection rectangle.";
            return;  // No pixmap set, nothing to draw on
        }

        // Copy the original pixmap each time before drawing the new rectangle
        QPixmap tempPixmap = originalPixmap.copy();

        // Prepare the painter to draw on the pixmap copy
        QPainter painter(&tempPixmap);
        painter.setPen(QPen(Qt::black, 2, Qt::DashLine));
            //  painter.setPen(QPen(Qt::DashLine)); // Set dashed line style

        // Calculate the scaling factor to map from QLabel coordinates to image coordinates
        QSize labelSize = pictureLabel->size();
        QSize imageSize = tempPixmap.size();
        float scaleFactorX = static_cast<float>(imageSize.width()) / labelSize.width();
        float scaleFactorY = static_cast<float>(imageSize.height()) / labelSize.height();

        // Convert selectionStart and selectionEnd to pixmap coordinates
        QPoint pixmapStart(selectionStart.x() * scaleFactorX, selectionStart.y() * scaleFactorY);
        QPoint pixmapEnd(selectionEnd.x() * scaleFactorX, selectionEnd.y() * scaleFactorY);

        // Draw the rectangle using adjusted coordinates
        painter.drawRect(QRect(pixmapStart, pixmapEnd));

        // Update the label with the new pixmap that includes the drawn rectangle
        pictureLabel->setPixmap(tempPixmap);
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (pictureLabel->underMouse()) {
            isSelecting = true;

            // Save the original pixmap to avoid overwriting
            originalPixmap = pictureLabel->pixmap().copy(); // Save original state

            // Record the selection start point in widget coordinates
            selectionStart = event->pos();

            // Ensure selectionEnd starts at the same position initially
            selectionEnd = selectionStart;

            drawSelectionRectangle(); // Initial draw on mouse press
        }
    }

    void mouseMoveEvent(QMouseEvent *event) override {
        if (isSelecting) {
            // Update the selection end point as the mouse moves
            selectionEnd = event->pos();
            drawSelectionRectangle();  // Redraw the rectangle dynamically as the mouse moves
        }
    }

    void mouseReleaseEvent(QMouseEvent *event) override {
        if (isSelecting) {
            // Finalize the selection rectangle on mouse release
            isSelecting = false;

            // Normalize the selected region and draw the final rectangle
            selectedRegion = QRect(selectionStart, selectionEnd).normalized();
            drawSelectionRectangle();  // Final draw on mouse release
        }
    }

    bool eventFilter(QObject *watched, QEvent *event) override {
        if (watched == pictureLabel) {
            if (event->type() == QEvent::MouseMove && isSelecting) {
                QMouseEvent mouseEvent = static_cast<QMouseEvent>(event);
                // Update the selection end point as the mouse moves
                selectionEnd = mouseEvent->pos();
                drawSelectionRectangle();  // Redraw rectangle as the mouse moves
                return true;
            }
        }
        return QWidget::eventFilter(watched, event);
    }


public slots:
    void onBrowseFile() {
        QString filePath = QFileDialog::getOpenFileName(this, "Open File", "", "Text Files (*.txt)");

        if (!filePath.isEmpty()) {
            txtFilePath = filePath.toStdString();
            loadImage(txtFilePath);
            displayImage(mergedPngFilePath);
        }
    }

    void onApplyResize() {
        if (!txtFilePath.empty() && !imageData.empty()) {
            imageData = resizeImageBilinear(imageData, 3000, 832); // Resize to new dimensions
            savePNG(mergedPngFilePath, imageData, imageData[0].size(), imageData.size());
            displayImage(mergedPngFilePath);
        } else {
            QMessageBox::warning(this, "No File", "Please browse and select a file first.");
        }
    }

    void onApplyContrast() {
        if (!txtFilePath.empty() && !imageData.empty()) {
            contrastStretching(imageData, 70);
            savePNG(mergedPngFilePath, imageData, imageData[0].size(), imageData.size());
            displayImage(mergedPngFilePath);
        } else {
            QMessageBox::warning(this, "No File", "Please browse and select a file first.");
        }
    }

    void onApplySharpening() {
        if (!txtFilePath.empty() && !imageData.empty()) {
            imageData = applyHighPassSharpening(imageData, 5, 1.5f);
            savePNG(mergedPngFilePath, imageData, imageData[0].size(), imageData.size());
            displayImage(mergedPngFilePath);
        } else {
            QMessageBox::warning(this, "No File", "Please browse and select a file first.");
        }
    }

    void onApplyHighPassFiltering() {
        if (!txtFilePath.empty() && !imageData.empty()) {
            imageData = applyHighPassSharpening(imageData, 5, 1.5f); // Reapply high-pass filter with default parameters
            savePNG(mergedPngFilePath, imageData, imageData[0].size(), imageData.size());
            displayImage(mergedPngFilePath);
        } else {
            QMessageBox::warning(this, "No File", "Please browse and select a file first.");
        }
    }

    void onApplyGammaCorrection() {
        if (isProcessing) return;  // Avoid reprocessing if an operation is already running
        isProcessing = true;

        if (imageData.empty()) {
            QMessageBox::warning(this, "Error", "No image data loaded.");
            isProcessing = false;
            return;
        }

        if (!selectedRegion.isNull()) {
            // Ensure image width and height are valid
            if (imageData.empty() || imageData[0].empty()) {
                QMessageBox::warning(this, "Error", "Invalid image dimensions.");
                return;
            }

            QSize widgetSize = pictureLabel->size();
            int imageWidth = imageData[0].size();
            int imageHeight = imageData.size();

            int xStart = std::clamp(selectedRegion.left() * imageWidth / widgetSize.width(), 0, imageWidth - 1);
            int yStart = std::clamp(selectedRegion.top() * imageHeight / widgetSize.height(), 0, imageHeight - 1);
            int xEnd = std::clamp(selectedRegion.right() * imageWidth / widgetSize.width(), 0, imageWidth - 1);
            int yEnd = std::clamp(selectedRegion.bottom() * imageHeight / widgetSize.height(), 0, imageHeight - 1);

            // Get gamma value from slider (dividing by 100 to get float value from 0.5 to 2.0)
            float gammaValue = gammaSlider->value() / 100.0f;

            // Apply gamma correction to the selected region
            gammaCorrectionRegion(imageData, xStart, yStart, xEnd, yEnd, gammaValue);

            // Save the updated image
            savePNG(mergedPngFilePath, imageData, imageWidth, imageHeight);
            displayImage(mergedPngFilePath);
        } else {
            QMessageBox::warning(this, "No Selection", "Please select a region first.");
        }
        isProcessing = false;
    }

    void onApplyRotation() {
        if (!txtFilePath.empty() && !imageData.empty()) {
            // Perform rotation here
            imageData = rotateImage(imageData, 90); // Rotate image by 90 degrees
            savePNG(mergedPngFilePath, imageData, imageData[0].size(), imageData.size());
            displayImage(mergedPngFilePath);
        } else {
            QMessageBox::warning(this, "No File", "Please browse and select a file first.");
        }
        this->resize(800, 600);
    }

    void onThresholdChanged(int value) {
        darkPixelThreshold = value; // Update the threshold when the slider value changes
    }

    void onApplyRegionContrast() {
        if (!txtFilePath.empty() && !imageData.empty() && !selectedRegion.isNull()) {
            // Get QLabel size and image size
            QSize widgetSize = pictureLabel->size();
            int imageWidth = imageData[0].size();
            int imageHeight = imageData.size();

            // Convert widget coordinates to image coordinates
            int xStart = selectedRegion.left() * imageWidth / widgetSize.width();
            int yStart = selectedRegion.top() * imageHeight / widgetSize.height();
            int xEnd = selectedRegion.right() * imageWidth / widgetSize.width();
            int yEnd = selectedRegion.bottom() * imageHeight / widgetSize.height();

            // Get the scaling factor from the slider (convert from 0-100 to 0.0-1.0)
            float scalingFactor = scalingFactorSlider->value() / 100.0f;

            // Apply contrast stretching to the selected region with the new scaling factor
            contrastStretchingRegion(imageData, xStart, yStart, xEnd, yEnd, darkPixelThreshold, scalingFactor);

            // Save the updated image with transparency
            savePNGWithTransparency(mergedPngFilePath, imageData, imageWidth, imageHeight);
            displayImage(mergedPngFilePath);
        } else {
            QMessageBox::warning(this, "No Selection", "Please select a region and load an image first.");
        }
    }

private:
    int darkPixelThreshold;
    std::string txtFilePath;
    std::string mergedPngFilePath = "output.png";
    std::vector<std::vector<uint16_t>> imageData;
    QLabel *pictureLabel;

    QSlider *scalingFactorSlider;  // Declare the scaling factor slider
    QSlider *gammaSlider;
    QSlider *thresholdSlider;

    bool isSelecting;
    QPoint selectionStart, selectionEnd;
    QRect selectedRegion;

    QPixmap originalPixmap;

    bool isProcessing = false;

    void loadImage(const std::string &txtFilePath) {
        createCalibratedMergedPNG(txtFilePath, mergedPngFilePath, 70, 3000, 832, imageData);
    }

    void displayImage(const std::string &filePath) {
        QImage image(QString::fromStdString(filePath));
        if (!image.isNull()) {
            pictureLabel->setPixmap(QPixmap::fromImage(image));
            pictureLabel->setScaledContents(true);
        } else {
            QMessageBox::critical(this, "Error", "Failed to load image.");
        }
    }

    // Function to calculate average intensity for a given region
    float calculateAverageIntensity(const std::vector<std::vector<uint16_t>>& image,
                                    int xStart, int yStart, int xEnd, int yEnd) {
        float sumIntensity = 0.0f;
        int numPixels = 0;

        for (int y = yStart; y <= yEnd; ++y) {
            for (int x = xStart; x <= xEnd; ++x) {
                sumIntensity += image[y][x];
                numPixels++;
            }
        }

        return sumIntensity / numPixels;
    }

    // Function to perform contrast stretching on the object detected within the selected region
    void contrastStretchingRegion(std::vector<std::vector<uint16_t>>& image,
                                  int xStart, int yStart, int xEnd, int yEnd,
                                  float intensityThreshold = 15000.0f, float scalingFactor = 0.8f) {
        // Calculate average intensity for the selected region
        float avgIntensity = calculateAverageIntensity(image, xStart, yStart, xEnd, yEnd);
        std::cout << "Average Intensity: " << avgIntensity << std::endl; // Debug output

        // Set intensity threshold based on average intensity
        float threshold = intensityThreshold * avgIntensity;

        std::cout << "Object Detection Threshold: " << threshold << std::endl; // Debug output

        // Apply contrast stretching to the detected object within the selected region
        for (int y = yStart; y <= yEnd; ++y) {
            for (int x = xStart; x <= xEnd; ++x) {
                if (image[y][x] >= threshold) {
                    // Perform contrast stretching on pixels identified as part of the object
                    uint16_t newIntensity = static_cast<uint16_t>(
                        (image[y][x] - threshold) * (65535.0f / (65535.0f - threshold)) // Stretching calculation
                        );

                    // Clamp values to 16-bit range
                    image[y][x] = std::min(std::max(newIntensity, static_cast<uint16_t>(0)), static_cast<uint16_t>(65535));
                }
            }
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    ImageProcessor window;
    window.setWindowTitle("Image Processor");
    window.resize(500, 300);
    window.show();

    return app.exec();
}

#include "main.moc"