#include "control_panel.h"
#include <QPushButton>
#include <QGroupBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>

ControlPanel::ControlPanel(ImageProcessor& imageProcessor, ImageLabel* imageLabel, QWidget* parent)
    : QWidget(parent), m_imageProcessor(imageProcessor), m_imageLabel(imageLabel)
{
    m_mainLayout = new QVBoxLayout(this);

    // Set a minimum width for the control panel
    this->setMinimumWidth(280);

    setupPixelInfoLabel();

    m_scrollArea = new QScrollArea(this);
    QWidget* scrollWidget = new QWidget(m_scrollArea);
    m_scrollLayout = new QVBoxLayout(scrollWidget);

    setupFileOperations();
    setupBasicOperations();
    setupFilteringOperations();
    setupAdvancedOperations();
    setupGlobalAdjustments();
    setupRegionalAdjustments();

    scrollWidget->setLayout(m_scrollLayout);
    m_scrollArea->setWidget(scrollWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_mainLayout->addWidget(m_scrollArea);
    setLayout(m_mainLayout);
}

void ControlPanel::setupPixelInfoLabel()
{
    QVBoxLayout* infoLayout = new QVBoxLayout();

    m_pixelInfoLabel = new QLabel("Pixel Info: ");
    m_pixelInfoLabel->setFixedHeight(30);
    infoLayout->addWidget(m_pixelInfoLabel);

    m_lastActionLabel = new QLabel("Last Action: None");
    m_lastActionLabel->setFixedHeight(30);
    infoLayout->addWidget(m_lastActionLabel);

    m_mainLayout->addLayout(infoLayout);
}

void ControlPanel::updateLastAction(const QString& action, const QString& parameters)
{
    QString fullAction = action;
    if (!parameters.isEmpty()) {
        fullAction += " - " + parameters;
    }
    m_imageProcessor.setLastAction(fullAction);
    m_lastActionLabel->setText("Last Action: " + m_imageProcessor.getCurrentAction());
}

void ControlPanel::updatePixelInfo(const QPoint& pos)
{
    const auto& finalImage = m_imageProcessor.getFinalImage();
    if (!finalImage.empty()) {
        int x = std::clamp(pos.x(), 0, static_cast<int>(finalImage[0].size()) - 1);
        int y = std::clamp(pos.y(), 0, static_cast<int>(finalImage.size()) - 1);
        uint16_t pixelValue = finalImage[y][x];
        QString info = QString("Pixel Info: X: %1, Y: %2, Value: %3")
                           .arg(x).arg(y)
                           .arg(pixelValue);
        m_pixelInfoLabel->setText(info);
    } else {
        m_pixelInfoLabel->setText("No image loaded");
    }
}

void ControlPanel::setupFileOperations()
{
    createGroupBox("File Operations", {
                                          {"Browse", [this]() {
                                               QString fileName = QFileDialog::getOpenFileName(this, "Open Text File", "", "Text Files (*.txt)");
                                               if (!fileName.isEmpty()) {
                                                   try {
                                                       m_imageProcessor.loadTxtImage(fileName.toStdString());
                                                       updateImageDisplay();
                                                       updateLastAction("Load Image");
                                                       qDebug() << "Image loaded successfully from:" << fileName;
                                                   } catch (const std::exception& e) {
                                                       QMessageBox::critical(this, "Error", QString("Failed to load image: %1").arg(e.what()));
                                                       qDebug() << "Error loading image:" << e.what();
                                                   }
                                               }
                                           }},
                                          {"Save", [this]() {
                                               QString filePath = QFileDialog::getSaveFileName(this, "Save Image", "", "PNG Files (*.png)");
                                               if (!filePath.isEmpty()) {
                                                   m_imageProcessor.saveImage(filePath);
                                                   updateLastAction("Save");
                                               }
                                           }},
                                       {"Revert", [this]() {
                                            QString revertedAction = m_imageProcessor.revertImage();
                                            if (!revertedAction.isEmpty()) {
                                                updateImageDisplay();
                                                updateLastAction(revertedAction);
                                            } else {
                                                QMessageBox::information(this, "Revert", "No more actions to revert.");
                                            }
                                        }}
                                      });
}

void ControlPanel::setupBasicOperations()
{
    createGroupBox("Basic Operations", {
                                           {"Crop", [this]() {
                                                if (m_imageLabel->isRegionSelected()) {
                                                    QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                    QRect normalizedRegion = selectedRegion.normalized();
                                                    if (!normalizedRegion.isEmpty()) {
                                                        m_imageProcessor.cropRegion(normalizedRegion);
                                                        updateImageDisplay();
                                                        m_imageLabel->clearSelection();
                                                        updateLastAction("Crop");
                                                    } else {
                                                        QMessageBox::warning(this, "Crop Error", "Invalid region selected for cropping.");
                                                    }
                                                } else {
                                                    QMessageBox::information(this, "Crop Info", "Please select a region to crop first.");
                                                }
                                            }},
                                           {"Rotate CW", [this]() {
                                                auto rotatedImage = m_imageProcessor.rotateImage(m_imageProcessor.getFinalImage(), 90);
                                                m_imageProcessor.updateAndSaveFinalImage(rotatedImage);
                                                updateImageDisplay();
                                                updateLastAction("Rotate Clockwise");
                                            }},
                                           {"Rotate CCW", [this]() {
                                                auto rotatedImage = m_imageProcessor.rotateImage(m_imageProcessor.getFinalImage(), 270);
                                                m_imageProcessor.updateAndSaveFinalImage(rotatedImage);
                                                updateImageDisplay();
                                                updateLastAction("Rotate CCW");
                                            }},
                                        {"Calibration", [this]() {
                                             auto [linesToProcessY, yOk] = showInputDialog("Calibration", "Enter lines to process for Y-axis:", 10, 1, 1000);
                                             if (!yOk) return;

                                             auto [linesToProcessX, xOk] = showInputDialog("Calibration", "Enter lines to process for X-axis:", 10, 1, 1000);
                                             if (xOk) {
                                                 m_imageProcessor.processYXAxis(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()), linesToProcessY, linesToProcessX);
                                                 updateImageDisplay();
                                                 updateLastAction("Calibration", QString("Y: %1, X: %2").arg(linesToProcessY).arg(linesToProcessX));
                                             }
                                         }},
                                           {"Split & Merge", [this]() {
                                                m_imageProcessor.processAndMergeImageParts();
                                                updateImageDisplay();
                                                updateLastAction("Split & Merge");
                                            }}
                                       });
}

