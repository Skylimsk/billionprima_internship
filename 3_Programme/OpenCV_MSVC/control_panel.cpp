#include "control_panel.h"
#include "adjustments.h"
#include "darkline_pointer.h"
#include "interlace.h"
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
    : QWidget(parent)
    , m_imageProcessor(imageProcessor)
    , m_imageLabel(imageLabel)
    , m_lastGpuTime(-1)
    , m_lastCpuTime(-1)
    , m_hasCpuClaheTime(false)
    , m_hasGpuClaheTime(false)
    , m_zoomControlsGroup(nullptr)
    , m_zoomWarningBox(nullptr)
    , m_zoomButton(nullptr)  // Initialize m_zoomButton
{
    m_mainLayout = new QVBoxLayout(this);
    this->setMinimumWidth(280);

    // Connect the fileLoaded signal to enableButtons slot
    connect(this, &ControlPanel::fileLoaded, this, &ControlPanel::enableButtons);

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

    // Setup zoom warning box before setupBasicOperations
    m_zoomWarningBox = new QMessageBox(this);
    m_zoomWarningBox->setIcon(QMessageBox::Warning);
    m_zoomWarningBox->setWindowTitle("Zoom Mode Active");
    m_zoomWarningBox->setText("No functions can be applied while in Zoom Mode.\nPlease exit Zoom Mode first.");
    m_zoomWarningBox->setStandardButtons(QMessageBox::Ok);

    // Setup components in the correct order
    setupFileOperations();
    setupPreProcessingOperations();
    setupZoomControls();  // Setup zoom controls before basic operations
    setupBasicOperations();
    setupFilteringOperations();
    setupAdvancedOperations();
    setupCLAHEOperations();
    setupBlackLineDetection();
    setupPointerProcessing();
    setupGlobalAdjustments();
    setupRegionalAdjustments();

    scrollWidget->setLayout(m_scrollLayout);
    m_scrollArea->setWidget(scrollWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_mainLayout->addWidget(m_scrollArea);
    setLayout(m_mainLayout);
}

void ControlPanel::enableButtons(bool enable) {
    for (QPushButton* button : m_allButtons) {
        if (button && button->text() != "Browse") {
            button->setEnabled(enable);
        }
    }
}

bool ControlPanel::checkZoomMode() {
    auto& zoomManager = m_imageProcessor.getZoomManager();
    // Return true only if zoom mode is active and NOT fixed
    return (zoomManager.isZoomModeActive() && !zoomManager.isZoomFixed());
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
    toggleHistogramButton->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(toggleHistogramButton);
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

    // Connect fileLoaded signal to enable histogram button
    connect(this, &ControlPanel::fileLoaded, this, [toggleHistogramButton](bool loaded) {
        toggleHistogramButton->setEnabled(loaded);
    });

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

    QPushButton* zoomInBtn = new QPushButton("Zoom In");
    QPushButton* zoomOutBtn = new QPushButton("Zoom Out");
    QPushButton* resetZoomBtn = new QPushButton("Reset Zoom");
    m_fixZoomButton = new QPushButton("Fix Zoom");
    m_fixZoomButton->setCheckable(true);

    // Store pointers to zoom buttons for enabling/disabling
    m_zoomInButton = zoomInBtn;
    m_zoomOutButton = zoomOutBtn;
    m_resetZoomButton = resetZoomBtn;

    // Initially disable all zoom buttons
    zoomInBtn->setEnabled(false);
    zoomOutBtn->setEnabled(false);
    resetZoomBtn->setEnabled(false);
    m_fixZoomButton->setEnabled(false);

    // Add to m_allButtons for automatic enabling/disabling
    m_allButtons.push_back(zoomInBtn);
    m_allButtons.push_back(zoomOutBtn);
    m_allButtons.push_back(resetZoomBtn);
    m_allButtons.push_back(m_fixZoomButton);

    // Set fixed height for buttons
    const int buttonHeight = 35;
    zoomInBtn->setFixedHeight(buttonHeight);
    zoomOutBtn->setFixedHeight(buttonHeight);
    resetZoomBtn->setFixedHeight(buttonHeight);
    m_fixZoomButton->setFixedHeight(buttonHeight);

    // Add tooltips
    zoomInBtn->setToolTip("Increase zoom level by 20%");
    zoomOutBtn->setToolTip("Decrease zoom level by 20%");
    resetZoomBtn->setToolTip("Reset to original size (100%)");
    m_fixZoomButton->setToolTip("Fix current zoom level for image processing");

    zoomLayout->addWidget(zoomInBtn);
    zoomLayout->addWidget(zoomOutBtn);
    zoomLayout->addWidget(resetZoomBtn);
    zoomLayout->addWidget(m_fixZoomButton);

    // Connect zoom buttons
    connect(zoomInBtn, &QPushButton::clicked, [this]() {
        m_imageLabel->clearSelection();
        m_imageProcessor.getZoomManager().zoomIn();
        updateImageDisplay();
        updateLastAction("Zoom In", QString("%1x").arg(m_imageProcessor.getZoomManager().getZoomLevel(), 0, 'f', 2));
    });

    connect(zoomOutBtn, &QPushButton::clicked, [this]() {
        m_imageLabel->clearSelection();
        m_imageProcessor.getZoomManager().zoomOut();
        updateImageDisplay();
        updateLastAction("Zoom Out", QString("%1x").arg(m_imageProcessor.getZoomManager().getZoomLevel(), 0, 'f', 2));
    });

    connect(resetZoomBtn, &QPushButton::clicked, [this]() {
        m_imageLabel->clearSelection();
        m_imageProcessor.getZoomManager().resetZoom();
        m_fixZoomButton->setChecked(false);
        m_imageProcessor.getZoomManager().toggleFixedZoom(false);
        updateImageDisplay();
        updateLastAction("Reset Zoom", "1x");
    });

    connect(m_fixZoomButton, &QPushButton::toggled, [this](bool checked) {
        auto& zoomManager = m_imageProcessor.getZoomManager();
        zoomManager.toggleFixedZoom(checked);

        // Enable all processing buttons regardless of fix state
        // Only control zoom buttons based on fix state
        for (QPushButton* button : m_allButtons) {
            if (button) {
                QString buttonText = button->text();
                if (buttonText == "Browse") {
                    // Browse always enabled
                    continue;
                } else if (buttonText == "Zoom In" ||
                           buttonText == "Zoom Out" ||
                           buttonText == "Reset Zoom") {
                    // Zoom control buttons enabled only when not fixed
                    button->setEnabled(!checked);
                } else if (buttonText == "Deactivate Zoom") {
                    // Deactivate button always enabled
                    button->setEnabled(true);
                } else {
                    // All other processing buttons always enabled
                    button->setEnabled(true);
                }
            }
        }

        m_fixZoomButton->setText(checked ? "Unfix Zoom" : "Fix Zoom");
        QString status = checked ?
                             QString("Fixed at %1x").arg(zoomManager.getZoomLevel(), 0, 'f', 2) :
                             QString("Unfixed at %1x").arg(zoomManager.getZoomLevel(), 0, 'f', 2);
        updateLastAction("Fix Zoom", status);
    });

    m_zoomControlsGroup->hide();
}

void ControlPanel::toggleZoomMode(bool active) {
    if (!m_zoomButton) return;

    auto& zoomManager = m_imageProcessor.getZoomManager();
    if (zoomManager.isZoomModeActive() == active) return;

    if (!active && zoomManager.isZoomFixed()) {
        QMessageBox::warning(this, "Warning",
                             "Cannot deactivate zoom mode while zoom is fixed.\nPlease unfix zoom first.");
        m_zoomButton->setChecked(true);
        return;
    }

    if (!active) {
        float currentZoom = zoomManager.getZoomLevel();
        zoomManager.toggleZoomMode(false);

        if (m_zoomControlsGroup) {
            m_zoomControlsGroup->hide();
            m_scrollLayout->removeWidget(m_zoomControlsGroup);
        }

        // Re-enable all buttons except Browse when deactivating zoom
        for (QPushButton* button : m_allButtons) {
            if (button && button->text() != "Browse") {
                button->setEnabled(true);
            }
        }

        updateImageDisplay();
        updateLastAction("Zoom Mode", QString("Deactivated (Maintained %1x)").arg(currentZoom, 0, 'f', 2));

        m_zoomButton->setChecked(false);
        m_zoomButton->setText("Activate Zoom");
        m_zoomButton->setProperty("state", "");
    } else {
        zoomManager.toggleZoomMode(true);

        if (m_zoomControlsGroup) {
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
        }

        // Disable all buttons except Browse and zoom-related buttons when activating zoom
        for (QPushButton* button : m_allButtons) {
            if (button) {
                QString buttonText = button->text();
                bool isZoomRelated = (buttonText == "Activate Zoom" ||
                                      buttonText == "Deactivate Zoom" ||
                                      buttonText == "Zoom In" ||
                                      buttonText == "Zoom Out" ||
                                      buttonText == "Reset Zoom" ||
                                      buttonText == "Fix Zoom" ||
                                      buttonText == "Unfix Zoom" ||
                                      buttonText == "Browse");
                button->setEnabled(isZoomRelated);
            }
        }

        m_zoomButton->setChecked(true);
        m_zoomButton->setText("Deactivate Zoom");
        m_zoomButton->setProperty("state", zoomManager.isZoomFixed() ? "deactivate-fix" : "deactivate-unfix");

        updateLastAction("Zoom Mode", "Zoom Mode Activated");
    }

    m_zoomButton->style()->unpolish(m_zoomButton);
    m_zoomButton->style()->polish(m_zoomButton);

    updateImageDisplay();
}

void ControlPanel::setupFileOperations()
{
    // Initially disable all buttons except Browse
    connect(this, &ControlPanel::fileLoaded, this, &ControlPanel::enableButtons);

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
                                                    emit fileLoaded(true);
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
                                                   m_darkLineInfoLabel->hide();
                                                   m_imageLabel->clearSelection();
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
    if (!m_zoomButton) {
        m_zoomButton = new QPushButton("Activate Zoom", this);
        m_zoomButton->setCheckable(true);
        m_zoomButton->setFixedHeight(35);
        m_zoomButton->setEnabled(false);  // Initially disabled
        m_allButtons.push_back(m_zoomButton);

        m_zoomButton->setStyleSheet(
            "QPushButton {"
            "    background-color: #f0f0f0;"
            "    border: 1px solid #c0c0c0;"
            "    border-radius: 4px;"
            "    padding: 5px;"
            "}"
            "QPushButton:hover {"
            "    background-color: #e0e0e0;"
            "}"
            "QPushButton[state=\"deactivate-fix\"] {"  // Red background for Deactivate + Fix
            "    background-color: #ff4444;"
            "    color: white;"
            "    border: 1px solid #cc0000;"
            "}"
            "QPushButton[state=\"deactivate-fix\"]:hover {"
            "    background-color: #ff6666;"
            "}"
            "QPushButton[state=\"deactivate-unfix\"] {"  // Blue background for Deactivate + Unfix
            "    background-color: #4444ff;"
            "    color: white;"
            "    border: 1px solid #0000cc;"
            "}"
            "QPushButton[state=\"deactivate-unfix\"]:hover {"
            "    background-color: #6666ff;"
            "}"
            );

        // Connect the zoom button signal
        if (m_zoomButton) {  // Add null check
            connect(m_zoomButton, &QPushButton::clicked, this, [this]() {
                auto& zoomManager = m_imageProcessor.getZoomManager();
                bool isActive = zoomManager.isZoomModeActive();
                bool isFixed = zoomManager.isZoomFixed();

                if (!isActive) {
                    toggleZoomMode(true);
                } else {
                    if (isFixed) {
                        QMessageBox::warning(this, "Warning",
                                             "Cannot deactivate zoom mode while zoom is fixed.\nPlease unfix zoom first.");
                        return;
                    }
                    toggleZoomMode(false);
                }
            });
        }
    }

    // Connect to fix zoom button to update main zoom button state
    connect(m_fixZoomButton, &QPushButton::toggled, this, [this](bool checked) {
        if (m_zoomButton && m_zoomButton->text() == "Deactivate Zoom") {
            m_zoomButton->setProperty("state", checked ? "deactivate-fix" : "deactivate-unfix");
            m_zoomButton->style()->unpolish(m_zoomButton);
            m_zoomButton->style()->polish(m_zoomButton);
        }
    });
    createGroupBox("Basic Operations", {
                                           {"Zoom", m_zoomButton},  // Pass the button instead of creating a new one
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
                                                    QMessageBox::information(this, "Crop Info", "Please select a region first.");
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

void ControlPanel::setupPreProcessingOperations() {
    QGroupBox* groupBox = new QGroupBox("Pre-Processing Operations");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);

    // Create calibration button
    m_calibrationButton = new QPushButton("Calibration");
    m_calibrationButton->setFixedHeight(35);
    m_calibrationButton->setToolTip("Apply calibration using Y-axis and X-axis parameters");
    m_calibrationButton->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(m_calibrationButton);
    layout->addWidget(m_calibrationButton);

    // Create reset calibration button
    m_resetCalibrationButton = new QPushButton("Reset Calibration Parameters");
    m_resetCalibrationButton->setFixedHeight(35);
    m_resetCalibrationButton->setEnabled(false); // Initially disabled
    m_resetCalibrationButton->setToolTip("Reset stored calibration parameters");
    m_allButtons.push_back(m_resetCalibrationButton);
    layout->addWidget(m_resetCalibrationButton);

    // Create Enhanced Interlace button
    QPushButton* enhancedInterlaceBtn = new QPushButton("Enhanced Interlace");
    enhancedInterlaceBtn->setFixedHeight(35);
    enhancedInterlaceBtn->setToolTip("Process image using enhanced interlacing method");
    enhancedInterlaceBtn->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(enhancedInterlaceBtn);
    layout->addWidget(enhancedInterlaceBtn);

    // Connect calibration button
    connect(m_calibrationButton, &QPushButton::clicked, this, [this]() {
        if (checkZoomMode()) return;
        m_darkLineInfoLabel->hide();
        resetDetectedLines();

        if (!InterlaceProcessor::hasCalibrationParams()) {
            // First time calibration - ask for parameters
            auto [linesToProcessY, yOk] = showInputDialog(
                "Calibration",
                "Enter lines to process for Y-axis:",
                10, 1, 1000
                );
            if (!yOk) return;

            auto [linesToProcessX, xOk] = showInputDialog(
                "Calibration",
                "Enter lines to process for X-axis:",
                10, 1, 1000
                );
            if (!xOk) return;

            m_imageProcessor.processYXAxis(
                const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage()),
                linesToProcessY,
                linesToProcessX
                );

            // Enable reset button after first calibration
            m_resetCalibrationButton->setEnabled(true);
        } else {
            // Use existing parameters
            m_imageProcessor.processYXAxisWithStoredParams(
                const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage())
                );
        }

        m_imageLabel->clearSelection();
        updateImageDisplay();
        updateLastAction("Calibration", InterlaceProcessor::getCalibrationParamsString());
        updateCalibrationButtonText();
    });

    // Connect reset button
    connect(m_resetCalibrationButton, &QPushButton::clicked, this, [this]() {
        InterlaceProcessor::resetCalibrationParams();
        m_resetCalibrationButton->setEnabled(false);
        updateCalibrationButtonText();
        QMessageBox::information(this, "Calibration Reset",
                                 "Calibration parameters have been reset. Next calibration will require new parameters.");
    });

    // Connect Enhanced Interlace button
    connect(enhancedInterlaceBtn, &QPushButton::clicked, this, [this]() {
        if (checkZoomMode()) return;
        m_darkLineInfoLabel->hide();
        resetDetectedLines();

        // Create dialog for options
        QDialog dialog(this);
        dialog.setWindowTitle("Enhanced Interlace Options");
        dialog.setMinimumWidth(400);
        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        // Low Energy Section
        QGroupBox* lowEnergyBox = new QGroupBox("Low Energy Section Start");
        QVBoxLayout* lowEnergyLayout = new QVBoxLayout(lowEnergyBox);
        QRadioButton* leftLeftLowRadio = new QRadioButton("Start with Left Left");
        QRadioButton* leftRightLowRadio = new QRadioButton("Start with Left Right");
        leftLeftLowRadio->setChecked(true);

        // Add explanatory label for Low Energy
        QLabel* lowEnergyExplanation = new QLabel(
            "Low Energy interlace pattern will be:\n"
            "Row 1: Selected start section\n"
            "Row 2: Other section\n"
            "Repeating for each original row"
            );
        lowEnergyExplanation->setStyleSheet("color: #666; font-size: 10px;");

        lowEnergyLayout->addWidget(leftLeftLowRadio);
        lowEnergyLayout->addWidget(leftRightLowRadio);
        lowEnergyLayout->addWidget(lowEnergyExplanation);
        dialogLayout->addWidget(lowEnergyBox);

        // High Energy Section
        QGroupBox* highEnergyBox = new QGroupBox("High Energy Section Start");
        QVBoxLayout* highEnergyLayout = new QVBoxLayout(highEnergyBox);
        QRadioButton* rightLeftHighRadio = new QRadioButton("Start with Right Left");
        QRadioButton* rightRightHighRadio = new QRadioButton("Start with Right Right");
        rightLeftHighRadio->setChecked(true);

        // Add explanatory label for High Energy
        QLabel* highEnergyExplanation = new QLabel(
            "High Energy interlace pattern will be:\n"
            "Row 1: Selected start section\n"
            "Row 2: Other section\n"
            "Repeating for each original row"
            );
        highEnergyExplanation->setStyleSheet("color: #666; font-size: 10px;");

        highEnergyLayout->addWidget(rightLeftHighRadio);
        highEnergyLayout->addWidget(rightRightHighRadio);
        highEnergyLayout->addWidget(highEnergyExplanation);
        dialogLayout->addWidget(highEnergyBox);

        // Merge Method Selection
        QGroupBox* mergeBox = new QGroupBox("Merge Method");
        QVBoxLayout* mergeLayout = new QVBoxLayout(mergeBox);
        QRadioButton* weightedAvgRadio = new QRadioButton("Weighted Average");
        QRadioButton* minValueRadio = new QRadioButton("Minimum Value");
        weightedAvgRadio->setChecked(true);

        // Add explanatory label for Merge Method
        QLabel* mergeExplanation = new QLabel(
            "Weighted Average: Average of corresponding pixels\n"
            "Minimum Value: Choose the smaller value between corresponding pixels"
            );
        mergeExplanation->setStyleSheet("color: #666; font-size: 10px;");

        mergeLayout->addWidget(weightedAvgRadio);
        mergeLayout->addWidget(minValueRadio);
        mergeLayout->addWidget(mergeExplanation);
        dialogLayout->addWidget(mergeBox);

        // Add note about automatic calibration
        QLabel* autoCalibrationNote = new QLabel(
            "\nNote: After processing, the system will automatically perform "
            "calibration using your previous calibration parameters "
            "(if available).\n"
            "This ensures consistent results without requiring additional input."
            );
        autoCalibrationNote->setStyleSheet(
            "color: #444;"
            "font-style: italic;"
            "font-size: 10px;"
            "background-color: #f8f8f8;"
            "padding: 8px;"
            "margin-top: 5px;"
            "margin-bottom: 5px;"
            "border-radius: 4px;"
            "border: 1px solid #ddd;"
            );
        autoCalibrationNote->setWordWrap(true);
        dialogLayout->addWidget(autoCalibrationNote);

        // Show current calibration parameters if available
        if (InterlaceProcessor::hasCalibrationParams()) {
            QLabel* currentParamsLabel = new QLabel(
                QString("Current calibration parameters: %1")
                    .arg(InterlaceProcessor::getCalibrationParamsString())
                );
            currentParamsLabel->setStyleSheet(
                "color: #0066cc;"
                "font-size: 10px;"
                "margin-bottom: 8px;"
                );
            dialogLayout->addWidget(currentParamsLabel);
        }

        // Add buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
            Qt::Horizontal, &dialog);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            // Convert UI selections to InterlaceProcessor types
            InterlaceProcessor::StartPoint lowEnergyStart =
                leftLeftLowRadio->isChecked() ?
                    InterlaceProcessor::StartPoint::LEFT_LEFT :
                    InterlaceProcessor::StartPoint::LEFT_RIGHT;

            InterlaceProcessor::StartPoint highEnergyStart =
                rightLeftHighRadio->isChecked() ?
                    InterlaceProcessor::StartPoint::RIGHT_LEFT :
                    InterlaceProcessor::StartPoint::RIGHT_RIGHT;

            InterlaceProcessor::MergeMethod mergeMethod =
                weightedAvgRadio->isChecked() ?
                    InterlaceProcessor::MergeMethod::WEIGHTED_AVERAGE :
                    InterlaceProcessor::MergeMethod::MINIMUM_VALUE;

            // Process the image with selected options
            m_imageProcessor.processEnhancedInterlacedSections(
                lowEnergyStart,
                highEnergyStart,
                mergeMethod
                );

            m_imageLabel->clearSelection();
            updateImageDisplay();

            // Create status message
            QString lowEnergyStr = leftLeftLowRadio->isChecked() ? "LeftLeft" : "LeftRight";
            QString highEnergyStr = rightLeftHighRadio->isChecked() ? "RightLeft" : "RightRight";
            QString mergeStr = weightedAvgRadio->isChecked() ? "WeightedAvg" : "MinValue";

            // Add calibration info to status if available
            QString statusMsg = QString("Low: %1, High: %2, Merge: %3")
                                    .arg(lowEnergyStr)
                                    .arg(highEnergyStr)
                                    .arg(mergeStr);

            if (InterlaceProcessor::hasCalibrationParams()) {
                statusMsg += QString(" (Auto-calibrated: %1)")
                .arg(InterlaceProcessor::getCalibrationParamsString());
            }

            updateLastAction("Enhanced Interlace", statusMsg);
        }
    });

    layout->setSpacing(10);
    layout->addStretch();
    m_scrollLayout->addWidget(groupBox);
    updateCalibrationButtonText();
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

