#include "imageprocessor.h"
#include <QMessageBox>

ImageProcessor::ImageProcessor(QWidget *parent)
    : QMainWindow(parent), regionSelected(false) {
    setupUI();
    connectSignalsSlots();
}

void ImageProcessor::setupUI() {
    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    imageLabel = new QLabel(this);
    imageLabel->setMinimumSize(400, 400);
    imageLabel->setAlignment(Qt::AlignCenter);
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    loadButton = new QPushButton("Load Image Text File", this);
    saveButton = new QPushButton("Save Image", this);

    layout->addWidget(imageLabel);
    layout->addWidget(loadButton);
    layout->addWidget(saveButton);

    setWindowTitle("Image Processor");
    resize(800, 600);
}

void ImageProcessor::connectSignalsSlots() {
    connect(loadButton, &QPushButton::clicked, this, &ImageProcessor::loadFile);
    connect(saveButton, &QPushButton::clicked, this, &ImageProcessor::saveFile);
}

void ImageProcessor::loadFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Text File"), "",
                                                    tr("Text Files (*.txt);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    loadTxtImage(fileName.toStdString());
    updateDisplayImage();
}

void ImageProcessor::loadTxtImage(const std::string& txtFilePath) {
    std::ifstream inFile(txtFilePath);
    if (!inFile.is_open()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot open file: %1").arg(QString::fromStdString(txtFilePath)));
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
            row.push_back(static_cast<uint16_t>(std::min(value, static_cast<uint32_t>(65535))));
        }
        if (!row.empty()) {
            maxWidth = std::max(maxWidth, row.size());
            imgData.push_back(row);
        }
    }
    inFile.close();

    if (imgData.empty()) {
        QMessageBox::warning(this, tr("Error"), tr("No valid data found in the file."));
        return;
    }

    // Pad rows to same width
    for (auto& row : imgData) {
        if (row.size() < maxWidth) {
            row.resize(maxWidth, row.empty() ? 0 : row.back());
        }
    }

    originalImg = imgData;
    finalImage = imgData;
    saveCurrentState();

    selectedRegion = QRect();
    regionSelected = false;
}

void ImageProcessor::updateDisplayImage() {
    if (imgData.empty()) return;

    int height = imgData.size();
    int width = imgData[0].size();

    // Create QImage from the 16-bit data
    QImage image(width, height, QImage::Format_Grayscale8);

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Convert 16-bit value to 8-bit for display
            uint8_t pixelValue = static_cast<uint8_t>(imgData[y][x] >> 8);
            image.setPixel(x, y, qRgb(pixelValue, pixelValue, pixelValue));
        }
    }

    displayImage = image;
    imageLabel->setPixmap(QPixmap::fromImage(displayImage));
}

void ImageProcessor::saveFile() {
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Image"), "",
                                                    tr("PNG Images (*.png);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    if (!displayImage.save(fileName)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Cannot save image %1").arg(fileName));
    }
}

void ImageProcessor::saveCurrentState() {
    // Implementation for maintaining image state history if needed
}
