#include "control_panel.h"
#include "adjustments.h"
#include <QPushButton>
#include <QGroupBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>

#ifdef _MSC_VER
#pragma warning(disable: 4068) // unknown pragma
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4244) // conversion from 'double' to 'float', possible loss of data
#pragma warning(disable: 4458) // declaration hides class member
#pragma warning(disable: 4005) // macro redefinition
#endif

ControlPanel::ControlPanel(ImageProcessor& imageProcessor, ImageLabel* imageLabel, QWidget* parent)
    : QWidget(parent),
    m_imageProcessor(imageProcessor),
    m_imageLabel(imageLabel),
    m_lastGpuTime(-1),
    m_lastCpuTime(-1),
    m_hasCpuClaheTime(false),
    m_hasGpuClaheTime(false),
    m_preZoomLevel(1.0f),
    m_zoomControlsGroup(nullptr),
    m_zoomModeActive(false)
{
    m_mainLayout = new QVBoxLayout(this);
    this->setMinimumWidth(280);

    setupPixelInfoLabel();

    m_scrollArea = new QScrollArea(this);
    QWidget* scrollWidget = new QWidget(m_scrollArea);
    m_scrollLayout = new QVBoxLayout(scrollWidget);

    setupFileOperations();
    setupBasicOperations();
    setupZoomControls();  // Initialize zoom controls (but don't add to layout yet)
    setupFilteringOperations();
    setupAdvancedOperations();
    setupCLAHEOperations();
    setupBlackLineDetection();
    setupGlobalAdjustments();
    setupRegionalAdjustments();

    scrollWidget->setLayout(m_scrollLayout);
    m_scrollArea->setWidget(scrollWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_mainLayout->addWidget(m_scrollArea);
    setLayout(m_mainLayout);

    m_zoomWarningBox = new QMessageBox(this);
    m_zoomWarningBox->setIcon(QMessageBox::Warning);
    m_zoomWarningBox->setWindowTitle("Zoom Mode Active");
    m_zoomWarningBox->setText("No functions can be applied while in Zoom Mode.\nPlease exit Zoom Mode first.");
    m_zoomWarningBox->setStandardButtons(QMessageBox::Ok);
}

bool ControlPanel::checkZoomMode() {
    if (m_zoomModeActive) {
        m_zoomWarningBox->exec();
        return true;
    }
    return false;
}

void ControlPanel::setupPixelInfoLabel()
{
    QVBoxLayout* infoLayout = new QVBoxLayout();

    m_pixelInfoLabel = new QLabel("Pixel Info: ");
    m_pixelInfoLabel->setFixedHeight(30);
    infoLayout->addWidget(m_pixelInfoLabel);

    m_lastActionLabel = new QLabel("Last Action: None");
    m_lastActionLabel->setFixedHeight(25);
    infoLayout->addWidget(m_lastActionLabel);

    m_lastActionParamsLabel = new QLabel("");
    m_lastActionParamsLabel->setFixedHeight(25);
    m_lastActionParamsLabel->setStyleSheet("color: black;");
    m_lastActionParamsLabel->setWordWrap(true);
    infoLayout->addWidget(m_lastActionParamsLabel);

    m_gpuTimingLabel = new QLabel("");
    m_gpuTimingLabel->setFixedHeight(30);
    m_gpuTimingLabel->setVisible(false);
    infoLayout->addWidget(m_gpuTimingLabel);

    m_cpuTimingLabel = new QLabel("");
    m_cpuTimingLabel->setFixedHeight(30);
    m_cpuTimingLabel->setVisible(false);
    infoLayout->addWidget(m_cpuTimingLabel);

    QPushButton* toggleHistogramButton = new QPushButton("Show Histogram");
    toggleHistogramButton->setFixedHeight(30);
    toggleHistogramButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #f0f0f0;"
        "    border: 1px solid #c0c0c0;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e0e0e0;"
        "}"
        );
    infoLayout->addWidget(toggleHistogramButton);

    // Create histogram with explicit sizing
    m_histogram = new Histogram(this);
    m_histogram->setMinimumWidth(250);
    m_histogram->setVisible(false); // Start with histogram hidden
    infoLayout->addWidget(m_histogram);

    // Connect the toggle button
    connect(toggleHistogramButton, &QPushButton::clicked, this, [this, toggleHistogramButton]() {
        bool isCurrentlyVisible = m_histogram->isVisible();
        m_histogram->setVisible(!isCurrentlyVisible);
        toggleHistogramButton->setText(isCurrentlyVisible ? "Show Histogram" : "Hide Histogram");

        if (!isCurrentlyVisible) {
            const auto& finalImage = m_imageProcessor.getFinalImage();
            if (!finalImage.empty()) {
                m_histogram->updateHistogram(finalImage);
            }
        }

        // Force layout update
        m_mainLayout->invalidate();
        updateGeometry();
    });

    m_mainLayout->addLayout(infoLayout);

    m_lastGpuTime = -1;
    m_lastCpuTime = -1;
}