void ControlPanel::createGroupBox(const QString& title,
                                  const std::vector<std::pair<QString, std::variant<std::function<void()>, QPushButton*>>>& buttons)
{
    // Clear previous buttons if this is a new setup
    if (title == "File Operations") {
        m_allButtons.clear();
    }

    QGroupBox* groupBox = new QGroupBox(title);
    QVBoxLayout* groupLayout = new QVBoxLayout(groupBox);

    const int groupBoxMinWidth = 250;
    const int buttonHeight = 35;

    groupBox->setMinimumWidth(groupBoxMinWidth);
    groupBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    for (const auto& button_pair : buttons) {
        QPushButton* button;
        if (auto* existingButton = std::get_if<QPushButton*>(&button_pair.second)) {
            // Use existing button
            button = *existingButton;
        } else {
            // Create new button
            button = new QPushButton(button_pair.first);
            button->setFixedHeight(buttonHeight);
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            // Store button in the vector and disable if it's not Browse
            m_allButtons.push_back(button);
            if (button->text() != "Browse") {
                button->setEnabled(false);
            }

            // Connect the lambda if provided
            if (auto* func = std::get_if<std::function<void()>>(&button_pair.second)) {
                connect(button, &QPushButton::clicked, *func);
            }
        }
        groupLayout->addWidget(button);
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

                                                // Line selection list with multi-selection for neighbor values
                                                QGroupBox* lineSelectionBox = new QGroupBox("Select Lines to Process");
                                                QVBoxLayout* selectionLayout = new QVBoxLayout();
                                                QListWidget* lineList = new QListWidget();

                                                // Set selection mode based on the method
                                                lineList->setSelectionMode(QAbstractItemView::ExtendedSelection);  // 默认允许多选

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

                                                // Connect stitch radio button to change selection mode
                                                connect(stitchRadio, &QRadioButton::toggled, [lineList](bool checked) {
                                                    lineList->setSelectionMode(checked ?
                                                                                   QAbstractItemView::SingleSelection :
                                                                                   QAbstractItemView::ExtendedSelection);
                                                    if (checked && lineList->selectedItems().count() > 1) {
                                                        // If switching to single selection and multiple items are selected,
                                                        // keep only the first selection
                                                        lineList->clearSelection();
                                                        if (lineList->count() > 0) {
                                                            lineList->item(0)->setSelected(true);
                                                        }
                                                    }
                                                });

                                                lineList->setMinimumHeight(200);  // 增加列表高度以便于多选
                                                lineList->setMaximumHeight(300);
                                                selectionLayout->addWidget(lineList);
                                                lineSelectionBox->setLayout(selectionLayout);
                                                layout->addWidget(lineSelectionBox);

                                                // Add a select all button for neighbor values mode
                                                QPushButton* selectAllButton = new QPushButton("Select All Lines");
                                                selectAllButton->setVisible(!stitchRadio->isChecked());
                                                connect(selectAllButton, &QPushButton::clicked, lineList, &QListWidget::selectAll);
                                                layout->addWidget(selectAllButton);

                                                // Connect radio buttons to toggle select all button visibility
                                                connect(stitchRadio, &QRadioButton::toggled, selectAllButton, &QPushButton::setHidden);
                                                connect(neighborValuesRadio, &QRadioButton::toggled, selectAllButton, &QPushButton::setVisible);

                                                // Control visibility logic
                                                auto updateVisibility = [&]() {
                                                    bool isInObject = inObjectRadio->isChecked();
                                                    methodBox->setVisible(isInObject);
                                                    lineSelectionBox->setVisible(isInObject);
                                                    selectAllButton->setVisible(isInObject && neighborValuesRadio->isChecked());
                                                    dialog.adjustSize();
                                                };

                                                connect(inObjectRadio, &QRadioButton::toggled, updateVisibility);
                                                connect(isolatedRadio, &QRadioButton::toggled, updateVisibility);
                                                connect(neighborValuesRadio, &QRadioButton::toggled, updateVisibility);
                                                connect(stitchRadio, &QRadioButton::toggled, updateVisibility);

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

                                                        // Get selected lines
                                                        auto selectedItems = lineList->selectedItems();
                                                        if (selectedItems.isEmpty()) {
                                                            QMessageBox::warning(this, "Warning", "Please select at least one line for processing.");
                                                            return;
                                                        }

                                                        // Collect all selected lines
                                                        for (QListWidgetItem* item : selectedItems) {
                                                            int index = item->data(Qt::UserRole).toInt();
                                                            selectedLines.push_back(m_detectedLines[index]);
                                                        }

                                                        // Process the selected lines
                                                        m_imageProcessor.removeDarkLinesSequential(
                                                            selectedLines,
                                                            true,   // removeInObject
                                                            false,  // removeIsolated
                                                            method
                                                            );
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

void ControlPanel::updateDarkLineInfoDisplay() {
    QFontMetrics fm(m_darkLineInfoLabel->font());
    QString text = m_darkLineInfoLabel->text();
    int textHeight = fm.lineSpacing() * text.count('\n') + 40;
    int preferredHeight = qMin(textHeight, 300);
    preferredHeight = qMax(preferredHeight, 30);

    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
        qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
        );
    darkLineScrollArea->setFixedHeight(preferredHeight);

    m_darkLineInfoLabel->setVisible(true);
    darkLineScrollArea->setVisible(true);
}

void ControlPanel::updateImageDisplay() {
    const auto& finalImage = m_imageProcessor.getFinalImage();
    if (!finalImage.empty()) {
        int height = finalImage.size();
        int width = finalImage[0].size();

        // Update image size label with both original and zoomed dimensions
        const auto& zoomManager = m_imageProcessor.getZoomManager();
        if (zoomManager.isZoomModeActive() && zoomManager.getZoomLevel() != 1.0f) {
            QSize zoomedSize = zoomManager.getZoomedSize(QSize(width, height));
            m_imageSizeLabel->setText(QString("Image Size: %1 x %2 (Zoomed: %3 x %4)")
                                          .arg(width).arg(height)
                                          .arg(zoomedSize.width()).arg(zoomedSize.height()));
        } else {
            m_imageSizeLabel->setText(QString("Image Size: %1 x %2").arg(width).arg(height));
        }

        // Create the base image
        QImage image(width, height, QImage::Format_Grayscale16);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint16_t pixelValue = finalImage[y][x];
                image.setPixel(x, y, qRgb(pixelValue >> 8, pixelValue >> 8, pixelValue >> 8));
            }
        }

        // Create base pixmap and apply zoom if needed
        QPixmap pixmap = QPixmap::fromImage(image);
        if (zoomManager.isZoomModeActive() && zoomManager.getZoomLevel() != 1.0f) {
            QSize zoomedSize = zoomManager.getZoomedSize(pixmap.size());
            pixmap = pixmap.scaled(zoomedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }

        // Start painting overlays
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);

        if (!m_detectedLines.empty()) {
            float penWidth = zoomManager.isZoomModeActive() ?
                                 std::max(1.0f, 2.0f * zoomManager.getZoomLevel()) : 2.0f;  // Ensure minimum pen width

            for (size_t i = 0; i < m_detectedLines.size(); ++i) {
                const auto& line = m_detectedLines[i];
                QColor lineColor = line.inObject ? Qt::blue : Qt::red;
                painter.setPen(QPen(lineColor, penWidth, Qt::SolidLine));

                QString labelText = QString("Line %1 - %2")
                                        .arg(i + 1)
                                        .arg(line.inObject ? "In Object" : "Isolated");

                // Calculate base font size that maintains readability
                float baseFontSize = painter.font().pointSizeF();
                float scaledFontSize;
                float minFontSize = 8.0f;  // Minimum readable font size

                if (zoomManager.isZoomModeActive()) {
                    if (zoomManager.getZoomLevel() < 1.0f) {
                        // When zoomed out, maintain minimum readable size
                        scaledFontSize = std::max(minFontSize, baseFontSize);
                    } else {
                        // When zoomed in, scale up normally
                        scaledFontSize = baseFontSize * zoomManager.getZoomLevel();
                    }
                } else {
                    scaledFontSize = baseFontSize;
                }

                // Set font size
                QFont font = painter.font();
                font.setPointSizeF(scaledFontSize);
                painter.setFont(font);

                // Calculate label metrics with new font
                QFontMetrics fm(font);
                float labelWidth = fm.horizontalAdvance(labelText) + 10;
                float labelHeight = std::max(20.0f, fm.height() + 4.0f);

                // Calculate label spacing that maintains readability
                float baseSpacing = 25.0f;
                float labelSpacing = zoomManager.isZoomModeActive() ?
                                         std::max(baseSpacing, baseSpacing * zoomManager.getZoomLevel()) : baseSpacing;

                if (line.isVertical) {
                    // Draw vertical line
                    QRect lineRect(line.x, 0, line.width, height - 1);
                    if (zoomManager.isZoomModeActive()) {
                        lineRect = zoomManager.getZoomedRect(lineRect);
                    }
                    painter.drawRect(lineRect);

                    // Position label with minimum spacing from line
                    float minOffset = 5.0f;
                    float labelX = lineRect.right() + minOffset;
                    float labelY = 10.0f + i * labelSpacing;

                    // Create semi-transparent background for better readability
                    QRectF textRect(labelX, labelY, labelWidth, labelHeight);
                    painter.fillRect(textRect, QColor(255, 255, 255, 230));

                    // Draw text
                    painter.setPen(Qt::black);
                    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, " " + labelText);

                } else {
                    // Draw horizontal line
                    QRect lineRect(0, line.y, width - 1, line.width);
                    if (zoomManager.isZoomModeActive()) {
                        lineRect = zoomManager.getZoomedRect(lineRect);
                    }
                    painter.drawRect(lineRect);

                    // Position label with minimum spacing from line
                    float minOffset = 5.0f;
                    float labelX = 10.0f;
                    float labelY = lineRect.bottom() + minOffset;

                    // Create semi-transparent background for better readability
                    QRectF textRect(labelX, labelY, labelWidth, labelHeight);
                    painter.fillRect(textRect, QColor(255, 255, 255, 230));

                    // Draw text
                    painter.setPen(Qt::black);
                    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, " " + labelText);
                }
            }
        }

        // Draw selection rectangle if exists
        if (m_imageLabel->isRegionSelected()) {
            float selectionPenWidth = zoomManager.isZoomModeActive() ?
                                          2.0f * zoomManager.getZoomLevel() : 2.0f;
            painter.setPen(QPen(Qt::blue, selectionPenWidth));

            QRect selectedRegion = m_imageLabel->getSelectedRegion();
            if (zoomManager.isZoomModeActive()) {
                selectedRegion = zoomManager.getZoomedRect(selectedRegion);
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
        // No image loaded - clear display
        m_imageSizeLabel->setText("Image Size: No image loaded");
        m_imageLabel->clear();
        if (m_histogram->isVisible()) {
            m_histogram->setVisible(false);
        }
    }
}

