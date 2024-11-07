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

    m_darkLineInfoLabel = new QLabel("");
    m_darkLineInfoLabel->setMinimumHeight(30);  // Minimum height for single line
    m_darkLineInfoLabel->setStyleSheet(
        "QLabel {"
        "    color: black;"
        "    font-family: monospace;"
        "    font-size: 11px;"
        "    background-color: #f8f8f8;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    margin: 4px 0px;"
        "}"
        );
    m_darkLineInfoLabel->setTextFormat(Qt::PlainText);
    m_darkLineInfoLabel->setWordWrap(true);
    m_darkLineInfoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_darkLineInfoLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    m_darkLineInfoLabel->setVisible(false);

    // Create and setup the scroll area for the dark line info
    QScrollArea* darkLineScrollArea = new QScrollArea;
    darkLineScrollArea->setWidget(m_darkLineInfoLabel);
    darkLineScrollArea->setWidgetResizable(true);
    darkLineScrollArea->setMaximumHeight(300);  // Maximum height for scroll area
    darkLineScrollArea->setMinimumHeight(30);   // Minimum height for scroll area
    darkLineScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    darkLineScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    darkLineScrollArea->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    darkLineScrollArea->setStyleSheet(
        "QScrollArea {"
        "    border: none;"
        "    background: transparent;"
        "}"
        "QScrollBar:vertical {"
        "    width: 10px;"
        "    background: transparent;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: #CCCCCC;"
        "    border-radius: 5px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0px;"
        "}"
        );
    darkLineScrollArea->setVisible(false);

    // Add the scroll area to the layout
    QVBoxLayout* infoLayout = qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout());
    infoLayout->insertWidget(3, darkLineScrollArea);
    infoLayout->insertWidget(3, darkLineScrollArea);

    // Setup other components
    setupFileOperations();
    setupPreProcessingOperations();
    setupBasicOperations();
    setupZoomControls();
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

    // Setup zoom warning box
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

    // Add image size label
    m_imageSizeLabel = new QLabel("Image Size: No image loaded");
    m_imageSizeLabel->setFixedHeight(30);
    infoLayout->addWidget(m_imageSizeLabel);

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

    // Dark line info label setup remains the same...
    m_darkLineInfoLabel = new QLabel("");
    m_darkLineInfoLabel->setMinimumHeight(15);
    m_darkLineInfoLabel->setMaximumHeight(30);
    m_darkLineInfoLabel->setStyleSheet(
        "QLabel {"
        "    color: black;"
        "    font-family: Arial;"
        "    font-size: 11px;"
        "    background-color: #f8f8f8;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 8px;"
        "    margin: 4px 0px;"
        "}"
        );
    m_darkLineInfoLabel->setTextFormat(Qt::PlainText);
    m_darkLineInfoLabel->setWordWrap(true);
    m_darkLineInfoLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_darkLineInfoLabel->setVisible(false);
    infoLayout->addWidget(m_darkLineInfoLabel);

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

    m_histogram = new Histogram(this);
    m_histogram->setMinimumWidth(250);
    m_histogram->setVisible(false);
    infoLayout->addWidget(m_histogram);

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
                                                       m_darkLineInfoLabel->hide();
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
                                                   resetDetectedLines();
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
                                                m_darkLineInfoLabel->hide();
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
                                                m_darkLineInfoLabel->hide();
                                                resetDetectedLines();
                                                auto rotatedImage = m_imageProcessor.rotateImage(m_imageProcessor.getFinalImage(), 90);
                                                m_imageProcessor.updateAndSaveFinalImage(rotatedImage);
                                                m_imageLabel->clearSelection();
                                                updateImageDisplay();
                                                updateLastAction("Rotate Clockwise");
                                            }},
                                           {"Rotate CCW", [this]() {
                                                if (checkZoomMode()) return;
                                                m_darkLineInfoLabel->hide();
                                                resetDetectedLines();
                                                auto rotatedImage = m_imageProcessor.rotateImage(m_imageProcessor.getFinalImage(), 270);
                                                m_imageProcessor.updateAndSaveFinalImage(rotatedImage);
                                                m_imageLabel->clearSelection();
                                                updateImageDisplay();
                                                updateLastAction("Rotate CCW");
                                            }}
                                       });
}