void ControlPanel::updateLastAction(const QString& action, const QString& parameters)
{
    m_lastActionLabel->setText("Last Action: " + action);
    if (parameters.isEmpty()) {
        m_lastActionParamsLabel->clear();
        m_lastActionParamsLabel->setVisible(false);
    } else {
        QString prefix;
        if (action == "Load Image" || action == "Save Image") {
            prefix = "File Name: ";
        } else if (action == "Detect Black Lines" || action == "Remove Black Lines") {
            prefix = "Line Info: ";
        } else if (action ==  "Split & Merge"){
            prefix = "";
        } else if (action == "Zoom In" || action == "Zoom Out" || action == "Reset Zoom"){
            prefix = "Zoom Factor: ";
        } else if (action == "Zoom Mode"){
            prefix = "Status: ";
        } else {
            prefix = "Parameters: ";
        }
        m_lastActionParamsLabel->setText(prefix + parameters);
        m_lastActionParamsLabel->setVisible(true);
    }
    m_imageProcessor.setLastAction(action, parameters);
}

void ControlPanel::handleRevert() {
    // Update display of both timings based on stored values
    if (m_hasGpuClaheTime && m_lastGpuTime >= 0) {
        m_gpuTimingLabel->setText(QString("CLAHE Processing Time (GPU): %1 ms").arg(m_lastGpuTime, 0, 'f', 2));
        m_gpuTimingLabel->setVisible(true);
    } else {
        m_gpuTimingLabel->setVisible(false);
    }

    if (m_hasCpuClaheTime && m_lastCpuTime >= 0) {
        m_cpuTimingLabel->setText(QString("CLAHE Processing Time (CPU): %1 ms").arg(m_lastCpuTime, 0, 'f', 2));
        m_cpuTimingLabel->setVisible(true);
    } else {
        m_cpuTimingLabel->setVisible(false);
    }
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

void ControlPanel::setupZoomControls() {
    m_zoomControlsGroup = new QGroupBox("Zoom Controls");
    QVBoxLayout* zoomLayout = new QVBoxLayout(m_zoomControlsGroup);

    // Create zoom control buttons with consistent styling
    QPushButton* zoomInBtn = new QPushButton("Zoom In");
    QPushButton* zoomOutBtn = new QPushButton("Zoom Out");
    QPushButton* resetZoomBtn = new QPushButton("Reset Zoom");

    // Set fixed height for buttons to match other controls
    const int buttonHeight = 35;
    zoomInBtn->setFixedHeight(buttonHeight);
    zoomOutBtn->setFixedHeight(buttonHeight);
    resetZoomBtn->setFixedHeight(buttonHeight);

    // Add tooltips
    zoomInBtn->setToolTip("Increase zoom level by 20%");
    zoomOutBtn->setToolTip("Decrease zoom level by 20%");
    resetZoomBtn->setToolTip("Reset to original size (100%)");

    // Add buttons to layout
    zoomLayout->addWidget(zoomInBtn);
    zoomLayout->addWidget(zoomOutBtn);
    zoomLayout->addWidget(resetZoomBtn);

    // Connect zoom buttons
    connect(zoomInBtn, &QPushButton::clicked, [this]() {
        m_imageLabel->clearSelection();
        m_imageProcessor.zoomIn();
        updateImageDisplay();
        updateLastAction("Zoom In", QString("%1x").arg(m_imageProcessor.getZoomLevel(), 0, 'f', 2));
    });

    connect(zoomOutBtn, &QPushButton::clicked, [this]() {
        m_imageLabel->clearSelection();
        m_imageProcessor.zoomOut();
        updateImageDisplay();
        updateLastAction("Zoom Out", QString("%1x").arg(m_imageProcessor.getZoomLevel(), 0, 'f', 2));
    });

    connect(resetZoomBtn, &QPushButton::clicked, [this]() {
        m_imageLabel->clearSelection();
        m_imageProcessor.resetZoom();
        updateImageDisplay();
        updateLastAction("Reset Zoom", "1x");
    });

    // Initially hide zoom controls
    m_zoomControlsGroup->hide();
}

void ControlPanel::toggleZoomMode(bool active) {
    if (m_zoomModeActive == active) return;  // No change needed

    m_zoomModeActive = active;

    if (active) {
        // Store current zoom level
        m_preZoomLevel = m_imageProcessor.getZoomLevel();

        // Find the Basic Operations group and insert zoom controls after it
        int basicOpIndex = -1;
        for (int i = 0; i < m_scrollLayout->count(); ++i) {
            QGroupBox* box = qobject_cast<QGroupBox*>(m_scrollLayout->itemAt(i)->widget());
            if (box && box->title() == "Basic Operations") {
                basicOpIndex = i;
                break;
            }
        }

        if (basicOpIndex >= 0) {
            m_scrollLayout->insertWidget(basicOpIndex + 1, m_zoomControlsGroup);
        }
        m_zoomControlsGroup->show();

        updateLastAction("Zoom Mode", "Zoom Activated");
    } else {
        // Restore pre-zoom state
        m_imageProcessor.setZoomLevel(m_preZoomLevel);
        updateImageDisplay();

        // Hide and remove zoom controls
        m_zoomControlsGroup->hide();
        m_scrollLayout->removeWidget(m_zoomControlsGroup);

        updateLastAction("Zoom Mode", "Zoom Deactivated");
    }
}

void ControlPanel::setupFileOperations()
{
    createGroupBox("File Operations", {
                                          {"Browse", [this]() {
                                               QString fileName = QFileDialog::getOpenFileName(this, "Open Text File", "", "Text Files (*.txt)");
                                               if (!fileName.isEmpty()) {
                                                   try {
                                                       resetDetectedLines();
                                                       m_imageProcessor.loadTxtImage(fileName.toStdString());
                                                       m_imageLabel->clearSelection();
                                                       updateImageDisplay();
                                                       // Extract just the file name without path
                                                       QFileInfo fileInfo(fileName);
                                                       updateLastAction("Load Image", fileInfo.fileName());
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
                                                   // Extract just the file name without path
                                                   QFileInfo fileInfo(filePath);
                                                   updateLastAction("Save Image",  fileInfo.fileName());
                                               }
                                           }},
                                          {"Revert", [this]() {
                                               QString revertedAction = m_imageProcessor.revertImage();
                                               if (!revertedAction.isEmpty()) {
                                                   updateImageDisplay();
                                                   handleRevert();

                                                   // Split action and parameters if they exist
                                                   QString lastAction = m_imageProcessor.getLastActionRecord().action;
                                                   QString lastParams = m_imageProcessor.getLastActionRecord().parameters;

                                                   m_lastActionLabel->setText("Last Action: " + lastAction);
                                                   if (lastParams.isEmpty()) {
                                                       m_lastActionParamsLabel->clear();
                                                       m_lastActionParamsLabel->setVisible(false);
                                                   } else {
                                                       m_lastActionParamsLabel->setText(lastParams);
                                                       m_lastActionParamsLabel->setVisible(true);
                                                   }
                                               } else {
                                                   QMessageBox::information(this, "Revert", "No more actions to revert.");
                                                   m_lastActionLabel->setText("Last Action: None");
                                                   m_lastActionParamsLabel->clear();
                                                   m_lastActionParamsLabel->setVisible(false);
                                               }
                                           }}
                                      });
}

void ControlPanel::setupBasicOperations()
{
    createGroupBox("Basic Operations", {
                                            {"Zoom", [this]() {
                                             toggleZoomMode(!m_zoomModeActive);
                                            }},
                                           {"Crop", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
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
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
                                                auto rotatedImage = m_imageProcessor.rotateImage(m_imageProcessor.getFinalImage(), 90);
                                                m_imageProcessor.updateAndSaveFinalImage(rotatedImage);
                                                m_imageLabel->clearSelection();
                                                updateImageDisplay();
                                                updateLastAction("Rotate Clockwise");
                                            }},
                                           {"Rotate CCW", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
                                                auto rotatedImage = m_imageProcessor.rotateImage(m_imageProcessor.getFinalImage(), 270);
                                                m_imageProcessor.updateAndSaveFinalImage(rotatedImage);
                                                m_imageLabel->clearSelection();
                                                updateImageDisplay();
                                                updateLastAction("Rotate CCW");
                                            }},
                                           {"Calibration", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
                                                auto [linesToProcessY, yOk] = showInputDialog("Calibration", "Enter lines to process for Y-axis:", 10, 1, 1000);
                                                if (!yOk) return;

                                                auto [linesToProcessX, xOk] = showInputDialog("Calibration", "Enter lines to process for X-axis:", 10, 1, 1000);
                                                if (xOk) {
                                                    m_imageProcessor.processYXAxis(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()), linesToProcessY, linesToProcessX);
                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("Calibration", QString("Y: %1, X: %2").arg(linesToProcessY).arg(linesToProcessX));
                                                }
                                            }},
                                        {"Split & Merge", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();

                                             // Create dialog for options
                                             QDialog dialog(this);
                                             dialog.setWindowTitle("Split & Merge Options");
                                             QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                             // Split mode selection
                                             QGroupBox* splitBox = new QGroupBox("Split Mode");
                                             QVBoxLayout* splitLayout = new QVBoxLayout(splitBox);
                                             QRadioButton* allPartsRadio = new QRadioButton("All Parts");
                                             QRadioButton* leftMostRadio = new QRadioButton("Left Most (Parts 1-2)");
                                             QRadioButton* rightMostRadio = new QRadioButton("Right Most (Parts 3-4)");
                                             allPartsRadio->setChecked(true);
                                             splitLayout->addWidget(allPartsRadio);
                                             splitLayout->addWidget(leftMostRadio);
                                             splitLayout->addWidget(rightMostRadio);
                                             layout->addWidget(splitBox);

                                             // Merge method selection
                                             QGroupBox* mergeBox = new QGroupBox("Merge Method");
                                             QVBoxLayout* mergeLayout = new QVBoxLayout(mergeBox);
                                             QRadioButton* weightedAvgRadio = new QRadioButton("Weighted Average");
                                             QRadioButton* minValueRadio = new QRadioButton("Minimum Value");
                                             weightedAvgRadio->setChecked(true);
                                             mergeLayout->addWidget(weightedAvgRadio);
                                             mergeLayout->addWidget(minValueRadio);
                                             layout->addWidget(mergeBox);

                                             // Add OK and Cancel buttons
                                             QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                 Qt::Horizontal, &dialog);
                                             layout->addWidget(buttonBox);

                                             connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                             connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                             if (dialog.exec() == QDialog::Accepted) {
                                                 ImageProcessor::SplitMode splitMode;
                                                 if (leftMostRadio->isChecked()) {
                                                     splitMode = ImageProcessor::SplitMode::LEFT_MOST;
                                                 } else if (rightMostRadio->isChecked()) {
                                                     splitMode = ImageProcessor::SplitMode::RIGHT_MOST;
                                                 } else {
                                                     splitMode = ImageProcessor::SplitMode::ALL_PARTS;
                                                 }

                                                 ImageProcessor::MergeMethod mergeMethod = minValueRadio->isChecked() ?
                                                                                               ImageProcessor::MergeMethod::MINIMUM_VALUE :
                                                                                               ImageProcessor::MergeMethod::WEIGHTED_AVERAGE;

                                                 m_imageProcessor.processAndMergeImageParts(splitMode, mergeMethod);
                                                 m_imageLabel->clearSelection();
                                                 updateImageDisplay();

                                                 // Update last action with the selected options
                                                 QString modeStr;
                                                 switch (splitMode) {
                                                 case ImageProcessor::SplitMode::LEFT_MOST:
                                                     modeStr = "Left Most (Parts 1-2)";
                                                     break;
                                                 case ImageProcessor::SplitMode::RIGHT_MOST:
                                                     modeStr = "Right Most (Parts 3-4)";
                                                     break;
                                                 default:
                                                     modeStr = "All Parts";
                                                 }

                                                 QString methodStr = mergeMethod == ImageProcessor::MergeMethod::MINIMUM_VALUE ?
                                                                         "Minimum Value" : "Weighted Average";

                                                 updateLastAction("Split & Merge",
                                                                  QString("Mode: %1, Method: %2").arg(modeStr).arg(methodStr));
                                             }
                                         }}
                                       });
}