// Helper function to draw line labels with proper zoom scaling
void ControlPanel :: drawLineLabel(QPainter& painter, const QString& text, const QPointF& pos, const ZoomManager& zoomManager) {
    QFont font = painter.font();
    QFontMetrics fm(font);

    // Scale font if zoomed
    if (zoomManager.isZoomModeActive()) {
        font.setPointSizeF(font.pointSizeF() * zoomManager.getZoomLevel());
        painter.setFont(font);
    }

    // Calculate label dimensions
    float labelWidth = fm.horizontalAdvance(text) + 10;
    float labelHeight = 20;
    if (zoomManager.isZoomModeActive()) {
        labelWidth *= zoomManager.getZoomLevel();
        labelHeight *= zoomManager.getZoomLevel();
    }

    QRectF textRect(pos.x(), pos.y(), labelWidth, labelHeight);

    // Draw label background
    painter.fillRect(textRect, QColor(255, 255, 255, 230));

    // Draw text
    painter.setPen(Qt::black);
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, " " + text);
}

void ControlPanel::updateCalibrationButtonText() {
    if (InterlaceProcessor::hasCalibrationParams()) {
        m_calibrationButton->setText(
            QString("Calibration (%1)")
                .arg(InterlaceProcessor::getCalibrationParamsString())
            );
    } else {
        m_calibrationButton->setText("Calibration");
    }
}

