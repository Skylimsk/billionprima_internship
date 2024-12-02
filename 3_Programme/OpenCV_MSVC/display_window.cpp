#include "display_window.h"
#include <QFileDialog>
#include <QMessageBox>

DisplayWindow::DisplayWindow(const QString& title, QWidget* parent, const QPoint& position)
    : QWidget(parent)
    , currentZoom(1.0)
    , originalImage(nullptr) {
    setWindowTitle(title);
    setupUI();
    move(position);  // Set initial position
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
    if (!originalImage) return;

    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Image"), "",
                                                    tr("PNG Files (*.png);;All Files (*)"));

    if (fileName.isEmpty()) return;

    QImage qImage(originalImage->at(0).size(), originalImage->size(), QImage::Format_Grayscale16);
    for (size_t y = 0; y < originalImage->size(); ++y) {
        uint16_t* scanLine = reinterpret_cast<uint16_t*>(qImage.scanLine(y));
        for (size_t x = 0; x < originalImage->at(0).size(); ++x) {
            scanLine[x] = originalImage->at(y)[x];
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

void DisplayWindow::updateImage(const std::vector<std::vector<uint16_t>>& image) {
    if (!originalImage) {
        originalImage = std::make_unique<std::vector<std::vector<uint16_t>>>(image);
    } else {
        *originalImage = image;
    }
    updateDisplayedImage();
}

void DisplayWindow::updateDisplayedImage() {
    if (!originalImage) return;

    QImage qImage(originalImage->at(0).size(), originalImage->size(), QImage::Format_Grayscale16);
    for (size_t y = 0; y < originalImage->size(); ++y) {
        uint16_t* scanLine = reinterpret_cast<uint16_t*>(qImage.scanLine(y));
        for (size_t x = 0; x < originalImage->at(0).size(); ++x) {
            scanLine[x] = originalImage->at(y)[x];
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

void DisplayWindow::setWindowPosition(const QPoint& position) {
    move(position);
}

void DisplayWindow::clear() {
    if (imageLabel) {
        imageLabel->clear();
        imageLabel->setMinimumSize(0, 0);
    }
    originalImage.reset();
    currentZoom = 1.0;
    if (zoomSlider) {
        zoomSlider->setValue(100);
    }
    if (zoomLabel) {
        zoomLabel->setText("Zoom: 100%");
    }
}