void ControlPanel::setupFilteringOperations() {
    createGroupBox("Filtering Operations", {
                                               {"Median Filter", [this]() {
                                                 if (checkZoomMode()) return;
                                                 resetDetectedLines();
                                                    auto [filterKernelSize, ok] = showInputDialog("Median Filter", "Enter kernel size:", 3, 1, 21);
                                                    if (ok) {
                                                        m_imageProcessor.applyMedianFilter(
                                                            const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()),
                                                            filterKernelSize
                                                            );
                                                        m_imageLabel->clearSelection();
                                                        updateImageDisplay();
                                                        updateLastAction("Median Filter", QString("Size: %1").arg(filterKernelSize));
                                                    }
                                                }},
                                               {"High-Pass Filter", [this]() {
                                                 if (checkZoomMode()) return;
                                                 resetDetectedLines();
                                                    m_imageProcessor.applyHighPassFilter(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()));
                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("High-Pass Filter");
                                                }}
                                           });
}

void ControlPanel::setupAdvancedOperations()
{
    createGroupBox("Advanced Operations", {
                                           {"Stretch", [this]() {
                                                if (checkZoomMode()) return;
                                                resetDetectedLines();

                                                // Create dialog for stretch options
                                                QDialog dialog(this);
                                                dialog.setWindowTitle("Stretch Options");
                                                QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                                // Direction selection
                                                QGroupBox* directionBox = new QGroupBox("Stretch Direction");
                                                QVBoxLayout* directionLayout = new QVBoxLayout(directionBox);
                                                QRadioButton* verticalRadio = new QRadioButton("Vertical");
                                                QRadioButton* horizontalRadio = new QRadioButton("Horizontal");
                                                verticalRadio->setChecked(true);
                                                directionLayout->addWidget(verticalRadio);
                                                directionLayout->addWidget(horizontalRadio);
                                                layout->addWidget(directionBox);

                                                // Stretch factor input
                                                QLabel* factorLabel = new QLabel("Stretch Factor:");
                                                QDoubleSpinBox* factorSpinBox = new QDoubleSpinBox();
                                                factorSpinBox->setRange(0.1, 10.0);
                                                factorSpinBox->setValue(1.5);
                                                factorSpinBox->setSingleStep(0.1);
                                                layout->addWidget(factorLabel);
                                                layout->addWidget(factorSpinBox);

                                                // Add OK and Cancel buttons
                                                QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                    Qt::Horizontal, &dialog);
                                                layout->addWidget(buttonBox);

                                                connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                if (dialog.exec() == QDialog::Accepted) {
                                                    float stretchFactor = factorSpinBox->value();
                                                    bool isVertical = verticalRadio->isChecked();

                                                    if (isVertical) {
                                                        m_imageProcessor.stretchImageY(
                                                            const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()),
                                                            stretchFactor);
                                                    } else {
                                                        m_imageProcessor.stretchImageX(
                                                            const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()),
                                                            stretchFactor);
                                                    }

                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("Stretch",
                                                                     QString("%1 - Factor: %2")
                                                                         .arg(isVertical ? "Vertical" : "Horizontal")
                                                                         .arg(stretchFactor, 0, 'f', 2));
                                                }
                                            }},
                                              {"Padding", [this]() {
                                                if (checkZoomMode()) return;
                                                resetDetectedLines();
                                                   auto [paddingSize, ok] = showInputDialog("Padding Size", "Enter padding size:", 10, 1, 1000);
                                                   if (ok) {
                                                       auto paddedImage = m_imageProcessor.addPadding(m_imageProcessor.getFinalImage(), paddingSize);
                                                       m_imageProcessor.updateAndSaveFinalImage(paddedImage);
                                                       m_imageLabel->clearSelection();
                                                       updateImageDisplay();
                                                       updateLastAction("Padding", QString::number(paddingSize));
                                                   }
                                               }},
                                              {"Apply Distortion", [this]() {
                                                if (checkZoomMode()) return;
                                                resetDetectedLines();
                                                   QStringList directions = {"Left", "Right", "Top", "Bottom"};
                                                   bool ok;
                                                   QString selectedDirection = QInputDialog::getItem(this, "Distortion Direction", "Select direction:", directions, 0, false, &ok);
                                                   if (!ok) return;

                                                   auto [distortionFactor, factorOk] = showInputDialog("Distortion Factor", "Enter distortion factor:", 1.5, 1.0, 100.0);
                                                   if (factorOk) {
                                                       auto distortedImage = m_imageProcessor.distortImage(m_imageProcessor.getFinalImage(), distortionFactor, selectedDirection.toStdString());
                                                       m_imageProcessor.updateAndSaveFinalImage(distortedImage);
                                                       m_imageLabel->clearSelection();
                                                       updateImageDisplay();
                                                       updateLastAction("Distortion", QString("%1 - %2").arg(selectedDirection).arg(distortionFactor, 0, 'f', 2));
                                                   }
                                               }}
                                          });
}