void ControlPanel::setupPreProcessingOperations()
{
    createGroupBox("Pre-Processing Operations", {
                                                    {"Calibration", [this]() {
                                                         if (checkZoomMode()) return;
                                                         m_darkLineInfoLabel->hide();
                                                         resetDetectedLines();
                                                         auto [linesToProcessY, yOk] = showInputDialog("Calibration", "Enter lines to process for Y-axis:", 10, 1, 1000);
                                                         if (!yOk) return;

                                                         auto [linesToProcessX, xOk] = showInputDialog("Calibration", "Enter lines to process for X-axis:", 10, 1, 1000);
                                                         if (xOk) {
                                                             m_imageProcessor.processYXAxis(const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()),
                                                                                            linesToProcessY, linesToProcessX);
                                                             m_imageLabel->clearSelection();
                                                             updateImageDisplay();
                                                             updateLastAction("Calibration", QString("Y: %1, X: %2").arg(linesToProcessY).arg(linesToProcessX));
                                                         }
                                                     }},
                                                 {"Interlace", [this]() {
                                                      if (checkZoomMode()) return;
                                                      m_darkLineInfoLabel->hide();
                                                      resetDetectedLines();

                                                      // Create dialog for options
                                                      QDialog dialog(this);
                                                      dialog.setWindowTitle("Interlace Processing Options");
                                                      dialog.setMinimumWidth(400);
                                                      QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                                      // Low Energy Section
                                                      QGroupBox* lowEnergyBox = new QGroupBox("Low Energy Section Start");
                                                      QVBoxLayout* lowEnergyLayout = new QVBoxLayout(lowEnergyBox);
                                                      QRadioButton* leftLeftLowRadio = new QRadioButton("Start with Left Left");
                                                      QRadioButton* leftRightLowRadio = new QRadioButton("Start with Left Right");
                                                      leftLeftLowRadio->setChecked(true);

                                                      // Add explanatory label for Low Energy
                                                      QLabel* lowEnergyExplanation = new QLabel("Low Energy interlace pattern will be:\nRow 1: Selected start section\nRow 2: Other section\nRepeating for each original row");
                                                      lowEnergyExplanation->setStyleSheet("color: #666; font-size: 10px;");

                                                      lowEnergyLayout->addWidget(leftLeftLowRadio);
                                                      lowEnergyLayout->addWidget(leftRightLowRadio);
                                                      lowEnergyLayout->addWidget(lowEnergyExplanation);
                                                      lowEnergyBox->setLayout(lowEnergyLayout);
                                                      layout->addWidget(lowEnergyBox);

                                                      // High Energy Section
                                                      QGroupBox* highEnergyBox = new QGroupBox("High Energy Section Start");
                                                      QVBoxLayout* highEnergyLayout = new QVBoxLayout(highEnergyBox);
                                                      QRadioButton* rightLeftHighRadio = new QRadioButton("Start with Right Left");
                                                      QRadioButton* rightRightHighRadio = new QRadioButton("Start with Right Right");
                                                      rightLeftHighRadio->setChecked(true);

                                                      // Add explanatory label for High Energy
                                                      QLabel* highEnergyExplanation = new QLabel("High Energy interlace pattern will be:\nRow 1: Selected start section\nRow 2: Other section\nRepeating for each original row");
                                                      highEnergyExplanation->setStyleSheet("color: #666; font-size: 10px;");

                                                      highEnergyLayout->addWidget(rightLeftHighRadio);
                                                      highEnergyLayout->addWidget(rightRightHighRadio);
                                                      highEnergyLayout->addWidget(highEnergyExplanation);
                                                      highEnergyBox->setLayout(highEnergyLayout);
                                                      layout->addWidget(highEnergyBox);

                                                      // Add buttons
                                                      QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                          QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
                                                          Qt::Horizontal, &dialog);
                                                      layout->addWidget(buttonBox);

                                                      connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                      connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                      if (dialog.exec() == QDialog::Accepted) {
                                                          ImageProcessor::InterlaceStartPoint lowEnergyStart =
                                                              leftLeftLowRadio->isChecked() ?
                                                                  ImageProcessor::InterlaceStartPoint::LEFT_LEFT :
                                                                  ImageProcessor::InterlaceStartPoint::LEFT_RIGHT;

                                                          ImageProcessor::InterlaceStartPoint highEnergyStart =
                                                              rightLeftHighRadio->isChecked() ?
                                                                  ImageProcessor::InterlaceStartPoint::RIGHT_LEFT :
                                                                  ImageProcessor::InterlaceStartPoint::RIGHT_RIGHT;

                                                          m_imageProcessor.processInterlacedEnergySectionsWithDisplay(
                                                              lowEnergyStart,
                                                              highEnergyStart
                                                              );

                                                          m_imageLabel->clearSelection();
                                                          updateImageDisplay();

                                                          QString lowEnergyStr = leftLeftLowRadio->isChecked() ? "LeftLeft" : "LeftRight";
                                                          QString highEnergyStr = rightLeftHighRadio->isChecked() ? "RightLeft" : "RightRight";

                                                          updateLastAction("Interlace",
                                                                           QString("Low: %1, High: %2")
                                                                               .arg(lowEnergyStr)
                                                                               .arg(highEnergyStr));
                                                      }
                                                  }},
                                                    {"Split & Merge", [this]() {
                                                         if (checkZoomMode()) return;
                                                         m_darkLineInfoLabel->hide();
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
                                                 m_darkLineInfoLabel->hide();
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
                                                 m_darkLineInfoLabel->hide();
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
                                                m_darkLineInfoLabel->hide();
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
                                                m_darkLineInfoLabel->hide();
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
                                                m_darkLineInfoLabel->hide();
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
                                             m_darkLineInfoLabel->hide();
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
                                             m_darkLineInfoLabel->hide();
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
                                             m_darkLineInfoLabel->hide();
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
                                             m_darkLineInfoLabel->hide();
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
                                                  m_darkLineInfoLabel->hide();
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
                                               m_darkLineInfoLabel->hide();
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
                                               m_darkLineInfoLabel->hide();
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
                                                 m_darkLineInfoLabel->hide();
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
                                                 m_darkLineInfoLabel->hide();
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
                                                 m_darkLineInfoLabel->hide();
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
    createGroupBox("Dark Line Detection", {
                                              {"Detect Lines", [this]() {
                                                   if (checkZoomMode()) return;

                                                   // Clear previous detections first
                                                   resetDetectedLines();
                                                   m_imageProcessor.saveCurrentState();
                                                   m_detectedLines = m_imageProcessor.detectDarkLines();

                                                   // Create detection info summary
                                                   QString detectionInfo = "Detected Lines:\n\n";

                                                   // Count lines by type
                                                   int inObjectCount = 0;
                                                   int isolatedCount = 0;

                                                   // List all detected lines with details
                                                   for (size_t i = 0; i < m_detectedLines.size(); ++i) {
                                                       const auto& line = m_detectedLines[i];

                                                       QString coordinates;
                                                       if (line.isVertical) {
                                                           coordinates = QString("(%1,0)").arg(line.x);
                                                       } else {
                                                           coordinates = QString("(0,%1)").arg(line.y);
                                                       }

                                                       detectionInfo += QString("Line %1: %2 with width %3 pixels (%4)\n")
                                                                            .arg(i + 1)
                                                                            .arg(coordinates)
                                                                            .arg(line.width)
                                                                            .arg(line.inObject ? "In Object" : "Isolated");

                                                       if (line.inObject) {
                                                           inObjectCount++;
                                                       } else {
                                                           isolatedCount++;
                                                       }
                                                   }

                                                   // Add summary statistics
                                                   detectionInfo += QString("\nSummary:\n");
                                                   detectionInfo += QString("Total Lines: %1\n").arg(m_detectedLines.size());
                                                   detectionInfo += QString("In-Object Lines: %1\n").arg(inObjectCount);
                                                   detectionInfo += QString("Isolated Lines: %1\n").arg(isolatedCount);

                                                   // Update the info label and adjust its height
                                                   m_darkLineInfoLabel->setText(detectionInfo);

                                                   // Adjust scroll area height based on content
                                                   QFontMetrics fm(m_darkLineInfoLabel->font());
                                                   int textHeight = fm.lineSpacing() * detectionInfo.count('\n') + 40;
                                                   int preferredHeight = qMin(textHeight, 300);
                                                   preferredHeight = qMax(preferredHeight, 30);

                                                   QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
                                                       qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
                                                       );
                                                   darkLineScrollArea->setFixedHeight(preferredHeight);

                                                   // Show the info label and scroll area
                                                   m_darkLineInfoLabel->setVisible(true);
                                                   darkLineScrollArea->setVisible(true);

                                                   updateImageDisplay();
                                                   updateLastAction("Detect Lines");
                                               }},

                                              {"Remove Lines", [this]() {
                                                   if (checkZoomMode()) return;

                                                   if (m_detectedLines.empty()) {
                                                       QMessageBox::information(this, "Remove Lines", "Please detect lines first.");
                                                       return;
                                                   }

                                                   // Store initial line information for comparison
                                                   std::vector<ImageProcessor::DarkLine> initialLines = m_detectedLines;

                                                   // Create main dialog
                                                   QDialog dialog(this);
                                                   dialog.setWindowTitle("Remove Lines");
                                                   dialog.setMinimumWidth(400);

                                                   QVBoxLayout* layout = new QVBoxLayout(&dialog);
                                                   layout->setSpacing(10);

                                                   // Count lines
                                                   int inObjectCount = 0;
                                                   int isolatedCount = 0;
                                                   for (const auto& line : m_detectedLines) {
                                                       if (line.inObject) inObjectCount++;
                                                       else isolatedCount++;
                                                   }

                                                   // Line type selection
                                                   QGroupBox* removalTypeBox = new QGroupBox("Select Lines to Remove");
                                                   QVBoxLayout* typeLayout = new QVBoxLayout();

                                                   QRadioButton* inObjectRadio = new QRadioButton(
                                                       QString("Remove In Object Lines (%1 lines)").arg(inObjectCount));
                                                   QRadioButton* isolatedRadio = new QRadioButton(
                                                       QString("Remove Isolated Lines (%1 lines)").arg(isolatedCount));
                                                   inObjectRadio->setChecked(true);

                                                   typeLayout->addWidget(inObjectRadio);
                                                   typeLayout->addWidget(isolatedRadio);
                                                   removalTypeBox->setLayout(typeLayout);
                                                   layout->addWidget(removalTypeBox);

                                                   // Method selection (for In Object Lines)
                                                   QGroupBox* methodBox = new QGroupBox("Removal Method");
                                                   QVBoxLayout* methodLayout = new QVBoxLayout();
                                                   QRadioButton* neighborValuesRadio = new QRadioButton("Use Neighbor Values");
                                                   QRadioButton* stitchRadio = new QRadioButton("Direct Stitch");
                                                   neighborValuesRadio->setChecked(true);
                                                   methodLayout->addWidget(neighborValuesRadio);
                                                   methodLayout->addWidget(stitchRadio);
                                                   methodBox->setLayout(methodLayout);
                                                   layout->addWidget(methodBox);

                                                   // Line selection list (only for In Object + Direct Stitch)
                                                   QGroupBox* lineSelectionBox = new QGroupBox("Select Line to Process");
                                                   QVBoxLayout* selectionLayout = new QVBoxLayout();
                                                   QListWidget* lineList = new QListWidget();
                                                   lineList->setSelectionMode(QAbstractItemView::SingleSelection);

                                                   // Add only In Object lines to the list
                                                   for (size_t i = 0; i < m_detectedLines.size(); ++i) {
                                                       const auto& line = m_detectedLines[i];
                                                       if (line.inObject) {
                                                           QString lineInfo = QString("Line %1: %2 at %3 with width %4")
                                                           .arg(i + 1)
                                                               .arg(line.isVertical ? "Vertical" : "Horizontal")
                                                               .arg(line.isVertical ? QString("x=%1").arg(line.x) : QString("y=%1").arg(line.y))
                                                               .arg(line.width);
                                                           QListWidgetItem* item = new QListWidgetItem(lineInfo);
                                                           item->setData(Qt::UserRole, static_cast<int>(i));
                                                           lineList->addItem(item);
                                                       }
                                                   }

                                                   lineList->setMinimumHeight(100);
                                                   lineList->setMaximumHeight(200);
                                                   selectionLayout->addWidget(lineList);
                                                   lineSelectionBox->setLayout(selectionLayout);
                                                   layout->addWidget(lineSelectionBox);

                                                   // Control visibility logic
                                                   auto updateVisibility = [&]() {
                                                       bool isInObject = inObjectRadio->isChecked();
                                                       methodBox->setVisible(isInObject);
                                                       lineSelectionBox->setVisible(isInObject && stitchRadio->isChecked());
                                                       dialog.adjustSize();
                                                   };

                                                   connect(inObjectRadio, &QRadioButton::toggled, updateVisibility);
                                                   connect(isolatedRadio, &QRadioButton::toggled, updateVisibility);
                                                   connect(stitchRadio, &QRadioButton::toggled, updateVisibility);
                                                   connect(neighborValuesRadio, &QRadioButton::toggled, updateVisibility);

                                                   // Add OK and Cancel buttons
                                                   QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                       QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                                   layout->addWidget(buttonBox);

                                                   connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                   connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                   dialog.setLayout(layout);
                                                   updateVisibility();

                                                   if (dialog.exec() == QDialog::Accepted) {
                                                       bool removeInObject = inObjectRadio->isChecked();
                                                       QString methodStr;
                                                       QString typeStr = removeInObject ? "In-Object Lines" : "Isolated Lines";

                                                       std::vector<ImageProcessor::DarkLine> selectedLines;

                                                       if (removeInObject) {
                                                           ImageProcessor::LineRemovalMethod method =
                                                               stitchRadio->isChecked() ?
                                                                   ImageProcessor::LineRemovalMethod::DIRECT_STITCH :
                                                                   ImageProcessor::LineRemovalMethod::NEIGHBOR_VALUES;

                                                           methodStr = stitchRadio->isChecked() ? "Direct Stitch" : "Neighbor Values";

                                                           if (method == ImageProcessor::LineRemovalMethod::DIRECT_STITCH) {
                                                               QListWidgetItem* selectedItem = lineList->currentItem();
                                                               if (!selectedItem) {
                                                                   QMessageBox::warning(this, "Warning", "Please select a line for processing.");
                                                                   return;
                                                               }

                                                               int index = selectedItem->data(Qt::UserRole).toInt();
                                                               selectedLines.push_back(m_detectedLines[index]);

                                                               m_imageProcessor.removeDarkLinesSequential(
                                                                   selectedLines,
                                                                   true,   // removeInObject
                                                                   false,  // removeIsolated
                                                                   method
                                                                   );
                                                           } else {
                                                               m_imageProcessor.removeDarkLinesSelective(
                                                                   true,   // removeInObject
                                                                   false,  // removeIsolated
                                                                   method
                                                                   );
                                                           }
                                                       } else {
                                                           methodStr = "Neighbor Values";
                                                           m_imageProcessor.removeDarkLinesSelective(
                                                               false,  // removeInObject
                                                               true,   // removeIsolated
                                                               ImageProcessor::LineRemovalMethod::NEIGHBOR_VALUES
                                                               );
                                                       }

                                                       // Get updated lines after removal
                                                       std::vector<ImageProcessor::DarkLine> remainingLines = m_imageProcessor.detectDarkLines();

                                                       // Create removal info summary
                                                       QString removalInfo = "Line Removal Summary:\n\n";
                                                       removalInfo += QString("Method Used: %1\n").arg(methodStr);
                                                       removalInfo += QString("Type: %1\n\n").arg(typeStr);

                                                       removalInfo += "Initial Lines:\n";
                                                       int initialInObjectCount = 0;
                                                       int initialIsolatedCount = 0;

                                                       for (const auto& line : initialLines) {
                                                           if (line.inObject) initialInObjectCount++;
                                                           else initialIsolatedCount++;
                                                       }

                                                       removalInfo += QString("Total Lines: %1\n").arg(initialLines.size());
                                                       removalInfo += QString("In-Object Lines: %1\n").arg(initialInObjectCount);
                                                       removalInfo += QString("Isolated Lines: %1\n\n").arg(initialIsolatedCount);

                                                       // Add removed lines info
                                                       removalInfo += "Removed Lines:\n";
                                                       if (stitchRadio->isChecked() && removeInObject) {
                                                           // For direct stitch, only show specifically selected lines
                                                           for (const auto& line : selectedLines) {
                                                               QString coordinates;
                                                               if (line.isVertical) {
                                                                   coordinates = QString("(%1,0)").arg(line.x);
                                                               } else {
                                                                   coordinates = QString("(0,%1)").arg(line.y);
                                                               }

                                                               removalInfo += QString("Line at %1 with width %2 pixels (%3)\n")
                                                                                  .arg(coordinates)
                                                                                  .arg(line.width)
                                                                                  .arg(line.inObject ? "In Object" : "Isolated");
                                                           }
                                                       } else {
                                                           // For neighbor values method or isolated lines, show all removed lines
                                                           for (const auto& line : initialLines) {
                                                               bool wasRemoved = true;
                                                               for (const auto& remainingLine : remainingLines) {
                                                                   if ((line.isVertical == remainingLine.isVertical) &&
                                                                       (line.isVertical ? (line.x == remainingLine.x) : (line.y == remainingLine.y)) &&
                                                                       (line.width == remainingLine.width)) {
                                                                       wasRemoved = false;
                                                                       break;
                                                                   }
                                                               }

                                                               if (wasRemoved) {
                                                                   QString coordinates;
                                                                   if (line.isVertical) {
                                                                       coordinates = QString("(%1,0)").arg(line.x);
                                                                   } else {
                                                                       coordinates = QString("(0,%1)").arg(line.y);
                                                                   }

                                                                   removalInfo += QString("Line at %1 with width %2 pixels (%3)\n")
                                                                                      .arg(coordinates)
                                                                                      .arg(line.width)
                                                                                      .arg(line.inObject ? "In Object" : "Isolated");
                                                               }
                                                           }
                                                       }

                                                       // Add remaining lines summary with detailed info
                                                       int remainingInObjectCount = 0;
                                                       int remainingIsolatedCount = 0;

                                                       removalInfo += QString("\nRemaining Lines:\n");

                                                       // Show detailed information for each remaining line
                                                       for (size_t i = 0; i < remainingLines.size(); ++i) {
                                                           const auto& line = remainingLines[i];
                                                           if (line.inObject) remainingInObjectCount++;
                                                           else remainingIsolatedCount++;

                                                           QString coordinates;
                                                           if (line.isVertical) {
                                                               coordinates = QString("(%1,0)").arg(line.x);
                                                           } else {
                                                               coordinates = QString("(0,%1)").arg(line.y);
                                                           }

                                                           removalInfo += QString("Line %1: %2 with width %3 pixels (%4)\n")
                                                                              .arg(i + 1)
                                                                              .arg(coordinates)
                                                                              .arg(line.width)
                                                                              .arg(line.inObject ? "In Object" : "Isolated");
                                                       }

                                                       removalInfo += QString("\nSummary of Remaining Lines:\n");
                                                       removalInfo += QString("Total Lines: %1\n").arg(remainingLines.size());
                                                       removalInfo += QString("In-Object Lines: %1\n").arg(remainingInObjectCount);
                                                       removalInfo += QString("Isolated Lines: %1\n").arg(remainingIsolatedCount);

                                                       // Update the info label and adjust its height
                                                       m_darkLineInfoLabel->setText(removalInfo);

                                                       // Adjust scroll area height based on content
                                                       QFontMetrics fm(m_darkLineInfoLabel->font());
                                                       int textHeight = fm.lineSpacing() * removalInfo.count('\n') + 40;
                                                       int preferredHeight = qMin(textHeight, 300);
                                                       preferredHeight = qMax(preferredHeight, 30);

                                                       QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
                                                           qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
                                                           );
                                                       darkLineScrollArea->setFixedHeight(preferredHeight);

                                                       // Show the info label and scroll area
                                                       m_darkLineInfoLabel->setVisible(true);
                                                       darkLineScrollArea->setVisible(true);

                                                       // Update detected lines with remaining lines
                                                       m_detectedLines = remainingLines;

                                                       // Update image display
                                                       updateImageDisplay();

                                                       // Update last action without parameters
                                                       updateLastAction("Remove Lines");
                                                   }
                                               }}
                                          });
}