void ControlPanel::setupFilteringOperations()
{
    createGroupBox("Filtering Operations", {
                                            {"Median Filter", [this]() {
                                                 auto [filterKernelSize, ok] = showInputDialog("Median Filter", "Enter kernel size:", 3, 1, 21);
                                                 if (ok) {
                                                     m_imageProcessor.applyMedianFilter(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()), filterKernelSize);
                                                     updateImageDisplay();
                                                     updateLastAction("Median Filter", QString::number(filterKernelSize));
                                                 }
                                             }},
                                               {"High-Pass Filter", [this]() {
                                                    m_imageProcessor.applyHighPassFilter(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()));
                                                    updateImageDisplay();
                                                    updateLastAction("High-Pass Filter");
                                                }},
                                            {"Apply CLAHE", [this]() {
                                                 auto [clipLimit, clipOk] = showInputDialog("CLAHE", "Enter clip limit:", 2.0, 0.1, 10.0);
                                                 if (!clipOk) return;

                                                 auto [tileSize, tileOk] = showInputDialog("CLAHE", "Enter tile size:", 8, 2, 32);
                                                 if (!tileOk) return;

                                                 cv::Mat matImage = m_imageProcessor.vectorToMat(m_imageProcessor.getFinalImage());
                                                 cv::Mat resultImage = m_imageProcessor.applyCLAHE(matImage, clipLimit, cv::Size(tileSize, tileSize));

                                                 m_imageProcessor.updateAndSaveFinalImage(m_imageProcessor.matToVector(resultImage));
                                                 updateImageDisplay();
                                                 updateLastAction("Apply CLAHE", QString("Clip: %1, Tile: %2").arg(clipLimit, 0, 'f', 2).arg(tileSize));
                                             }}
                                           });
}