void ControlPanel::setupCLAHEOperations() {
    createGroupBox("CLAHE Operations", {
                                           {"CLAHE (GPU)", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
                                                auto [clipLimit, clipOk] = showInputDialog("CLAHE", "Enter clip limit:", 2.0, 0.1, 1000);
                                                if (!clipOk) return;

                                                auto [tileSize, tileOk] = showInputDialog("CLAHE", "Enter tile size:", 8, 2, 1000);
                                                if (!tileOk) return;

                                                try {
                                                    cv::Mat matImage = m_imageProcessor.vectorToMat(m_imageProcessor.getFinalImage());
                                                    cv::Mat resultImage = m_imageProcessor.applyCLAHE(matImage, clipLimit, cv::Size(tileSize, tileSize));

                                                    m_lastGpuTime = m_imageProcessor.getLastPerformanceMetrics().processingTime;
                                                    m_hasGpuClaheTime = true;
                                                    m_gpuTimingLabel->setText(QString("CLAHE Processing Time (GPU): %1 ms").arg(m_lastGpuTime, 0, 'f', 2));
                                                    m_gpuTimingLabel->setVisible(true);

                                                    m_imageProcessor.updateAndSaveFinalImage(m_imageProcessor.matToVector(resultImage));
                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("CLAHE (GPU)", QString("Clip: %1, Tile: %2").arg(clipLimit, 0, 'f', 2).arg(tileSize));
                                                }
                                                catch (const cv::Exception& e) {
                                                    QMessageBox::critical(this, "Error", QString("GPU CLAHE processing failed: %1").arg(e.what()));
                                                }
                                            }},

                                           {"CLAHE (CPU)", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
                                                auto [clipLimit, clipOk] = showInputDialog("CLAHE", "Enter clip limit:", 2.0, 0.1, 1000);
                                                if (!clipOk) return;

                                                auto [tileSize, tileOk] = showInputDialog("CLAHE", "Enter tile size:", 8, 2, 1000);
                                                if (!tileOk) return;

                                                try {
                                                    cv::Mat matImage = m_imageProcessor.vectorToMat(m_imageProcessor.getFinalImage());
                                                    cv::Mat resultImage = m_imageProcessor.applyCLAHE_CPU(matImage, clipLimit, cv::Size(tileSize, tileSize));

                                                    m_lastCpuTime = m_imageProcessor.getLastPerformanceMetrics().processingTime;
                                                    m_hasCpuClaheTime = true;
                                                    m_cpuTimingLabel->setText(QString("CLAHE Processing Time (CPU): %1 ms").arg(m_lastCpuTime, 0, 'f', 2));
                                                    m_cpuTimingLabel->setVisible(true);

                                                    m_imageProcessor.updateAndSaveFinalImage(m_imageProcessor.matToVector(resultImage));
                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("CLAHE (CPU)", QString("Clip: %1, Tile: %2").arg(clipLimit, 0, 'f', 2).arg(tileSize));
                                                }
                                                catch (const cv::Exception& e) {
                                                    QMessageBox::critical(this, "Error", QString("CPU CLAHE processing failed: %1").arg(e.what()));
                                                }
                                            }},

                                           {"Threshold CLAHE (GPU)", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
                                                auto [threshold, thresholdOk] = showInputDialog("Threshold", "Enter threshold value:", 5000, 0, 65535);
                                                if (!thresholdOk) return;

                                                auto [clipLimit, clipOk] = showInputDialog("CLAHE", "Enter clip limit:", 2.0, 0.1, 1000);
                                                if (!clipOk) return;

                                                auto [tileSize, tileOk] = showInputDialog("CLAHE", "Enter tile size:", 8, 2, 1000);
                                                if (!tileOk) return;

                                                try {
                                                    m_imageProcessor.applyThresholdCLAHE_GPU(threshold, clipLimit, cv::Size(tileSize, tileSize));

                                                    m_lastGpuTime = m_imageProcessor.getLastPerformanceMetrics().processingTime;
                                                    m_hasGpuClaheTime = true;
                                                    m_gpuTimingLabel->setText(QString("CLAHE Processing Time (GPU): %1 ms").arg(m_lastGpuTime, 0, 'f', 2));
                                                    m_gpuTimingLabel->setVisible(true);

                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("Threshold CLAHE (GPU)",
                                                                     QString("Threshold: %1, Clip: %2, Tile: %3")
                                                                         .arg(threshold)
                                                                         .arg(clipLimit, 0, 'f', 2)
                                                                         .arg(tileSize));
                                                }
                                                catch (const cv::Exception& e) {
                                                    QMessageBox::critical(this, "Error", QString("GPU Threshold CLAHE processing failed: %1").arg(e.what()));
                                                }
                                            }},

                                           {"Threshold CLAHE (CPU)", [this]() {
                                             if (checkZoomMode()) return;
                                             resetDetectedLines();
                                                auto [threshold, thresholdOk] = showInputDialog("Threshold", "Enter threshold value:", 5000, 0, 65535);
                                                if (!thresholdOk) return;

                                                auto [clipLimit, clipOk] = showInputDialog("CLAHE", "Enter clip limit:", 2.0, 0.1, 100.0);
                                                if (!clipOk) return;

                                                auto [tileSize, tileOk] = showInputDialog("CLAHE", "Enter tile size:", 8, 2, 100);
                                                if (!tileOk) return;

                                                try {
                                                    m_imageProcessor.applyThresholdCLAHE_CPU(threshold, clipLimit, cv::Size(tileSize, tileSize));

                                                    m_lastCpuTime = m_imageProcessor.getLastPerformanceMetrics().processingTime;
                                                    m_hasCpuClaheTime = true;
                                                    m_cpuTimingLabel->setText(QString("CLAHE Processing Time (CPU): %1 ms").arg(m_lastCpuTime, 0, 'f', 2));
                                                    m_cpuTimingLabel->setVisible(true);

                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("Threshold CLAHE (CPU)",
                                                                     QString("Threshold: %1, Clip: %2, Tile: %3")
                                                                         .arg(threshold)
                                                                         .arg(clipLimit, 0, 'f', 2)
                                                                         .arg(tileSize));
                                                }
                                                catch (const cv::Exception& e) {
                                                    QMessageBox::critical(this, "Error", QString("CPU Threshold CLAHE processing failed: %1").arg(e.what()));
                                                }
                                            }}
                                       });
}