void ControlPanel::setupPointerProcessing() {
    createGroupBox("Process via Double 2D Pointer", {
                                                     {"Detect Lines (2D Pointer)", [this]() {
                                                          if (checkZoomMode()) return;

                                                          try {
                                                              m_darkLineInfoLabel->hide();
                                                              resetDetectedLines();

                                                              // Convert vector to DarkLineImageData
                                                              DarkLineImageData imageData = convertToImageData(m_imageProcessor.getFinalImage());

                                                              // Check if conversion was successful
                                                              if (!imageData.data || imageData.rows == 0 || imageData.cols == 0) {
                                                                  QMessageBox::warning(this, "Error", "Failed to convert image data");
                                                                  return;
                                                              }

                                                              // Detect lines using DarkLinePointerProcessor
                                                              DarkLinePtrArrayType* detectedLines = nullptr;
                                                              try {
                                                                  detectedLines = DarkLinePointerProcessor::detectDarkLines(imageData);
                                                                  if (!detectedLines) {
                                                                      throw std::runtime_error("Failed to detect lines");
                                                                  }
                                                              } catch (const std::exception& e) {
                                                                  // Clean up image data
                                                                  for (int i = 0; i < imageData.rows; ++i) {
                                                                      delete[] imageData.data[i];
                                                                  }
                                                                  delete[] imageData.data;

                                                                  QMessageBox::critical(this, "Error",
                                                                                        QString("Error during line detection: %1").arg(e.what()));
                                                                  return;
                                                              }

                                                              // Create detection info summary
                                                              QString detectionInfo = "Detected Lines (2D Pointer):\n\n";

                                                              // Count lines by type
                                                              int inObjectCount = 0;
                                                              int isolatedCount = 0;

                                                              // List all detected lines with details
                                                              for (int i = 0; i < detectedLines->count; ++i) {
                                                                  const auto& line = detectedLines->lines[i];

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
                                                              detectionInfo += QString("Total Lines: %1\n").arg(detectedLines->count);
                                                              detectionInfo += QString("In-Object Lines: %1\n").arg(inObjectCount);
                                                              detectionInfo += QString("Isolated Lines: %1\n").arg(isolatedCount);

                                                              // Update the info label
                                                              m_darkLineInfoLabel->setText(detectionInfo);
                                                              m_darkLineInfoLabel->setVisible(true);

                                                              // Store the detected lines for later use
                                                              m_detectedLines.clear();
                                                              for (int i = 0; i < detectedLines->count; ++i) {
                                                                  ImageProcessor::DarkLine line;
                                                                  line.x = detectedLines->lines[i].x;
                                                                  line.y = detectedLines->lines[i].y;
                                                                  line.width = detectedLines->lines[i].width;
                                                                  line.isVertical = detectedLines->lines[i].isVertical;
                                                                  line.inObject = detectedLines->lines[i].inObject;
                                                                  line.startX = detectedLines->lines[i].startX;
                                                                  line.startY = detectedLines->lines[i].startY;
                                                                  line.endX = detectedLines->lines[i].endX;
                                                                  line.endY = detectedLines->lines[i].endY;
                                                                  m_detectedLines.push_back(line);
                                                              }

                                                              // Clean up
                                                              delete[] detectedLines->lines;
                                                              delete detectedLines;

                                                              // Clean up image data
                                                              for (int i = 0; i < imageData.rows; ++i) {
                                                                  delete[] imageData.data[i];
                                                              }
                                                              delete[] imageData.data;

                                                              // Show the scroll area and update display
                                                              QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
                                                                  qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
                                                                  );
                                                              darkLineScrollArea->setVisible(true);

                                                              updateImageDisplay();
                                                              updateLastAction("Detect Lines (2D Pointer)");

                                                          } catch (const std::exception& e) {
                                                              QMessageBox::critical(this, "Error",
                                                                                    QString("Unexpected error during line detection: %1").arg(e.what()));
                                                          }
                                                      }},

                                                        {"Remove Lines (2D Pointer)", [this]() {
                                                             if (checkZoomMode()) return;

                                                             if (m_detectedLines.empty()) {
                                                                 QMessageBox::information(this, "Remove Lines",
                                                                                          "Please detect lines first using the 2D Pointer method.");
                                                                 return;
                                                             }

                                                             // Create dialog for removal options
                                                             QDialog dialog(this);
                                                             dialog.setWindowTitle("Remove Lines (2D Pointer)");
                                                             dialog.setMinimumWidth(400);
                                                             QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                                             // Method selection
                                                             QGroupBox* methodBox = new QGroupBox("Removal Method");
                                                             QVBoxLayout* methodLayout = new QVBoxLayout();
                                                             QRadioButton* neighborValuesRadio = new QRadioButton("Use Neighbor Values");
                                                             QRadioButton* stitchRadio = new QRadioButton("Direct Stitch");
                                                             neighborValuesRadio->setChecked(true);
                                                             methodLayout->addWidget(neighborValuesRadio);
                                                             methodLayout->addWidget(stitchRadio);
                                                             methodBox->setLayout(methodLayout);
                                                             layout->addWidget(methodBox);

                                                             // Line type selection
                                                             QGroupBox* typeBox = new QGroupBox("Line Type");
                                                             QVBoxLayout* typeLayout = new QVBoxLayout();
                                                             QRadioButton* allLinesRadio = new QRadioButton("All Lines");
                                                             QRadioButton* inObjectRadio = new QRadioButton("In-Object Lines Only");
                                                             QRadioButton* isolatedRadio = new QRadioButton("Isolated Lines Only");
                                                             allLinesRadio->setChecked(true);
                                                             typeLayout->addWidget(allLinesRadio);
                                                             typeLayout->addWidget(inObjectRadio);
                                                             typeLayout->addWidget(isolatedRadio);
                                                             typeBox->setLayout(typeLayout);
                                                             layout->addWidget(typeBox);

                                                             // Add OK and Cancel buttons
                                                             QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                                 QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                                             layout->addWidget(buttonBox);

                                                             connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                             connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                             if (dialog.exec() == QDialog::Accepted) {
                                                                 // Convert the current image to DarkLineImageData
                                                                 DarkLineImageData imageData = convertToImageData(m_imageProcessor.getFinalImage());

                                                                 // Create DarkLinePtrArray from m_detectedLines
                                                                 DarkLinePtrArrayType* lines = new DarkLinePtrArrayType();
                                                                 lines->lines = new DarkLinePtrType[m_detectedLines.size()];
                                                                 lines->count = m_detectedLines.size();
                                                                 lines->capacity = m_detectedLines.size();

                                                                 for (size_t i = 0; i < m_detectedLines.size(); ++i) {
                                                                     lines->lines[i].x = m_detectedLines[i].x;
                                                                     lines->lines[i].y = m_detectedLines[i].y;
                                                                     lines->lines[i].width = m_detectedLines[i].width;
                                                                     lines->lines[i].isVertical = m_detectedLines[i].isVertical;
                                                                     lines->lines[i].inObject = m_detectedLines[i].inObject;
                                                                     lines->lines[i].startX = m_detectedLines[i].startX;
                                                                     lines->lines[i].startY = m_detectedLines[i].startY;
                                                                     lines->lines[i].endX = m_detectedLines[i].endX;
                                                                     lines->lines[i].endY = m_detectedLines[i].endY;
                                                                 }

                                                                 // Determine removal method
                                                                 DarkLinePointerProcessor::RemovalMethod method = neighborValuesRadio->isChecked() ?
                                                                                                                      DarkLinePointerProcessor::RemovalMethod::NEIGHBOR_VALUES :
                                                                                                                      DarkLinePointerProcessor::RemovalMethod::DIRECT_STITCH;

                                                                 // Remove lines based on selection
                                                                 if (allLinesRadio->isChecked()) {
                                                                     DarkLinePointerProcessor::removeDarkLinesSelective(imageData, lines, true, true, method);
                                                                 } else if (inObjectRadio->isChecked()) {
                                                                     DarkLinePointerProcessor::removeDarkLinesSelective(imageData, lines, true, false, method);
                                                                 } else {
                                                                     DarkLinePointerProcessor::removeDarkLinesSelective(imageData, lines, false, true, method);
                                                                 }

                                                                 // Convert back to vector format and update image
                                                                 auto newImage = convertFromImageData(imageData);
                                                                 m_imageProcessor.updateAndSaveFinalImage(newImage);

                                                                 // Clean up
                                                                 delete[] lines->lines;
                                                                 delete lines;

                                                                 // Update display
                                                                 updateImageDisplay();
                                                                 updateLastAction("Remove Lines (2D Pointer)",
                                                                                  QString("%1 - %2")
                                                                                      .arg(neighborValuesRadio->isChecked() ? "Neighbor Values" : "Direct Stitch")
                                                                                      .arg(allLinesRadio->isChecked() ? "All Lines" :
                                                                                               inObjectRadio->isChecked() ? "In-Object Lines" : "Isolated Lines"));

                                                                 // Clear detected lines after removal
                                                                 resetDetectedLines();
                                                             }
                                                         }}
                                                    });
}