void ControlPanel::setupAdvancedOperations()
{
    createGroupBox("Advanced Operations", {
                                              {"Stretch", [this]() {
                                                   auto [stretchFactor, ok] = showInputDialog("Stretch Factor", "Enter stretch factor:", 1.5, 0.1, 10.0);
                                                   if (ok) {
                                                       m_imageProcessor.stretchImageY(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()), stretchFactor);
                                                       updateImageDisplay();
                                                       updateLastAction("Stretch", QString::number(stretchFactor, 'f', 2));
                                                   }
                                               }},
                                              {"Padding", [this]() {
                                                   auto [paddingSize, ok] = showInputDialog("Padding Size", "Enter padding size:", 10, 1, 1000);
                                                   if (ok) {
                                                       auto paddedImage = m_imageProcessor.addPadding(m_imageProcessor.getFinalImage(), paddingSize);
                                                       m_imageProcessor.updateAndSaveFinalImage(paddedImage);
                                                       updateImageDisplay();
                                                       updateLastAction("Padding", QString::number(paddingSize));
                                                   }
                                               }},
                                              {"Apply Distortion", [this]() {
                                                   QStringList directions = {"Left", "Right", "Top", "Bottom"};
                                                   bool ok;
                                                   QString selectedDirection = QInputDialog::getItem(this, "Distortion Direction", "Select direction:", directions, 0, false, &ok);
                                                   if (!ok) return;

                                                   auto [distortionFactor, factorOk] = showInputDialog("Distortion Factor", "Enter distortion factor:", 1.5, 1.0, 100.0);
                                                   if (factorOk) {
                                                       auto distortedImage = m_imageProcessor.distortImage(m_imageProcessor.getFinalImage(), distortionFactor, selectedDirection.toStdString());
                                                       m_imageProcessor.updateAndSaveFinalImage(distortedImage);
                                                       updateImageDisplay();
                                                       updateLastAction("Distortion", QString("%1 - %2").arg(selectedDirection).arg(distortionFactor, 0, 'f', 2));
                                                   }
                                               }}
                                          });
}

void ControlPanel::setupGlobalAdjustments()
{
    createGroupBox("Global Adjustments", {
                                             {"Overall Gamma", [this]() {
                                                  auto [gammaValue, ok] = showInputDialog("Overall Gamma", "Enter gamma value:", 1.0, 0.1, 10.0);
                                                  if (ok) {
                                                      m_imageProcessor.adjustGammaOverall(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()), gammaValue);
                                                      updateImageDisplay();
                                                      updateLastAction("Overall Gamma", QString::number(gammaValue, 'f', 2));
                                                  }
                                              }},
                                             {"Overall Sharpen", [this]() {
                                                  auto [sharpenStrength, ok] = showInputDialog("Overall Sharpen", "Enter sharpen strength:", 1.0, 0.1, 10.0);
                                                  if (ok) {
                                                      m_imageProcessor.sharpenImage(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()), sharpenStrength);
                                                      updateImageDisplay();
                                                      updateLastAction("Overall Sharpen", QString::number(sharpenStrength, 'f', 2));
                                                  }
                                              }},
                                             {"Overall Contrast", [this]() {
                                                  auto [contrastFactor, ok] = showInputDialog("Overall Contrast", "Enter contrast factor:", 1.0, 0.1, 10.0);
                                                  if (ok) {
                                                      m_imageProcessor.adjustContrast(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()), contrastFactor);
                                                      updateImageDisplay();
                                                      updateLastAction("Overall Contrast", QString::number(contrastFactor, 'f', 2));
                                                  }
                                              }}
                                         });
}