void ControlPanel::setupGlobalAdjustments()
{
    createGroupBox("Global Adjustments", {
                                             {"Overall Gamma", [this]() {
                                               if (checkZoomMode()) return;
                                                  resetDetectedLines();
                                                  auto [gammaValue, ok] = showInputDialog("Overall Gamma", "Enter gamma value:", 1.0, 0.1, 10.0);
                                                  if (ok) {
                                                      m_imageProcessor.adjustGammaOverall(gammaValue);  // Use the new wrapper method
                                                      m_imageLabel->clearSelection();
                                                      updateImageDisplay();
                                                      updateLastAction("Overall Gamma", QString::number(gammaValue, 'f', 2));
                                                  }
                                              }},
                                             {"Overall Sharpen", [this]() {
                                               if (checkZoomMode()) return;
                                                  resetDetectedLines();
                                                  auto [sharpenStrength, ok] = showInputDialog("Overall Sharpen", "Enter sharpen strength:", 1.0, 0.1, 10.0);
                                                  if (ok) {
                                                      m_imageProcessor.sharpenImage(sharpenStrength);  // Use the new wrapper method
                                                      m_imageLabel->clearSelection();
                                                      updateImageDisplay();
                                                      updateLastAction("Overall Sharpen", QString::number(sharpenStrength, 'f', 2));
                                                  }
                                              }},
                                             {"Overall Contrast", [this]() {
                                                  resetDetectedLines();
                                                  auto [contrastFactor, ok] = showInputDialog("Overall Contrast", "Enter contrast factor:", 1.0, 0.1, 10.0);
                                                  if (ok) {
                                                      m_imageProcessor.adjustContrast(contrastFactor);  // Use the new wrapper method
                                                      m_imageLabel->clearSelection();
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
                                                 if (checkZoomMode()) return;
                                                    resetDetectedLines();
                                                    if (m_imageLabel->isRegionSelected()) {
                                                        auto [gamma, ok] = showInputDialog("Region Gamma", "Enter gamma value:", 1.0, 0.1, 10.0);
                                                        if (ok) {
                                                            QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                            m_imageProcessor.adjustGammaForSelectedRegion(gamma, selectedRegion);  // Use the new wrapper method
                                                            m_imageLabel->clearSelection();
                                                            updateImageDisplay();
                                                            updateLastAction("Region Gamma", QString::number(gamma, 'f', 2));
                                                        }
                                                    } else {
                                                        QMessageBox::information(this, "Region Gamma", "Please select a region first.");
                                                    }
                                                }},
                                               {"Region Sharpen", [this]() {
                                                 if (checkZoomMode()) return;
                                                    resetDetectedLines();
                                                    if (m_imageLabel->isRegionSelected()) {
                                                        auto [sharpenStrength, ok] = showInputDialog("Region Sharpen", "Enter sharpen strength:", 0.5, 0.1, 5.0);
                                                        if (ok) {
                                                            QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                            m_imageProcessor.applySharpenToRegion(sharpenStrength, selectedRegion);  // Use the new wrapper method
                                                            m_imageLabel->clearSelection();
                                                            updateImageDisplay();
                                                            updateLastAction("Region Sharpen", QString::number(sharpenStrength, 'f', 2));
                                                        }
                                                    } else {
                                                        QMessageBox::information(this, "Region Sharpen", "Please select a region first.");
                                                    }
                                                }},
                                               {"Region Contrast", [this]() {
                                                 if (checkZoomMode()) return;
                                                    resetDetectedLines();
                                                    if (m_imageLabel->isRegionSelected()) {
                                                        auto [contrastFactor, ok] = showInputDialog("Region Contrast", "Enter contrast factor:", 1.5, 0.1, 5.0);
                                                        if (ok) {
                                                            QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                            m_imageProcessor.applyContrastToRegion(contrastFactor, selectedRegion);  // Use the new wrapper method
                                                            m_imageLabel->clearSelection();
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

void ControlPanel::setupBlackLineDetection() {
    createGroupBox("Dark Line Detection", {  // Renamed from "Black Line Detection"
                                           {"Detect Black Lines", [this]() {
                                                m_imageProcessor.saveCurrentState();
                                                m_detectedLines = m_imageProcessor.detectDarkLines();
                                                updateImageDisplay();

                                                // Format coordinates and weights info
                                                QStringList lineInfo;
                                                for (const auto& line : m_detectedLines) {
                                                    if (line.isVertical) {
                                                        lineInfo << QString("(%1,0) - Line Weight: %2").arg(line.x).arg(line.width);
                                                    } else {
                                                        lineInfo << QString("(0,%1) - Line Weight: %2").arg(line.y).arg(line.width);
                                                    }
                                                }

                                                // Update labels
                                                updateLastAction("Detect Black Lines", lineInfo.join(", "));
                                            }},

                                           {"Remove Black Lines", [this]() {
                                                if (m_detectedLines.empty()) {
                                                    QMessageBox::information(this, "Remove Black Lines",
                                                                             "Please detect black lines first.");
                                                    return;
                                                }

                                                // Store line info before removal
                                                QStringList removedLines;
                                                for (const auto& line : m_detectedLines) {
                                                    if (line.isVertical) {
                                                        removedLines << QString("(%1,0) - Line Weight: %2").arg(line.x).arg(line.width);
                                                    } else {
                                                        removedLines << QString("(0,%1) - Line Weight: %2").arg(line.y).arg(line.width);
                                                    }
                                                }

                                                m_imageProcessor.removeDarkLines(m_detectedLines);
                                                m_detectedLines.clear();
                                                updateImageDisplay();

                                                // Update labels with removed line information
                                                updateLastAction("Remove Black Lines", removedLines.join(", "));
                                            }}
                                          });
}

void ControlPanel::updateImageDisplay() {
    const auto& finalImage = m_imageProcessor.getFinalImage();
    if (!finalImage.empty()) {
        int height = finalImage.size();
        int width = finalImage[0].size();
        QImage image(width, height, QImage::Format_Grayscale16);

        // Copy image data
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint16_t pixelValue = finalImage[y][x];
                image.setPixel(x, y, qRgb(pixelValue >> 8, pixelValue >> 8, pixelValue >> 8));
            }
        }

        // Create base pixmap from image
        QPixmap pixmap = QPixmap::fromImage(image);

        // Apply zoom if in zoom mode
        if (m_zoomModeActive && m_imageProcessor.getZoomLevel() != 1.0f) {
            QSize zoomedSize = m_imageProcessor.getZoomedImageDimensions();
            pixmap = pixmap.scaled(zoomedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // Start painting on the pixmap
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        // Draw detected black lines
        const auto& detectedLines = m_imageProcessor.getDetectedLines();
        if (!detectedLines.empty()) {
            // First draw the lines
            for (const auto& line : detectedLines) {
                QColor lineColor = line.inObject ? Qt::blue : Qt::red;
                painter.setPen(QPen(lineColor, 2, Qt::SolidLine));
                if (line.isVertical) {
                    QRect lineRect(line.x, 0, line.width, height - 1);
                    painter.drawRect(lineRect);
                } else {
                    QRect lineRect(0, line.y, width - 1, line.width);
                    painter.drawRect(lineRect);
                }
            }

            // Then draw the status indicators
            int statusHeight = 20;
            int statusWidth = 80;
            int statusMargin = 5;
            int currentY = 10;

            painter.setFont(QFont("Arial", 8, QFont::Bold));

            for (const auto& line : detectedLines) {
                QRect statusRect(statusMargin, currentY, statusWidth, statusHeight);
                QColor bgColor = line.inObject ? QColor(0, 0, 255) : QColor(255, 0, 0);

                // Draw status box background
                painter.fillRect(statusRect, bgColor);

                // Draw status text
                painter.setPen(Qt::white);
                QString statusText = line.inObject ? "In Object" : "Isolated";
                painter.drawText(statusRect, Qt::AlignCenter, statusText);

                currentY += statusHeight + statusMargin;
            }
        }

        // Draw selection rectangle if exists
        if (m_imageLabel->isRegionSelected()) {
            painter.setPen(QPen(Qt::blue, 2));
            QRect selectedRegion = m_imageLabel->getSelectedRegion();
            if (m_zoomModeActive) {
                // Scale the selection rectangle according to zoom level
                float zoomLevel = m_imageProcessor.getZoomLevel();
                selectedRegion = QRect(
                    selectedRegion.x() * zoomLevel,
                    selectedRegion.y() * zoomLevel,
                    selectedRegion.width() * zoomLevel,
                    selectedRegion.height() * zoomLevel
                    );
            }
            painter.drawRect(selectedRegion);
        }

        painter.end();

        // Update the image label
        m_imageLabel->setPixmap(pixmap);
        m_imageLabel->setFixedSize(pixmap.size());

        // Update histogram if visible
        if (m_histogram->isVisible()) {
            m_histogram->updateHistogram(finalImage);
        }
    }
}

void ControlPanel::resetDetectedLines() {
    m_detectedLines.clear();
    m_imageProcessor.clearDetectedLines();
    updateImageDisplay();
}