void ControlPanel::updateImageDisplay() {
    const auto& finalImage = m_imageProcessor.getFinalImage();
    if (!finalImage.empty()) {
        int height = finalImage.size();
        int width = finalImage[0].size();

        // Update image size label
        m_imageSizeLabel->setText(QString("Image Size: %1 x %2").arg(width).arg(height));

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

        // Draw detected black lines if any exist
        if (!m_detectedLines.empty()) {
            for (size_t i = 0; i < m_detectedLines.size(); ++i) {
                const auto& line = m_detectedLines[i];
                QColor lineColor = line.inObject ? Qt::blue : Qt::red;
                painter.setPen(QPen(lineColor, 2, Qt::SolidLine));

                QString labelText = QString("Line %1 - %2")
                                        .arg(i + 1)
                                        .arg(line.inObject ? "In Object" : "Isolated");

                if (line.isVertical) {
                    QRect lineRect(line.x, 0, line.width, height - 1);
                    painter.drawRect(lineRect);

                    int labelY = 10 + i * 25;
                    QFontMetrics fm(painter.font());
                    int labelWidth = fm.horizontalAdvance(labelText) + 10;
                    QRect textRect(line.x + line.width + 5, labelY, labelWidth, 20);

                    painter.fillRect(textRect, QColor(255, 255, 255, 230));
                    painter.setPen(Qt::black);
                    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, " " + labelText);
                } else {
                    QRect lineRect(0, line.y, width - 1, line.width);
                    painter.drawRect(lineRect);

                    QFontMetrics fm(painter.font());
                    int labelWidth = fm.horizontalAdvance(labelText) + 10;
                    QRect textRect(10, line.y + line.width + 5, labelWidth, 20);

                    painter.fillRect(textRect, QColor(255, 255, 255, 230));
                    painter.setPen(Qt::black);
                    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, " " + labelText);
                }
            }
        }

        // Draw selection rectangle if exists
        if (m_imageLabel->isRegionSelected()) {
            painter.setPen(QPen(Qt::blue, 2));
            QRect selectedRegion = m_imageLabel->getSelectedRegion();
            if (m_zoomModeActive) {
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
    } else {
        // No image loaded - update labels accordingly
        m_imageSizeLabel->setText("Image Size: No image loaded");
        m_imageLabel->clear();
        // Instead of calling clear(), we'll just hide the histogram if it's visible
        if (m_histogram->isVisible()) {
            m_histogram->setVisible(false);
        }
    }
}

void ControlPanel::resetDetectedLines() {
    m_detectedLines.clear();
    m_imageProcessor.clearDetectedLines();

    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
        qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
        );

    m_darkLineInfoLabel->clear();
    m_darkLineInfoLabel->setVisible(false);
    darkLineScrollArea->setVisible(false);

    updateImageDisplay();
}