void ControlPanel::setupRegionalAdjustments()
{
    createGroupBox("Regional Adjustments", {
                                               {"Region Gamma", [this]() {
                                                    if (m_imageLabel->isRegionSelected()) {
                                                        auto [gamma, ok] = showInputDialog("Region Gamma", "Enter gamma value:", 1.0, 0.1, 10.0);
                                                        if (ok) {
                                                            QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                            m_imageProcessor.adjustGammaForSelectedRegion(gamma, selectedRegion);
                                                            updateImageDisplay();
                                                            updateLastAction("Region Gamma", QString::number(gamma, 'f', 2));
                                                        }
                                                    } else {
                                                        QMessageBox::information(this, "Region Gamma", "Please select a region first.");
                                                    }
                                                }},
                                               {"Region Sharpen", [this]() {
                                                    if (m_imageLabel->isRegionSelected()) {
                                                        auto [sharpenStrength, ok] = showInputDialog("Region Sharpen", "Enter sharpen strength:", 0.5, 0.1, 5.0);
                                                        if (ok) {
                                                            QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                            m_imageProcessor.applySharpenToRegion(sharpenStrength, selectedRegion);
                                                            updateImageDisplay();
                                                            updateLastAction("Region Sharpen", QString::number(sharpenStrength, 'f', 2));
                                                        }
                                                    } else {
                                                        QMessageBox::information(this, "Region Sharpen", "Please select a region first.");
                                                    }
                                                }},
                                               {"Region Contrast", [this]() {
                                                    if (m_imageLabel->isRegionSelected()) {
                                                        auto [contrastFactor, ok] = showInputDialog("Region Contrast", "Enter contrast factor:", 1.5, 0.1, 5.0);
                                                        if (ok) {
                                                            QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                            m_imageProcessor.applyContrastToRegion(contrastFactor, selectedRegion);
                                                            updateImageDisplay();
                                                            updateLastAction("Region Contrast", QString::number(contrastFactor, 'f', 2));
                                                        }
                                                    } else {
                                                        QMessageBox::information(this, "Region Contrast", "Please select a region first.");
                                                    }
                                                }}
                                           });
}

std::pair<double, bool> ControlPanel::showInputDialog(const QString& title, const QString& label, double defaultValue, double min, double max)
{
    bool ok;
    double value = QInputDialog::getDouble(this, title, label, defaultValue, min, max, 2, &ok);
    return std::make_pair(value, ok);
}

void ControlPanel::createGroupBox(const QString& title, const std::vector<std::pair<QString, std::function<void()>>>& buttons)
{
    QGroupBox* groupBox = new QGroupBox(title);
    QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);

    // Increase the minimum width of the group box
    const int groupBoxMinWidth = 250;
    const int buttonHeight = 35;

    groupBox->setMinimumWidth(groupBoxMinWidth);
    groupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    for (const auto& button_pair : buttons) {
        QPushButton* button = new QPushButton(button_pair.first);
        button->setFixedHeight(buttonHeight);
        button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        groupLayout->addWidget(button);

        connect(button, &QPushButton::clicked, button_pair.second);
    }

    groupLayout->setSpacing(5);
    groupLayout->setContentsMargins(5, 10, 5, 10);

    m_scrollLayout->addWidget(groupBox);
}

void ControlPanel::updateImageDisplay()
{
    const auto& finalImage = m_imageProcessor.getFinalImage();
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
        if (m_imageLabel->isRegionSelected()) {
            QPainter painter(&pixmap);
            painter.setPen(QPen(Qt::red, 2));  // Red color, 2-pixel thickness
            painter.drawRect(m_imageLabel->getSelectedRegion());
        }

        // Set the pixmap to the QLabel without scaling
        m_imageLabel->setPixmap(pixmap);
        m_imageLabel->setFixedSize(pixmap.size());  // Adjust the QLabel size to match the image
    }
}