DarkLineImageData ControlPanel::convertToImageData(const std::vector<std::vector<uint16_t>>& image) {
    DarkLineImageData imageData;

    // Check for empty image
    if (image.empty() || image[0].empty()) {
        imageData.rows = 0;
        imageData.cols = 0;
        imageData.data = nullptr;
        return imageData;
    }

    imageData.rows = static_cast<int>(image.size());
    imageData.cols = static_cast<int>(image[0].size());

    try {
        // Allocate rows
        imageData.data = new double*[imageData.rows];

        // Allocate columns for each row
        for (int i = 0; i < imageData.rows; ++i) {
            imageData.data[i] = new double[imageData.cols];

            // Convert and copy data
            for (int j = 0; j < imageData.cols; ++j) {
                imageData.data[i][j] = static_cast<double>(image[i][j]);
            }
        }
    } catch (const std::bad_alloc& e) {
        // Clean up any allocated memory if allocation fails
        if (imageData.data) {
            for (int i = 0; i < imageData.rows; ++i) {
                delete[] imageData.data[i];
            }
            delete[] imageData.data;
            imageData.data = nullptr;
        }
        imageData.rows = 0;
        imageData.cols = 0;

        QMessageBox::critical(nullptr, "Error", "Failed to allocate memory for image data conversion");
        return imageData;
    }

    return imageData;
}

std::vector<std::vector<uint16_t>> ControlPanel::convertFromImageData(const DarkLineImageData& imageData) {
    std::vector<std::vector<uint16_t>> image(imageData.rows, std::vector<uint16_t>(imageData.cols));

    for (int i = 0; i < imageData.rows; ++i) {
        for (int j = 0; j < imageData.cols; ++j) {
            image[i][j] = static_cast<uint16_t>(std::round(imageData.data[i][j]));
        }
    }

    return image;
}
