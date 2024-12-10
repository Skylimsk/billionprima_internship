#include "display_window.h"
#include <QFileDialog>
#include <QMessageBox>
#include <algorithm>

DisplayWindow::DisplayWindow(const QString& title, QWidget* parent, const QPoint& position)
    : QWidget(parent)
    , currentZoom(1.0)
    , originalImage(nullptr)
    , imageHeight(0)
    , imageWidth(0) {
    setWindowTitle(title);
    setupUI();
    move(position);  // Set initial position
}

DisplayWindow::~DisplayWindow() {
    cleanupImage();
}

void DisplayWindow::setupUI() {
    mainLayout = new QVBoxLayout(this);

    // Controls layout
    controlsLayout = new QHBoxLayout();

    // Save button
    saveButton = new QPushButton("Save", this);
    connect(saveButton, &QPushButton::clicked, this, &DisplayWindow::saveImage);
    controlsLayout->addWidget(saveButton);

    // Zoom slider
    zoomSlider = new QSlider(Qt::Horizontal, this);
    zoomSlider->setRange(10, 200);  // 10% to 200%
    zoomSlider->setValue(100);      // 100% default
    connect(zoomSlider, &QSlider::valueChanged, this, &DisplayWindow::zoomChanged);
    controlsLayout->addWidget(zoomSlider);

    // Zoom label
    zoomLabel = new QLabel("Zoom: 100%", this);
    controlsLayout->addWidget(zoomLabel);

    mainLayout->addLayout(controlsLayout);

    // Image display
    scrollArea = new QScrollArea(this);
    imageLabel = new QLabel(scrollArea);
    imageLabel->setAlignment(Qt::AlignCenter);
    scrollArea->setWidget(imageLabel);
    scrollArea->setWidgetResizable(true);
    mainLayout->addWidget(scrollArea);

    // Set initial size
    resize(800, 600);
}

void DisplayWindow::saveImage() {
    if (!originalImage || imageHeight <= 0 || imageWidth <= 0) return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Image"), "",
                                                    tr("PNG Files (*.png);;All Files (*)"));

    if (fileName.isEmpty()) return;

    QImage qImage(imageWidth, imageHeight, QImage::Format_Grayscale16);
    for (int y = 0; y < imageHeight; ++y) {
        uint16_t* scanLine = reinterpret_cast<uint16_t*>(qImage.scanLine(y));
        for (int x = 0; x < imageWidth; ++x) {
            scanLine[x] = static_cast<uint16_t>(std::clamp(originalImage[y][x], 0.0, 65535.0));
        }
    }

    if (qImage.save(fileName)) {
        QMessageBox::information(this, tr("Success"),
                                 tr("Image saved successfully."));
    } else {
        QMessageBox::warning(this, tr("Error"),
                             tr("Failed to save image."));
    }
}

void DisplayWindow::zoomChanged(int value) {
    currentZoom = value / 100.0;
    zoomLabel->setText(QString("Zoom: %1%").arg(value));
    updateDisplayedImage();
}

void DisplayWindow::updateImage(double** image, int height, int width) {
    if (!image || height <= 0 || width <= 0) return;

    // Clean up old image if it exists
    cleanupImage();

    // Allocate new image memory
    originalImage = new double*[height];
    for (int i = 0; i < height; i++) {
        originalImage[i] = new double[width];
        // Copy data
        for (int j = 0; j < width; j++) {
            originalImage[i][j] = image[i][j];
        }
    }

    imageHeight = height;
    imageWidth = width;
    updateDisplayedImage();
}

void DisplayWindow::updateDisplayedImage() {
    if (!originalImage || imageHeight <= 0 || imageWidth <= 0) return;

    QImage qImage(imageWidth, imageHeight, QImage::Format_Grayscale16);
    for (int y = 0; y < imageHeight; ++y) {
        uint16_t* scanLine = reinterpret_cast<uint16_t*>(qImage.scanLine(y));
        for (int x = 0; x < imageWidth; ++x) {
            scanLine[x] = static_cast<uint16_t>(std::clamp(originalImage[y][x], 0.0, 65535.0));
        }
    }

    // Calculate new size based on zoom
    QSize newSize = qImage.size() * currentZoom;

    // Update image label with scaled image
    imageLabel->setPixmap(QPixmap::fromImage(qImage.scaled(
        newSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation)));

    // Update scroll area if needed
    imageLabel->setMinimumSize(newSize);
}

void DisplayWindow::cleanupImage() {
    if (originalImage) {
        for (int i = 0; i < imageHeight; i++) {
            delete[] originalImage[i];
        }
        delete[] originalImage;
        originalImage = nullptr;
    }
    imageHeight = 0;
    imageWidth = 0;
}

void DisplayWindow::setWindowPosition(const QPoint& position) {
    move(position);
}

void DisplayWindow::clear() {
    if (imageLabel) {
        imageLabel->clear();
        imageLabel->setMinimumSize(0, 0);
    }
    cleanupImage();
    currentZoom = 1.0;
    if (zoomSlider) {
        zoomSlider->setValue(100);
    }
    if (zoomLabel) {
        zoomLabel->setText("Zoom: 100%");
    }
}
