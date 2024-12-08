#include "control_panel.h"
#include "pointer_operations.h"
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
#include <set>

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
    , m_zoomButton(nullptr)
    , m_detectedLinesPointer(nullptr)
    , m_lowEnergyWindow(nullptr)
    , m_highEnergyWindow(nullptr)
    , m_finalWindow(nullptr)
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
    setupBlackLineDetection();
    setupCombinedAdjustments();
    setupResetOperations();

    scrollWidget->setLayout(m_scrollLayout);
    m_scrollArea->setWidget(scrollWidget);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_mainLayout->addWidget(m_scrollArea);
    setLayout(m_mainLayout);
}

ControlPanel::~ControlPanel() {
    if (m_detectedLinesPointer) {
        DarkLinePointerProcessor::destroyDarkLineArray(m_detectedLinesPointer);
        m_detectedLinesPointer = nullptr;
    }
    if (m_lowEnergyWindow) {
        m_lowEnergyWindow->hide();
        m_lowEnergyWindow->close();
    }
    if (m_highEnergyWindow) {
        m_highEnergyWindow->hide();
        m_highEnergyWindow->close();
    }
    if (m_finalWindow) {
        m_finalWindow->hide();
        m_finalWindow->close();
    }
}

void ControlPanel::enableButtons(bool enable) {
    for (QPushButton* button : m_allButtons) {
        if (button && button->text() != "Browse" && button->text() != "Load Pointer") {
            button->setEnabled(enable);
        }
    }
}

bool ControlPanel::checkZoomMode() {
    auto& zoomManager = m_imageProcessor.getZoomManager();
    // Return true only if zoom mode is active and NOT fixed
    return (zoomManager.isZoomModeActive() && !zoomManager.isZoomFixed());
}

void ControlPanel::setupPixelInfoLabel() {
    QVBoxLayout* infoLayout = new QVBoxLayout();

    // Add image size label
    m_imageSizeLabel = new QLabel("Image Size: No image loaded");
    m_imageSizeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    infoLayout->addWidget(m_imageSizeLabel);

    m_pixelInfoLabel = new QLabel("Pixel Info: ");
    m_pixelInfoLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    infoLayout->addWidget(m_pixelInfoLabel);

    // Update last action label setup
    m_lastActionLabel = new QLabel("Last Action: None");
    m_lastActionLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_lastActionLabel->setWordWrap(true);
    infoLayout->addWidget(m_lastActionLabel);

    // Update last action parameters label setup
    m_lastActionParamsLabel = new QLabel("");
    m_lastActionParamsLabel->setMinimumHeight(25);
    m_lastActionParamsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_lastActionParamsLabel->setWordWrap(true);
    m_lastActionParamsLabel->setStyleSheet(
        "QLabel {"
        "    color: black;"
        "    background-color: #f8f8f8;"
        "    border: 1px solid #ddd;"
        "    border-radius: 4px;"
        "    padding: 4px 8px;"
        "    margin: 2px 0px;"
        "}"
        );
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

void ControlPanel::updateLastAction(const QString& action, const QString& parameters) {
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
        } else if (action == "Split & Merge") {
            prefix = "";
        } else if (action == "Zoom In" || action == "Zoom Out" || action == "Reset Zoom") {
            prefix = "Zoom Factor: ";
        } else if (action == "Zoom Mode") {
            prefix = "Status: ";
        } else {
            prefix = "Parameters: ";
        }
        m_lastActionParamsLabel->setText(prefix + parameters);
        m_lastActionParamsLabel->setVisible(true);
    }

    updateLastActionLabelSize();
    m_imageProcessor.setLastAction(action, parameters);
}

void ControlPanel::updateLastActionLabelSize() {
    // Calculate required height for action label
    QFontMetrics fmAction(m_lastActionLabel->font());
    int actionTextWidth = m_lastActionLabel->width() - 10; // Subtract margin
    QRect actionBounds = fmAction.boundingRect(
        QRect(0, 0, actionTextWidth, 0),
        Qt::TextWordWrap,
        m_lastActionLabel->text()
        );

    // Calculate required height for parameters label if visible
    int paramsHeight = 0;
    if (m_lastActionParamsLabel->isVisible()) {
        QFontMetrics fmParams(m_lastActionParamsLabel->font());
        int paramsTextWidth = m_lastActionParamsLabel->width() - 20; // Subtract padding
        QRect paramsBounds = fmParams.boundingRect(
            QRect(0, 0, paramsTextWidth, 0),
            Qt::TextWordWrap,
            m_lastActionParamsLabel->text()
            );
        paramsHeight = paramsBounds.height() + 10; // Add padding
    }

    // Set minimum heights with some padding
    m_lastActionLabel->setMinimumHeight(actionBounds.height() + 8);
    if (m_lastActionParamsLabel->isVisible()) {
        m_lastActionParamsLabel->setMinimumHeight(paramsHeight);
    }

    // Adjust layout if needed
    m_lastActionLabel->updateGeometry();
    m_lastActionParamsLabel->updateGeometry();
    layout()->activate();
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

    // Check if we're reverting to a state with detected lines
    QString lastAction = m_imageProcessor.getLastActionRecord().action;
    if (lastAction == "Detect Lines" || lastAction == "Detect Lines (2D Pointer)") {
        // Re-detect lines based on the current image state
        if (lastAction == "Detect Lines") {
            m_detectedLines = m_imageProcessor.detectDarkLines();

            // Generate detection info summary
            QString detectionInfo = "Detected Lines:\n\n";
            int inObjectCount = 0;
            int isolatedCount = 0;

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

            detectionInfo += QString("\nSummary:\n");
            detectionInfo += QString("Total Lines: %1\n").arg(m_detectedLines.size());
            detectionInfo += QString("In-Object Lines: %1\n").arg(inObjectCount);
            detectionInfo += QString("Isolated Lines: %1\n").arg(isolatedCount);

            // Update the info label
            m_darkLineInfoLabel->setText(detectionInfo);
            updateDarkLineInfoDisplay();

            // Update button states
            if (m_vectorRemoveBtn) m_vectorRemoveBtn->setEnabled(!m_detectedLines.empty());
            if (m_vectorResetBtn) m_vectorResetBtn->setEnabled(!m_detectedLines.empty());
            if (m_pointerDetectBtn) m_pointerDetectBtn->setEnabled(false);
            if (m_pointerRemoveBtn) m_pointerRemoveBtn->setEnabled(false);
            if (m_pointerResetBtn) m_pointerResetBtn->setEnabled(false);

        } else if (lastAction == "Detect Lines (2D Pointer)") {
            // Convert image to ImageData format and detect lines using pointer implementation
            auto imageData = convertToImageData(m_imageProcessor.getFinalImage());
            m_detectedLinesPointer = DarkLinePointerProcessor::detectDarkLines(imageData);

            if (m_detectedLinesPointer && m_detectedLinesPointer->rows > 0) {
                QString detectionInfo = PointerOperations::generateDarkLineInfo(m_detectedLinesPointer);
                m_darkLineInfoLabel->setText(detectionInfo);
                updateDarkLineInfoDisplayPointer();

                // Update button states
                if (m_pointerRemoveBtn) m_pointerRemoveBtn->setEnabled(true);
                if (m_pointerResetBtn) m_pointerResetBtn->setEnabled(true);
                if (m_vectorDetectBtn) m_vectorDetectBtn->setEnabled(false);
                if (m_vectorRemoveBtn) m_vectorRemoveBtn->setEnabled(false);
                if (m_vectorResetBtn) m_vectorResetBtn->setEnabled(false);
            }
        }
    } else {
        // If we're not reverting to a detection state, clear any existing detection info
        resetDetectedLines();
        resetDetectedLinesPointer();
        m_darkLineInfoLabel->clear();
        m_darkLineInfoLabel->setVisible(false);
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

void ControlPanel::setupFileOperations() {
    // Initially disable all buttons except Browse
    connect(this, &ControlPanel::fileLoaded, this, &ControlPanel::enableButtons);

    createGroupBox("File Operations", {
                                       {"Browse", [this]() {
                                            // Create format selection dialog
                                            QDialog formatDialog(this);
                                            formatDialog.setWindowTitle("Select Load Format");
                                            QVBoxLayout* layout = new QVBoxLayout(&formatDialog);

                                            // Create radio buttons for format selection
                                            QRadioButton* loadImageBtn = new QRadioButton("Load Image File (*.png, *.jpg, *.tiff, *.bmp)");
                                            QRadioButton* load2DTextBtn = new QRadioButton("Load Text File - 2D Format");
                                            QRadioButton* load1DTextBtn = new QRadioButton("Load Text File - 1D Format");
                                            loadImageBtn->setChecked(true);

                                            // Add tooltips for clarity
                                            load2DTextBtn->setToolTip("For text files with space-separated values in a 2D grid");
                                            load1DTextBtn->setToolTip("For text files with values in a single column");

                                            layout->addWidget(loadImageBtn);
                                            layout->addWidget(load2DTextBtn);
                                            layout->addWidget(load1DTextBtn);

                                            QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                            layout->addWidget(buttonBox);

                                            connect(buttonBox, &QDialogButtonBox::accepted, &formatDialog, &QDialog::accept);
                                            connect(buttonBox, &QDialogButtonBox::rejected, &formatDialog, &QDialog::reject);

                                            if (formatDialog.exec() == QDialog::Accepted) {
                                                QString filter;
                                                if (loadImageBtn->isChecked()) {
                                                    filter = "Image Files (*.png *.jpg *.jpeg *.tiff *.tif *.bmp)";
                                                } else {
                                                    filter = "Text Files (*.txt)";
                                                }

                                                QString fileName = QFileDialog::getOpenFileName(
                                                    this,
                                                    "Open File",
                                                    "",
                                                    filter);

                                                if (!fileName.isEmpty()) {
                                                    try {
                                                        resetDetectedLines();
                                                        resetDetectedLinesPointer();
                                                        m_darkLineInfoLabel->hide();

                                                        auto startLoad = std::chrono::high_resolution_clock::now();

                                                        if (loadImageBtn->isChecked()) {
                                                            m_imageProcessor.loadImage(fileName.toStdString());
                                                        } else {
                                                            // Use ImageReader for text files
                                                            if (load2DTextBtn->isChecked()) {
                                                                double** matrix = nullptr;
                                                                int rows = 0, cols = 0;
                                                                ImageReader::ReadTextToU2D(fileName.toStdString(), matrix, rows, cols);

                                                                if (matrix && rows > 0 && cols > 0) {
                                                                    std::vector<std::vector<uint16_t>> image(rows, std::vector<uint16_t>(cols));
                                                                    for (int i = 0; i < rows; ++i) {
                                                                        for (int j = 0; j < cols; ++j) {
                                                                            image[i][j] = static_cast<uint16_t>(std::min(65535.0, std::max(0.0, matrix[i][j])));
                                                                        }
                                                                    }

                                                                    // Update the image processor
                                                                    m_imageProcessor.updateAndSaveFinalImage(image);

                                                                    // Clean up the matrix
                                                                    for (int i = 0; i < rows; ++i) {
                                                                        delete[] matrix[i];
                                                                    }
                                                                    delete[] matrix;
                                                                }
                                                            } else {
                                                                uint32_t* matrix = nullptr;
                                                                int rows = 0, cols = 0;
                                                                ImageReader::ReadTextToU1D(fileName.toStdString(), matrix, rows, cols);

                                                                if (matrix && rows > 0 && cols > 0) {
                                                                    std::vector<std::vector<uint16_t>> image(rows, std::vector<uint16_t>(cols));
                                                                    for (int i = 0; i < rows; ++i) {
                                                                        for (int j = 0; j < cols; ++j) {
                                                                            image[i][j] = static_cast<uint16_t>(std::min(65535.0, std::max(0.0, static_cast<double>(matrix[i * cols + j]))));
                                                                        }
                                                                    }

                                                                    // Update the image processor
                                                                    m_imageProcessor.updateAndSaveFinalImage(image);

                                                                    // Clean up the matrix
                                                                    delete[] matrix;
                                                                }
                                                            }
                                                        }

                                                        auto endLoad = std::chrono::high_resolution_clock::now();
                                                        m_fileLoadTime = std::chrono::duration<double, std::milli>(endLoad - startLoad).count();

                                                        // Update histogram
                                                        auto startHist = std::chrono::high_resolution_clock::now();
                                                        if (m_histogram) {
                                                            m_histogram->updateHistogram(m_imageProcessor.getFinalImage());
                                                        }
                                                        auto endHist = std::chrono::high_resolution_clock::now();
                                                        m_histogramTime = std::chrono::duration<double, std::milli>(endHist - startHist).count();

                                                        m_imageLabel->clearSelection();
                                                        updateImageDisplay();

                                                        QString loadType;
                                                        if (loadImageBtn->isChecked()) {
                                                            loadType = "Image";
                                                        } else {
                                                            loadType = load2DTextBtn->isChecked() ? "Text (2D)" : "Text (1D)";
                                                        }

                                                        QFileInfo fileInfo(fileName);
                                                        QString timingInfo = QString("File: %1 (%2)\nProcessing Time - Load: %3 ms, Histogram: %4 ms")
                                                                                 .arg(fileInfo.fileName())
                                                                                 .arg(loadType)
                                                                                 .arg(m_fileLoadTime, 0, 'f', 2)
                                                                                 .arg(m_histogramTime, 0, 'f', 2);

                                                        updateLastAction("Load File", timingInfo);
                                                        emit fileLoaded(true);

                                                    } catch (const std::exception& e) {
                                                        QMessageBox::critical(this, "Error",
                                                                              QString("Failed to load file: %1").arg(e.what()));
                                                        qDebug() << "Error loading file:" << e.what();
                                                    }
                                                }
                                            }
                                        }},
                                          {"Save", [this]() {
                                               QString filePath = QFileDialog::getSaveFileName(
                                                   this,
                                                   "Save Image",
                                                   "",
                                                   "PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;TIFF Files (*.tiff *.tif);;BMP Files (*.bmp)");

                                               if (!filePath.isEmpty()) {
                                                   m_imageProcessor.saveImage(filePath);
                                                   QFileInfo fileInfo(filePath);
                                                   updateLastAction("Save Image", fileInfo.fileName());
                                               }
                                           }},
                                          {"Revert", [this]() {
                                               QString revertedAction = m_imageProcessor.revertImage();
                                               if (!revertedAction.isEmpty()) {
                                                   resetDetectedLines();
                                                   resetDetectedLinesPointer();
                                                   m_imageLabel->clearSelection();
                                                   updateImageDisplay();
                                                   handleRevert();

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
                                                resetDetectedLinesPointer();
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
                                                resetDetectedLinesPointer();
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

    // Create Data Calibration button
    QPushButton* dataCalibrationBtn = new QPushButton("Data Calibration");
    dataCalibrationBtn->setFixedHeight(35);
    dataCalibrationBtn->setToolTip("Calibrate image data using air sample values");
    dataCalibrationBtn->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(dataCalibrationBtn);
    layout->addWidget(dataCalibrationBtn);

    // Create calibration button
    m_calibrationButton = new QPushButton("Calibration");
    m_calibrationButton->setFixedHeight(35);
    m_calibrationButton->setToolTip("Apply calibration using Y-axis and/or X-axis parameters");
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

    // Connect reset button
    connect(m_resetCalibrationButton, &QPushButton::clicked, this, [this]() {
        InterlaceProcessor::resetCalibrationParams();
        m_resetCalibrationButton->setEnabled(false);
        updateCalibrationButtonText();
        QMessageBox::information(this, "Calibration Reset",
                                 "Calibration parameters have been reset. Next calibration will require new parameters.");
    });

    // Connect calibration button
    connect(m_calibrationButton, &QPushButton::clicked, this, [this]() {
        if (checkZoomMode()) return;
        m_darkLineInfoLabel->hide();
        resetDetectedLines();

        // Create dialog for calibration options
        QDialog dialog(this);
        dialog.setWindowTitle("Calibration Options");
        dialog.setMinimumWidth(300);
        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        // Add mode selection
        QGroupBox* modeBox = new QGroupBox("Calibration Mode");
        QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);

        QRadioButton* bothAxesRadio = new QRadioButton("Both Axes");
        QRadioButton* yAxisRadio = new QRadioButton("Y-Axis Only");
        QRadioButton* xAxisRadio = new QRadioButton("X-Axis Only");

        bothAxesRadio->setChecked(true);

        modeLayout->addWidget(bothAxesRadio);
        modeLayout->addWidget(yAxisRadio);
        modeLayout->addWidget(xAxisRadio);
        modeBox->setLayout(modeLayout);
        dialogLayout->addWidget(modeBox);

        // Add explanation labels
        QLabel* explanationLabel = new QLabel(
            "Y-Axis: Uses top lines as reference\n"
            "X-Axis: Uses rightmost lines as reference\n"
            "Both: Applies both calibrations sequentially"
            );
        explanationLabel->setStyleSheet("color: gray; font-size: 10px;");
        dialogLayout->addWidget(explanationLabel);

        // Add buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
            );
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            CalibrationMode mode;
            if (yAxisRadio->isChecked()) {
                mode = CalibrationMode::Y_AXIS_ONLY;
            } else if (xAxisRadio->isChecked()) {
                mode = CalibrationMode::X_AXIS_ONLY;
            } else {
                mode = CalibrationMode::BOTH_AXES;
            }

            // Call processCalibration with 0 values - it will handle parameter input
            processCalibration(0, 0, mode);
            m_resetCalibrationButton->setEnabled(true);
            m_imageLabel->clearSelection();
            updateImageDisplay();
            updateCalibrationButtonText();
        }
    });

    // Connect data calibration button
    connect(dataCalibrationBtn, &QPushButton::clicked, this, [this]() {
        if (checkZoomMode()) return;

        // Create dialog for calibration parameters
        QDialog dialog(this);
        dialog.setWindowTitle("Data Calibration Parameters");
        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        dialogLayout->setSizeConstraint(QLayout::SetFixedSize);

        dialogLayout->setContentsMargins(20, 20, 20, 20);
        dialogLayout->setSpacing(10);

        // Air sample start parameter
        QLabel* startLabel = new QLabel("Air Sample Start:");
        QSpinBox* startSpinBox = new QSpinBox();
        startSpinBox->setRange(0, 1000);
        startSpinBox->setValue(CGImageCalculationVariables.AirSampleStart);

        // Air sample end parameter
        QLabel* endLabel = new QLabel("Air Sample End:");
        QSpinBox* endSpinBox = new QSpinBox();
        endSpinBox->setRange(0, 1000);
        endSpinBox->setValue(CGImageCalculationVariables.AirSampleEnd);

        // Maximum value parameter
        QLabel* maxValueLabel = new QLabel("Maximum Value:");
        QSpinBox* maxValueSpinBox = new QSpinBox();
        maxValueSpinBox->setRange(0, 65535);
        maxValueSpinBox->setValue(CGImageCalculationVariables.PixelMaxValue);

        dialogLayout->addWidget(startLabel);
        dialogLayout->addWidget(startSpinBox);
        dialogLayout->addWidget(endLabel);
        dialogLayout->addWidget(endSpinBox);
        dialogLayout->addWidget(maxValueLabel);
        dialogLayout->addWidget(maxValueSpinBox);

        // Add dialog buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            try {
                const auto& finalImage = m_imageProcessor.getFinalImage();
                int rows = finalImage.size();
                int cols = finalImage[0].size();

                // Convert vector<vector<uint16_t>> to double**
                double** matrix;
                malloc2D(matrix, rows, cols);
                for(int i = 0; i < rows; i++) {
                    for(int j = 0; j < cols; j++) {
                        matrix[i][j] = static_cast<double>(finalImage[i][j]);
                    }
                }

                // Perform calibration using DataCalibration class
                CGData calibratedData = DataCalibration::CalibrateDataMatrix(
                    matrix,
                    rows,
                    cols,
                    maxValueSpinBox->value(),
                    startSpinBox->value(),
                    endSpinBox->value()
                    );

                // Convert calibrated data back to vector format
                std::vector<std::vector<uint16_t>> calibratedImage(calibratedData.Row,
                                                                   std::vector<uint16_t>(calibratedData.Column));

                for(int i = 0; i < calibratedData.Row; i++) {
                    for(int j = 0; j < calibratedData.Column; j++) {
                        calibratedImage[i][j] = static_cast<uint16_t>(
                            std::min(65535.0, std::max(0.0, calibratedData.Data[i][j]))
                            );
                    }
                }

                // Update the image
                m_imageProcessor.updateAndSaveFinalImage(calibratedImage);
                m_imageLabel->clearSelection();
                updateImageDisplay();

                // Free allocated memory
                for(int i = 0; i < rows; i++) {
                    free(matrix[i]);
                }
                free(matrix);

                // Update status
                QString params = QString("Air Sample: %1-%2, Max Value: %3")
                                     .arg(startSpinBox->value())
                                     .arg(endSpinBox->value())
                                     .arg(maxValueSpinBox->value());
                updateLastAction("Data Calibration", params);

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error",
                                      QString("Calibration failed: %1").arg(e.what()));
            }
        }
    });

    // Create Enhanced Interlace button
    QPushButton* enhancedInterlaceBtn = new QPushButton("Enhanced Interlace");
    enhancedInterlaceBtn->setFixedHeight(35);
    enhancedInterlaceBtn->setToolTip("Process image using enhanced interlacing method");
    enhancedInterlaceBtn->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(enhancedInterlaceBtn);
    layout->addWidget(enhancedInterlaceBtn);

    // Connect Enhanced Interlace button
    connect(enhancedInterlaceBtn, &QPushButton::clicked, this, [this]() {
        if (checkZoomMode()) return;
        m_darkLineInfoLabel->hide();
        resetDetectedLines();
        resetDetectedLinesPointer();

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
            "Weighted Average: Average of corresponding pixels with weights\n"
            "Minimum Value: Choose the smaller value between corresponding pixels"
            );
        mergeExplanation->setStyleSheet("color: #666; font-size: 10px;");

        mergeLayout->addWidget(weightedAvgRadio);
        mergeLayout->addWidget(minValueRadio);
        mergeLayout->addWidget(mergeExplanation);
        dialogLayout->addWidget(mergeBox);

        QLabel* weightingInfo = new QLabel(
            "Note: Weighting method will be automatically determined based on image characteristics:\n"
            "• High gradient areas: Edge-preserving weighting\n"
            "• High variance areas: Texture-preserving weighting\n"
            "• Uniform areas: Intensity-based weighting"
            );
        weightingInfo->setStyleSheet(
            "QLabel {"
            "    color: #666;"
            "    font-size: 10px;"
            "    background-color: #f8f8f8;"
            "    border: 1px solid #ddd;"
            "    border-radius: 4px;"
            "    padding: 8px;"
            "    margin: 4px 0px;"
            "}"
            );
        weightingInfo->setWordWrap(true);
        dialogLayout->addWidget(weightingInfo);

        // Add auto-calibration checkbox
        QCheckBox* autoCalibrationCheck = new QCheckBox("Auto-calibrate after interlace and merge");
        autoCalibrationCheck->setChecked(false);  // Default to unchecked
        autoCalibrationCheck->setToolTip("Automatically apply calibration after interlace and merge process");
        dialogLayout->addWidget(autoCalibrationCheck);

        // Add Display Windows checkbox
        QCheckBox* showWindowsCheck = new QCheckBox("Show Intermediate Results Windows");
        showWindowsCheck->setChecked(false);
        showWindowsCheck->setToolTip("Display separate windows showing low energy, high energy, and final merged results");
        dialogLayout->addWidget(showWindowsCheck);

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
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
            );
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            // Update display windows preference
            m_showDisplayWindows = showWindowsCheck->isChecked();

            // Hide any existing windows if they're not needed
            if (!m_showDisplayWindows) {
                if (m_lowEnergyWindow) m_lowEnergyWindow->hide();
                if (m_highEnergyWindow) m_highEnergyWindow->hide();
                if (m_finalWindow) m_finalWindow->hide();
            }

            // Get user selections for section starts
            InterlaceProcessor::StartPoint lowEnergyStart =
                leftLeftLowRadio->isChecked() ?
                    InterlaceProcessor::StartPoint::LEFT_LEFT :
                    InterlaceProcessor::StartPoint::LEFT_RIGHT;

            InterlaceProcessor::StartPoint highEnergyStart =
                rightLeftHighRadio->isChecked() ?
                    InterlaceProcessor::StartPoint::RIGHT_LEFT :
                    InterlaceProcessor::StartPoint::RIGHT_RIGHT;

            // Process the image with automatic weight determination
            auto result = m_imageProcessor.processEnhancedInterlacedSections(
                lowEnergyStart,
                highEnergyStart,
                InterlaceProcessor::MergeParams()  // Use default constructor for automatic selection
                );

            // Show intermediate windows if requested
            if (m_showDisplayWindows) {
                // Create windows if they don't exist
                if (!m_lowEnergyWindow) {
                    m_lowEnergyWindow = std::make_unique<DisplayWindow>(
                        "Low Energy Interlaced", nullptr,
                        QPoint(this->x() + this->width() + 10, this->y())
                        );
                    m_lowEnergyWindow->resize(800, 600);
                }
                if (!m_highEnergyWindow) {
                    m_highEnergyWindow = std::make_unique<DisplayWindow>(
                        "High Energy Interlaced", nullptr,
                        QPoint(this->x() + this->width() + 10, this->y() + 300)
                        );
                    m_highEnergyWindow->resize(800, 600);
                }
                if (!m_finalWindow) {
                    m_finalWindow = std::make_unique<DisplayWindow>(
                        "Final Merged Result", nullptr,
                        QPoint(this->x() + this->width() + 10, this->y() + 600)
                        );
                    m_finalWindow->resize(800, 600);
                }

                // Update and show the windows
                m_lowEnergyWindow->updateImage(result.lowEnergyImage);
                m_highEnergyWindow->updateImage(result.highEnergyImage);
                m_finalWindow->updateImage(result.combinedImage);

                m_lowEnergyWindow->show();
                m_highEnergyWindow->show();
                m_finalWindow->show();
                m_lowEnergyWindow->raise();
                m_highEnergyWindow->raise();
                m_finalWindow->raise();
            }

            // Update main display with the combined image
            m_imageProcessor.updateAndSaveFinalImage(result.combinedImage);

            // Apply auto-calibration if checked
            if (autoCalibrationCheck->isChecked()) {
                processCalibration(0, 0, CalibrationMode::BOTH_AXES);
            }

            m_imageLabel->clearSelection();
            updateImageDisplay();

            // Generate status message
            QString lowEnergyStr = leftLeftLowRadio->isChecked() ? "LeftLeft" : "LeftRight";
            QString highEnergyStr = rightLeftHighRadio->isChecked() ? "RightLeft" : "RightRight";

            // Get used weighting method information
            const auto& [lowImage, highImage] = std::make_pair(result.lowEnergyImage, result.highEnergyImage);
            auto bestParams = InterlaceProcessor::determineBestWeightingMethod(lowImage, highImage);
            QString weightMethodInfo = InterlaceProcessor::getWeightingMethodString(bestParams);

            QString statusMsg = QString("Sections: Low=%1, High=%2\nSelected Method: %3")
                                    .arg(lowEnergyStr)
                                    .arg(highEnergyStr)
                                    .arg(weightMethodInfo);

            if (autoCalibrationCheck->isChecked()) {
                statusMsg += QString("\nAuto-calibrated using %1")
                .arg(InterlaceProcessor::hasCalibrationParams() ?
                         InterlaceProcessor::getCalibrationParamsString() :
                         "default parameters");
            }

            updateLastAction("Enhanced Interlace", statusMsg);
        }
    });

    // Create Interlaced & Merged button
    QPushButton* interlacedMergedBtn = new QPushButton("Interlaced & Merged");
    interlacedMergedBtn->setFixedHeight(35);
    interlacedMergedBtn->setToolTip("Process image using interlaced and merged method");
    interlacedMergedBtn->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(interlacedMergedBtn);
    layout->addWidget(interlacedMergedBtn);

    connect(interlacedMergedBtn, &QPushButton::clicked, this, [this]() {
        if (checkZoomMode()) return;

        // Create dialog for interlace parameters
        QDialog dialog(this);
        dialog.setWindowTitle("Interlaced & Merged Parameters");
        dialog.setMinimumWidth(300);
        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        // Energy Mode selection
        QGroupBox* energyBox = new QGroupBox("Energy Mode");
        QVBoxLayout* energyLayout = new QVBoxLayout(energyBox);
        QRadioButton* singleEnergyRadio = new QRadioButton("Single Energy");
        QRadioButton* dualEnergyRadio = new QRadioButton("Dual Energy");
        energyLayout->addWidget(singleEnergyRadio);
        energyLayout->addWidget(dualEnergyRadio);
        energyBox->setLayout(energyLayout);
        dialogLayout->addWidget(energyBox);

        // Set default based on CGImageDisplayParameters
        if(CGImageDisplayParameters.LaneData.EnergyMode == 1) {
            singleEnergyRadio->setChecked(true);
        } else {
            dualEnergyRadio->setChecked(true);
        }

        // Dual Row Mode selection
        QGroupBox* rowModeBox = new QGroupBox("Dual Row Mode");
        QVBoxLayout* rowModeLayout = new QVBoxLayout(rowModeBox);
        QRadioButton* unfoldRadio = new QRadioButton("Unfold");
        QRadioButton* foldRadio = new QRadioButton("Fold");
        rowModeLayout->addWidget(unfoldRadio);
        rowModeLayout->addWidget(foldRadio);
        rowModeBox->setLayout(rowModeLayout);
        dialogLayout->addWidget(rowModeBox);

        // Set default based on CGImageDisplayParameters
        if(CGImageDisplayParameters.LaneData.DualRowMode == 0) {
            unfoldRadio->setChecked(true);
        } else {
            foldRadio->setChecked(true);
        }

        // Direction selection (only enabled when Fold is selected)
        QGroupBox* directionBox = new QGroupBox("Fold Direction");
        QVBoxLayout* directionLayout = new QVBoxLayout(directionBox);
        QRadioButton* negativeRadio = new QRadioButton("Negative");
        QRadioButton* positiveRadio = new QRadioButton("Positive");
        directionLayout->addWidget(negativeRadio);
        directionLayout->addWidget(positiveRadio);
        directionBox->setLayout(directionLayout);
        directionBox->setEnabled(foldRadio->isChecked());
        dialogLayout->addWidget(directionBox);

        // Set default direction
        if(CGImageDisplayParameters.LaneData.DualRowDirection == 0) {
            negativeRadio->setChecked(true);
        } else {
            positiveRadio->setChecked(true);
        }

        // Connect fold radio to enable/disable direction box
        connect(foldRadio, &QRadioButton::toggled, directionBox, &QGroupBox::setEnabled);

        // High-Low Layout selection
        QGroupBox* layoutBox = new QGroupBox("High-Low Layout");
        QVBoxLayout* layoutBoxLayout = new QVBoxLayout(layoutBox);
        QRadioButton* lowHighRadio = new QRadioButton("Low-High (01)");
        QRadioButton* highLowRadio = new QRadioButton("High-Low (10)");
        layoutBoxLayout->addWidget(lowHighRadio);
        layoutBoxLayout->addWidget(highLowRadio);
        layoutBox->setLayout(layoutBoxLayout);
        dialogLayout->addWidget(layoutBox);

        // Set default based on CGImageDisplayParameters
        if(CGImageDisplayParameters.LaneData.HighLowLayout == 1) {
            lowHighRadio->setChecked(true);
        } else {
            highLowRadio->setChecked(true);
        }

        // Speed settings
        QLabel* speedLabel = new QLabel("Speed (5-15):");
        QDoubleSpinBox* speedSpinBox = new QDoubleSpinBox();
        speedSpinBox->setRange(5.0, 15.0);
        speedSpinBox->setValue(CGImageDisplayParameters.LaneData.DefaultSpeed);
        speedSpinBox->setSingleStep(0.1);
        dialogLayout->addWidget(speedLabel);
        dialogLayout->addWidget(speedSpinBox);

        // Add dialog buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            try {
                qDebug() << "Starting image processing...";

                // Get current image
                const auto& finalImage = m_imageProcessor.getFinalImage();
                int rows = finalImage.size();
                int cols = finalImage[0].size();
                qDebug() << "Image dimensions:" << rows << "x" << cols;

                // Validate dimensions
                if (rows <= 0 || cols <= 0) {
                    QMessageBox::warning(this, "Error", "Invalid image dimensions");
                    return;
                }

                // Check fold mode requirements
                if (foldRadio->isChecked() && (rows % 2 != 0)) {
                    QMessageBox::warning(this, "Warning",
                                         "Image height must be even for fold mode. Please adjust the image dimensions.");
                    return;
                }

                // Allocate memory for input matrix
                qDebug() << "Allocating memory for input matrix...";
                double** inputMatrix = nullptr;
                try {
                    malloc2D(inputMatrix, rows, cols);
                    qDebug() << "Memory allocated successfully";

                    // Copy data to input matrix
                    qDebug() << "Copying data to input matrix...";
                    for (int i = 0; i < rows; i++) {
                        for (int j = 0; j < cols; j++) {
                            inputMatrix[i][j] = static_cast<double>(finalImage[i][j]);
                        }
                    }
                    qDebug() << "Data copied successfully";

                    // Create processor and set parameters
                    qDebug() << "Creating processor and setting parameters...";
                    CGProcessImage processor;
                    processor.SetEnergyMode(singleEnergyRadio->isChecked() ? 1 : 2);
                    processor.SetDualRowMode(unfoldRadio->isChecked() ? 0 : 1);
                    if (foldRadio->isChecked()) {
                        processor.SetDualRowDirection(negativeRadio->isChecked() ? 0 : 1);
                    }
                    processor.SetHighLowLayout(lowHighRadio->isChecked() ? 1 : 2);

                    // Process the image
                    qDebug() << "Processing image...";
                    processor.Process(inputMatrix, rows, cols);
                    qDebug() << "Image processed successfully";

                    // Get processed data
                    double** processedData = nullptr;
                    int processedRows = 0, processedCols = 0;
                    qDebug() << "Getting merged data...";

                    if (processor.GetMergedData(processedData, processedRows, processedCols)) {
                        qDebug() << "Merged data obtained. Dimensions:" << processedRows << "x" << processedCols;

                        // Convert back to vector format
                        qDebug() << "Converting to vector format...";
                        std::vector<std::vector<uint16_t>> processedImage(processedRows,
                                                                          std::vector<uint16_t>(processedCols));

                        for (int i = 0; i < processedRows; i++) {
                            for (int j = 0; j < processedCols; j++) {
                                processedImage[i][j] = static_cast<uint16_t>(
                                    std::min(65535.0, std::max(0.0, processedData[i][j]))
                                    );
                            }
                        }
                        qDebug() << "Conversion completed";

                        // Update display
                        qDebug() << "Updating display...";
                        m_imageProcessor.updateAndSaveFinalImage(processedImage);
                        m_imageLabel->clearSelection();
                        updateImageDisplay();

                        // Update status
                        QString params = QString("Energy Mode: %1, Row Mode: %2%3, Layout: %4, Speed: %5")
                                             .arg(singleEnergyRadio->isChecked() ? "Single" : "Dual")
                                             .arg(unfoldRadio->isChecked() ? "Unfold" : "Fold")
                                             .arg(foldRadio->isChecked()
                                                      ? QString(", Direction: %1").arg(negativeRadio->isChecked() ? "Negative" : "Positive")
                                                      : "")
                                             .arg(lowHighRadio->isChecked() ? "Low-High" : "High-Low")
                                             .arg(speedSpinBox->value());

                        updateLastAction("Interlaced & Merged", params);
                        qDebug() << "Processing completed successfully";
                    } else {
                        qDebug() << "Failed to get merged data";
                        throw std::runtime_error("Failed to get merged data");
                    }

                    // Cleanup input matrix
                    qDebug() << "Cleaning up input matrix...";
                    for (int i = 0; i < rows; i++) {
                        free(inputMatrix[i]);
                    }
                    free(inputMatrix);
                    qDebug() << "Cleanup completed";

                } catch (const std::exception& e) {
                    qDebug() << "Error during processing:" << e.what();
                    // Cleanup if needed
                    if (inputMatrix) {
                        for (int i = 0; i < rows; i++) {
                            free(inputMatrix[i]);
                        }
                        free(inputMatrix);
                    }
                    throw;
                }

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error",
                                      QString("Processing failed: %1").arg(e.what()));
            }
        }
    });

    layout->setSpacing(10);
    layout->addStretch();
    m_scrollLayout->addWidget(groupBox);
    updateCalibrationButtonText();
}

// Remove setupCLAHEOperations() from the constructor and modify setupFilteringOperations()

void ControlPanel::setupFilteringOperations() {
    createGroupBox("Image Enhancement", {
                                         {"CLAHE", [this]() {
                                              if (checkZoomMode()) return;
                                              m_darkLineInfoLabel->hide();
                                              resetDetectedLines();
                                              resetDetectedLinesPointer();

                                              // Get current image dimensions
                                              const auto& currentImage = m_imageProcessor.getFinalImage();
                                              if (currentImage.empty()) {
                                                  QMessageBox::warning(this, "Error", "No image data available for CLAHE processing");
                                                  return;
                                              }

                                              int height = currentImage.size();
                                              int width = currentImage[0].size();

                                              // Create dialog for CLAHE options (same as before)
                                              QDialog dialog(this);
                                              dialog.setWindowTitle("CLAHE Options");
                                              dialog.setMinimumWidth(300);
                                              QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                              // Processing mode selection
                                              QGroupBox* modeBox = new QGroupBox("Processing Mode");
                                              QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);
                                              QRadioButton* gpuRadio = new QRadioButton("GPU Processing");
                                              QRadioButton* cpuRadio = new QRadioButton("CPU Processing");
                                              gpuRadio->setChecked(true);

                                              modeLayout->addWidget(gpuRadio);
                                              modeLayout->addWidget(cpuRadio);
                                              layout->addWidget(modeBox);

                                              // Threshold option
                                              QCheckBox* useThresholdCheck = new QCheckBox("Apply Threshold");
                                              layout->addWidget(useThresholdCheck);

                                              // Threshold value input
                                              QLabel* thresholdLabel = new QLabel("Threshold Value:");
                                              QSpinBox* thresholdSpinBox = new QSpinBox();
                                              thresholdSpinBox->setRange(0, 65535);
                                              thresholdSpinBox->setValue(5000);
                                              thresholdSpinBox->setEnabled(false);
                                              layout->addWidget(thresholdLabel);
                                              layout->addWidget(thresholdSpinBox);

                                              connect(useThresholdCheck, &QCheckBox::toggled, thresholdSpinBox, &QSpinBox::setEnabled);

                                              // CLAHE parameters
                                              QLabel* clipLabel = new QLabel("Clip Limit:");
                                              QDoubleSpinBox* clipSpinBox = new QDoubleSpinBox();
                                              clipSpinBox->setRange(0.1, 1000.0);
                                              clipSpinBox->setValue(2.0);
                                              clipSpinBox->setSingleStep(0.1);
                                              layout->addWidget(clipLabel);
                                              layout->addWidget(clipSpinBox);

                                              QLabel* tileLabel = new QLabel("Tile Size:");
                                              QSpinBox* tileSpinBox = new QSpinBox();
                                              tileSpinBox->setRange(2, 1000);
                                              tileSpinBox->setValue(8);
                                              layout->addWidget(tileLabel);
                                              layout->addWidget(tileSpinBox);

                                              QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                              layout->addWidget(buttonBox);

                                              connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                              connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                              if (dialog.exec() == QDialog::Accepted) {
                                                  try {
                                                      // Allocate input and output buffers
                                                      double** inputBuffer = CLAHEProcessor::allocateImageBuffer(height, width);
                                                      double** outputBuffer = CLAHEProcessor::allocateImageBuffer(height, width);

                                                      // Convert input image to normalized double format
                                                      for (int y = 0; y < height; ++y) {
                                                          for (int x = 0; x < width; ++x) {
                                                              inputBuffer[y][x] = currentImage[y][x] / 65535.0;
                                                          }
                                                      }

                                                      // Get CLAHE parameters
                                                      bool useThreshold = useThresholdCheck->isChecked();
                                                      uint16_t threshold = thresholdSpinBox->value();
                                                      double clipLimit = clipSpinBox->value();
                                                      cv::Size tileSize(tileSpinBox->value(), tileSpinBox->value());

                                                      CLAHEProcessor claheProcessor;
                                                      if (useThreshold) {
                                                          // Apply threshold CLAHE directly to final buffer
                                                          claheProcessor.applyThresholdCLAHE(inputBuffer, height, width, threshold,
                                                                                             clipLimit, tileSize, false);

                                                          // Copy results
                                                          for (int y = 0; y < height; ++y) {
                                                              for (int x = 0; x < width; ++x) {
                                                                  outputBuffer[y][x] = inputBuffer[y][x];
                                                              }
                                                          }
                                                      } else {
                                                          // Apply regular CLAHE
                                                          if (gpuRadio->isChecked()) {
                                                              claheProcessor.applyCLAHE(outputBuffer, inputBuffer, height, width,
                                                                                        clipLimit, tileSize);
                                                              m_lastGpuTime = claheProcessor.getLastPerformanceMetrics().processingTime;
                                                              m_hasGpuClaheTime = true;
                                                              m_gpuTimingLabel->setText(QString("CLAHE Processing Time (GPU): %1 ms").arg(m_lastGpuTime, 0, 'f', 2));
                                                              m_gpuTimingLabel->setVisible(true);
                                                          } else {
                                                              claheProcessor.applyCLAHE_CPU(outputBuffer, inputBuffer, height, width,
                                                                                            clipLimit, tileSize);
                                                              m_lastCpuTime = claheProcessor.getLastPerformanceMetrics().processingTime;
                                                              m_hasCpuClaheTime = true;
                                                              m_cpuTimingLabel->setText(QString("CLAHE Processing Time (CPU): %1 ms").arg(m_lastCpuTime, 0, 'f', 2));
                                                              m_cpuTimingLabel->setVisible(true);
                                                          }
                                                      }

                                                      // Convert result back to uint16_t vector format
                                                      std::vector<std::vector<uint16_t>> resultImage(height, std::vector<uint16_t>(width));
                                                      for (int y = 0; y < height; ++y) {
                                                          for (int x = 0; x < width; ++x) {
                                                              resultImage[y][x] = static_cast<uint16_t>(std::min(65535.0, std::max(0.0, outputBuffer[y][x] * 65535.0)));
                                                          }
                                                      }

                                                      // Cleanup
                                                      CLAHEProcessor::deallocateImageBuffer(inputBuffer, height);
                                                      CLAHEProcessor::deallocateImageBuffer(outputBuffer, height);

                                                      // Update the image
                                                      m_imageProcessor.updateAndSaveFinalImage(resultImage);
                                                      m_imageLabel->clearSelection();
                                                      updateImageDisplay();

                                                      // Update status
                                                      QString methodStr = gpuRadio->isChecked() ? "GPU" : "CPU";
                                                      QString typeStr = useThreshold ? "Threshold" : "Regular";
                                                      QString paramsStr = QString("Method: %1, Type: %2, Clip: %3, Tile: %4%5")
                                                                              .arg(methodStr)
                                                                              .arg(typeStr)
                                                                              .arg(clipLimit, 0, 'f', 2)
                                                                              .arg(tileSpinBox->value())
                                                                              .arg(useThreshold ? QString(", Threshold: %1").arg(threshold) : "");
                                                      updateLastAction(QString("CLAHE"), paramsStr);

                                                  } catch (const cv::Exception& e) {
                                                      QMessageBox::critical(this, "Error",
                                                                            QString("%1 CLAHE processing failed: %2")
                                                                                .arg(gpuRadio->isChecked() ? "GPU" : "CPU")
                                                                                .arg(e.what()));
                                                  }
                                              }
                                          }},
                                            {"Median Filter", [this]() {
                                                 if (checkZoomMode()) return;
                                                 m_darkLineInfoLabel->hide();
                                                 resetDetectedLines();
                                                 resetDetectedLinesPointer();
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

                                            {"Edge Enhancement", [this]() {
                                                 if (checkZoomMode()) return;
                                                 m_darkLineInfoLabel->hide();
                                                 resetDetectedLines();
                                                 resetDetectedLinesPointer();

                                                 auto [strength, ok] = showInputDialog(
                                                     "Edge Enhancement",
                                                     "Enter enhancement strength (0.1-2.0):",
                                                     0.5, 0.1, 2.0);

                                                 if (ok) {
                                                     m_imageProcessor.applyEdgeEnhancement(strength);
                                                     m_imageLabel->clearSelection();
                                                     updateImageDisplay();
                                                     updateLastAction("Edge Enhancement",
                                                                      QString("Strength: %1").arg(strength, 0, 'f', 2));
                                                 }
                                             }},

                                            {"High-Pass Filter", [this]() {
                                                 if (checkZoomMode()) return;
                                                 m_darkLineInfoLabel->hide();
                                                 resetDetectedLines();
                                                 m_imageProcessor.applyHighPassFilter(
                                                     const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage())
                                                     );
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
                                                   resetDetectedLinesPointer();

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
                                                   resetDetectedLinesPointer();
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
                                                   resetDetectedLinesPointer();
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
            if (button->text() != "Browse" && button->text() != "Load Pointer") {
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
    // Create a unified group box
    QGroupBox* groupBox = new QGroupBox("Dark Line Operations");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);

    // Create buttons
    QPushButton* detectBtn = new QPushButton("Detect Dark Lines");
    QPushButton* removeBtn = new QPushButton("Remove Dark Lines");

    // Store pointers for access
    m_vectorDetectBtn = detectBtn;
    m_vectorRemoveBtn = removeBtn;

    // Initially disable remove and reset buttons
    detectBtn->setEnabled(false);
    removeBtn->setEnabled(false);

    // Set fixed height for buttons
    const int buttonHeight = 35;
    detectBtn->setFixedHeight(buttonHeight);
    removeBtn->setFixedHeight(buttonHeight);

    // Add buttons to layout
    layout->addWidget(detectBtn);
    layout->addWidget(removeBtn);

    connect(detectBtn, &QPushButton::clicked, [this, detectBtn, removeBtn]() {
        if (checkZoomMode()) return;

        // Create method selection dialog
        QDialog methodDialog(this);
        methodDialog.setWindowTitle("Select Detection Settings");
        methodDialog.setMinimumWidth(350);
        QVBoxLayout* dialogLayout = new QVBoxLayout(&methodDialog);

        // Create radio buttons for method selection
        QGroupBox* methodBox = new QGroupBox("Detection Method");
        QVBoxLayout* methodLayout = new QVBoxLayout(methodBox);

        QRadioButton* vectorMethodRadio = new QRadioButton("Vector Method");
        QRadioButton* pointerMethodRadio = new QRadioButton("2D Pointer Method");
        vectorMethodRadio->setChecked(true);

        QLabel* vectorExplanation = new QLabel("Vector Method: Traditional approach suitable for most cases");
        QLabel* pointerExplanation = new QLabel("2D Pointer Method: Alternative implementation using pointer arrays");
        vectorExplanation->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
        pointerExplanation->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

        methodLayout->addWidget(vectorMethodRadio);
        methodLayout->addWidget(vectorExplanation);
        methodLayout->addSpacing(5);
        methodLayout->addWidget(pointerMethodRadio);
        methodLayout->addWidget(pointerExplanation);
        methodBox->setLayout(methodLayout);
        dialogLayout->addWidget(methodBox);

        // Add line type selection group - for pointer method only
        QGroupBox* lineTypeBox = new QGroupBox("Line Types to Detect");
        QVBoxLayout* lineTypeLayout = new QVBoxLayout(lineTypeBox);

        QCheckBox* verticalCheck = new QCheckBox("Detect Vertical Lines");
        QCheckBox* horizontalCheck = new QCheckBox("Detect Horizontal Lines");

        // Add explanatory text for pointer method
        QLabel* lineTypeExplanation = new QLabel(
            "Select line types to detect.\n"
            "Leave unchecked to use comprehensive detection."
            );
        lineTypeExplanation->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

        lineTypeLayout->addWidget(verticalCheck);
        lineTypeLayout->addWidget(horizontalCheck);
        lineTypeLayout->addWidget(lineTypeExplanation);
        lineTypeBox->setLayout(lineTypeLayout);
        dialogLayout->addWidget(lineTypeBox);

        // Initially hide line type selection for vector method
        lineTypeBox->setVisible(false);

        // Connect radio buttons to show/hide line type selection
        connect(vectorMethodRadio, &QRadioButton::toggled, lineTypeBox, &QGroupBox::setHidden);
        connect(pointerMethodRadio, &QRadioButton::toggled, lineTypeBox, &QGroupBox::setVisible);

        // Add OK/Cancel buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &methodDialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &methodDialog, &QDialog::reject);

        if (methodDialog.exec() == QDialog::Accepted) {
            // Clear previous detections
            resetDetectedLines();
            resetDetectedLinesPointer();
            m_darkLineInfoLabel->hide();

            if (pointerMethodRadio->isChecked()) {
                try {
                    auto imageData = convertToImageData(m_imageProcessor.getFinalImage());
                    m_imageProcessor.saveCurrentState();

                    bool detectVertical = verticalCheck->isChecked();
                    bool detectHorizontal = horizontalCheck->isChecked();
                    DarkLineArray* outLines = nullptr;
                    bool success = false;

                    // If neither checkbox is checked, detect all lines
                    if (!detectVertical && !detectHorizontal) {
                        success = DarkLinePointerProcessor::checkforBoth(imageData, outLines);
                    } else {
                        // Use specific detection based on checkbox selection
                        if (detectVertical && detectHorizontal) {
                            success = DarkLinePointerProcessor::checkforBoth(imageData, outLines);
                        } else if (detectVertical) {
                            success = DarkLinePointerProcessor::checkforVertical(imageData, outLines);
                        } else if (detectHorizontal) {
                            success = DarkLinePointerProcessor::checkforHorizontal(imageData, outLines);
                        }
                    }

                    if (success && outLines && outLines->rows > 0) {
                        // Clean up any existing detection results
                        if (m_detectedLinesPointer) {
                            DarkLinePointerProcessor::destroyDarkLineArray(m_detectedLinesPointer);
                        }

                        m_detectedLinesPointer = outLines;
                        QString detectionInfo = PointerOperations::generateDarkLineInfo(m_detectedLinesPointer);
                        m_darkLineInfoLabel->setText(detectionInfo);
                        updateDarkLineInfoDisplayPointer();

                        // Update button states
                        if (m_pointerRemoveBtn) m_pointerRemoveBtn->setEnabled(true);
                        if (m_pointerResetBtn) m_pointerResetBtn->setEnabled(true);
                        if (m_vectorDetectBtn) m_vectorDetectBtn->setEnabled(false);
                        if (m_vectorRemoveBtn) m_vectorRemoveBtn->setEnabled(false);
                        if (m_vectorResetBtn) m_vectorResetBtn->setEnabled(false);
                    } else {
                        if (outLines) {
                            DarkLinePointerProcessor::destroyDarkLineArray(outLines);
                        }
                        QMessageBox::information(this, "Detection Result",
                                                 "No dark lines detected using pointer method.");
                        return;
                    }
                } catch (const std::exception& e) {
                    QMessageBox::critical(this, "Error",
                                          QString("Error in line detection: %1").arg(e.what()));
                    return;
                }
            } else {
                // Vector method implementation - always detect all lines
                m_imageProcessor.saveCurrentState();
                m_detectedLines = m_imageProcessor.detectDarkLines();

                // Generate detection info for vector method
                QString detectionInfo = "Detected Lines (Vector Method):\n\n";
                int inObjectCount = 0;
                int isolatedCount = 0;

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

                    if (line.inObject) inObjectCount++;
                    else isolatedCount++;
                }

                detectionInfo += QString("\nSummary:\n");
                detectionInfo += QString("Total Lines: %1\n").arg(m_detectedLines.size());
                detectionInfo += QString("In-Object Lines: %1\n").arg(inObjectCount);
                detectionInfo += QString("Isolated Lines: %1\n").arg(isolatedCount);

                m_darkLineInfoLabel->setText(detectionInfo);
                updateDarkLineInfoDisplay();
            }

            // Enable remove and reset buttons
            removeBtn->setEnabled(true);
            updateImageDisplay();

            // Update last action with detection type info
            QString methodStr = vectorMethodRadio->isChecked() ? "Vector Method" : "2D Pointer Method";
            QString typeStr;
            if (pointerMethodRadio->isChecked()) {
                bool detectVertical = verticalCheck->isChecked();
                bool detectHorizontal = horizontalCheck->isChecked();
                if (!detectVertical && !detectHorizontal) {
                    typeStr = "All Lines";
                } else {
                    QStringList types;
                    if (detectVertical) types << "Vertical";
                    if (detectHorizontal) types << "Horizontal";
                    typeStr = types.join(" and ");
                }
            } else {
                typeStr = "All Lines";  // Vector method always detects all lines
            }
            updateLastAction("Detect Lines", QString("%1 - %2").arg(methodStr).arg(typeStr));
        }
    });

    // Connect remove button
    connect(removeBtn, &QPushButton::clicked, [this, removeBtn]() {
        if (checkZoomMode()) return;

        if (m_detectedLines.empty() && (!m_detectedLinesPointer || m_detectedLinesPointer->rows == 0)) {
            QMessageBox::information(this, "Remove Lines", "Please detect lines first.");
            return;
        }

        // Determine which method was used for detection
        bool vectorMethodActive = !m_detectedLines.empty();
        bool pointerMethodActive = m_detectedLinesPointer && m_detectedLinesPointer->rows > 0;

        if (vectorMethodActive) {
            // Create removal dialog
            QDialog dialog(this);
            dialog.setWindowTitle("Remove Lines (Vector Method)");
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

            // Line selection list
            QGroupBox* lineSelectionBox = new QGroupBox("Select Lines to Process");
            QVBoxLayout* selectionLayout = new QVBoxLayout();
            QListWidget* lineList = new QListWidget();
            lineList->setSelectionMode(QAbstractItemView::ExtendedSelection);

            // Add lines to the list
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
                    lineList->clearSelection();
                    if (lineList->count() > 0) {
                        lineList->item(0)->setSelected(true);
                    }
                }
            });

            lineList->setMinimumHeight(200);
            lineList->setMaximumHeight(300);
            selectionLayout->addWidget(lineList);
            lineSelectionBox->setLayout(selectionLayout);
            layout->addWidget(lineSelectionBox);

            // Add select all button for neighbor values mode
            QPushButton* selectAllButton = new QPushButton("Select All Lines");
            selectAllButton->setVisible(!stitchRadio->isChecked());
            connect(selectAllButton, &QPushButton::clicked, lineList, &QListWidget::selectAll);
            layout->addWidget(selectAllButton);

            // Connect radio buttons to update UI
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

                if (removeInObject) {
                    ImageProcessor::LineRemovalMethod method = stitchRadio->isChecked() ?
                                                                   ImageProcessor::LineRemovalMethod::DIRECT_STITCH :
                                                                   ImageProcessor::LineRemovalMethod::NEIGHBOR_VALUES;

                    methodStr = stitchRadio->isChecked() ? "Direct Stitch" : "Neighbor Values";

                    // Process selected lines
                    auto selectedItems = lineList->selectedItems();
                    if (selectedItems.isEmpty()) {
                        QMessageBox::warning(this, "Warning", "Please select at least one line for processing.");
                        return;
                    }

                    std::vector<ImageProcessor::DarkLine> selectedLines;
                    for (QListWidgetItem* item : selectedItems) {
                        int index = item->data(Qt::UserRole).toInt();
                        selectedLines.push_back(m_detectedLines[index]);
                    }

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

                // Update detected lines after removal
                m_detectedLines = m_imageProcessor.detectDarkLines();

                // Generate updated detection info
                QString detectionInfo = "Detected Lines (Vector Method):\n\n";
                int inObjectCount = 0;
                int isolatedCount = 0;

                for (size_t i = 0; i < m_detectedLines.size(); ++i) {
                    const auto& line = m_detectedLines[i];
                    QString coordinates = line.isVertical ?
                                              QString("(%1,0)").arg(line.x) :
                                              QString("(0,%1)").arg(line.y);

                    detectionInfo += QString("Line %1: %2 with width %3 pixels (%4)\n")
                                         .arg(i + 1)
                                         .arg(coordinates)
                                         .arg(line.width)
                                         .arg(line.inObject ? "In Object" : "Isolated");

                    if (line.inObject) inObjectCount++;
                    else isolatedCount++;
                }

                detectionInfo += QString("\nSummary:\n");
                detectionInfo += QString("Total Lines: %1\n").arg(m_detectedLines.size());
                detectionInfo += QString("In-Object Lines: %1\n").arg(inObjectCount);
                detectionInfo += QString("Isolated Lines: %1\n").arg(isolatedCount);

                // Update the info label and display
                m_darkLineInfoLabel->setText(detectionInfo);
                updateDarkLineInfoDisplay();

                // Update UI elements
                updateImageDisplay();
                updateLastAction("Remove Lines", QString("%1 - %2").arg(typeStr).arg(methodStr));

                if (m_detectedLines.empty()) {
                    removeBtn->setEnabled(false);
                    m_darkLineInfoLabel->clear();
                    m_darkLineInfoLabel->setVisible(false);
                    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
                        qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
                        );
                    if (darkLineScrollArea) {
                        darkLineScrollArea->setVisible(false);
                    }
                }
            }
        } else if (pointerMethodActive) {
            // Handle pointer method removal
            PointerOperations::handleRemoveLinesDialog(this);
        }
    });

    // Add buttons to the list for general enable/disable management
    m_allButtons.push_back(detectBtn);
    m_allButtons.push_back(removeBtn);

    // Set tooltips
    detectBtn->setToolTip("Detect dark lines using vector or pointer method");
    removeBtn->setToolTip("Remove detected dark lines");

    // Add the group box to the main layout
    m_scrollLayout->addWidget(groupBox);
}

void ControlPanel::resetDetectedLines() {
    m_detectedLines.clear();
    // Remove the incorrect line: m_detectedLinesPointer->clear();
    m_imageProcessor.clearDetectedLines();

    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
        qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
        );

    m_darkLineInfoLabel->clear();
    m_darkLineInfoLabel->setVisible(false);
    darkLineScrollArea->setVisible(false);

    updateImageDisplay();
}

void ControlPanel::resetDetectedLinesPointer() {
    if (m_detectedLinesPointer) {
        DarkLinePointerProcessor::destroyDarkLineArray(m_detectedLinesPointer);
        m_detectedLinesPointer = nullptr;
    }

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

void ControlPanel::updateDarkLineInfoDisplayPointer() {
    if (!m_darkLineInfoLabel || !m_darkLineInfoLabel->text().isEmpty()) {
        QFontMetrics fm(m_darkLineInfoLabel->font());
        QString text = m_darkLineInfoLabel->text();
        int textHeight = fm.lineSpacing() * text.count('\n') + 40;
        int preferredHeight = qMin(textHeight, 300);
        preferredHeight = qMax(preferredHeight, 30);

        QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
            qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
            );

        if (darkLineScrollArea) {
            darkLineScrollArea->setFixedHeight(preferredHeight);
            m_darkLineInfoLabel->setVisible(true);
            darkLineScrollArea->setVisible(true);
        }
    }
}

ImageData ControlPanel::convertToImageData(const std::vector<std::vector<uint16_t>>& image) {
    ImageData imageData;

    if (image.empty() || image[0].empty()) {
        throw std::runtime_error("Empty image provided");
    }

    imageData.rows = static_cast<int>(image.size());
    imageData.cols = static_cast<int>(image[0].size());
    imageData.data = new double*[imageData.rows];

    for (int i = 0; i < imageData.rows; i++) {
        imageData.data[i] = new double[imageData.cols];
        for (int j = 0; j < imageData.cols; j++) {
            imageData.data[i][j] = static_cast<double>(image[i][j]);
        }
    }

    return imageData;
}

std::vector<std::vector<uint16_t>> ControlPanel::convertFromImageData(const ImageData& imageData) {
    if (!imageData.data || imageData.rows <= 0 || imageData.cols <= 0) {
        throw std::runtime_error("Invalid image data");
    }

    std::vector<std::vector<uint16_t>> result(imageData.rows);
    for (int i = 0; i < imageData.rows; i++) {
        result[i].resize(imageData.cols);
        for (int j = 0; j < imageData.cols; j++) {
            double value = imageData.data[i][j];
            result[i][j] = static_cast<uint16_t>(std::clamp(std::round(value), 0.0, 65535.0));
        }
    }

    return result;
}

void ControlPanel::validateImageData(const ImageData& imageData) {
    if (!imageData.data || imageData.rows <= 0 || imageData.cols <= 0) {
        throw std::runtime_error("Invalid image data");
    }
}

std::vector<std::vector<uint16_t>> ControlPanel::createVectorFromImageData(const ImageData& imageData) {
    std::vector<std::vector<uint16_t>> result(imageData.rows);

    for (int i = 0; i < imageData.rows; ++i) {
        result[i].resize(imageData.cols);
        convertRowToUint16(imageData.data[i], result[i], imageData.cols);
    }

    return result;
}

void ControlPanel::convertRowToUint16(const double* sourceRow, std::vector<uint16_t>& destRow, int cols) {
    for (int j = 0; j < cols; ++j) {
        double value = sourceRow[j];
        destRow[j] = static_cast<uint16_t>(std::clamp(std::round(value), 0.0, 65535.0));
    }
}

void ControlPanel::updateImageDisplay(double** doubleImage, int height, int width) {
    // 创建临时的 vector<vector<uint16_t>> 并调用主显示函数
    std::vector<std::vector<uint16_t>> tempImage(height, std::vector<uint16_t>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tempImage[y][x] = static_cast<uint16_t>(std::clamp(doubleImage[y][x] * 65535.0, 0.0, 65535.0));
        }
    }
    updateImageDisplay(&tempImage);
}

void ControlPanel::updateImageDisplay(const std::vector<std::vector<uint16_t>>* vectorImage) {
    const auto& finalImage = vectorImage ? *vectorImage : m_imageProcessor.getFinalImage();
    if (!finalImage.empty()) {
        // 1. Update image size label
        int height = static_cast<int>(finalImage.size());
        int width = static_cast<int>(finalImage[0].size());
        m_imageSizeLabel->setText(QString("Image Size: %1 x %2").arg(width).arg(height));

        try {
            // 2. Create QImage
            QImage image(width, height, QImage::Format_Grayscale16);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    uint16_t pixelValue = finalImage[y][x];
                    image.setPixel(x, y, qRgb(pixelValue >> 8, pixelValue >> 8, pixelValue >> 8));
                }
            }

            // 3. Convert to pixmap and handle zoom
            QPixmap pixmap = QPixmap::fromImage(image);
            const auto& zoomManager = m_imageProcessor.getZoomManager();
            float zoomLevel = zoomManager.getZoomLevel();

            // 4. Apply zoom if active
            if (zoomManager.isZoomModeActive() && zoomLevel != 1.0f) {
                QSize zoomedSize = zoomManager.getZoomedSize(pixmap.size());
                pixmap = pixmap.scaled(zoomedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            }

            // 5. Handle visualization of detected lines
            bool hasDetectedLines = !m_detectedLines.empty();
            bool hasDetectedLinesPointer = m_detectedLinesPointer && m_detectedLinesPointer->rows > 0;

            if (hasDetectedLines || hasDetectedLinesPointer) {
                QPixmap drawPixmap = pixmap;
                QPainter painter(&drawPixmap);
                painter.setRenderHint(QPainter::Antialiasing);

                float penWidth = zoomManager.isZoomModeActive() ?
                                     std::max(1.0f, 2.0f * zoomLevel) : 2.0f;

                // Configure font for labels
                QFont labelFont = painter.font();
                float scaledSize = std::min(11.0f * zoomLevel, 24.0f);
                labelFont.setPixelSize(static_cast<int>(std::max(11.0f, scaledSize)));
                labelFont.setFamily("Arial");
                painter.setFont(labelFont);

                int lineCount = 0;

                // Create a set to track which pointer lines have been matched
                std::vector<std::pair<int, int>> matchedPointerLines;

                // First draw lines from vector implementation
                if (hasDetectedLines) {
                    for (size_t i = 0; i < m_detectedLines.size(); ++i) {
                        const auto& vectorLine = m_detectedLines[i];
                        bool foundMatch = false;

                        // Check if this line matches any pointer line
                        if (hasDetectedLinesPointer) {
                            for (int pi = 0; pi < m_detectedLinesPointer->rows; ++pi) {
                                for (int pj = 0; pj < m_detectedLinesPointer->cols; ++pj) {
                                    const auto& pointerLine = m_detectedLinesPointer->lines[pi][pj];

                                    // Check if lines are identical
                                    if (vectorLine.isVertical == pointerLine.isVertical &&
                                        vectorLine.width == pointerLine.width &&
                                        ((vectorLine.isVertical && vectorLine.x == pointerLine.x) ||
                                         (!vectorLine.isVertical && vectorLine.y == pointerLine.y))) {

                                        matchedPointerLines.push_back({pi, pj});
                                        foundMatch = true;
                                        break;
                                    }
                                }
                                if (foundMatch) break;
                            }
                        }

                        // Draw line if it wasn't matched or is unique to vector implementation
                        QRect lineRect;
                        if (vectorLine.isVertical) {
                            int adjustedX = static_cast<int>(vectorLine.x * zoomLevel);
                            int adjustedWidth = std::max(1, static_cast<int>(vectorLine.width * zoomLevel));
                            lineRect = QRect(adjustedX, 0, adjustedWidth, height * zoomLevel);
                        } else {
                            int adjustedY = static_cast<int>(vectorLine.y * zoomLevel);
                            int adjustedHeight = std::max(1, static_cast<int>(vectorLine.width * zoomLevel));
                            lineRect = QRect(0, adjustedY, width * zoomLevel, adjustedHeight);
                        }

                        QColor lineColor = vectorLine.inObject ?
                                               QColor(0, 0, 255, 128) :
                                               QColor(255, 0, 0, 128);
                        painter.setPen(QPen(lineColor, penWidth, Qt::SolidLine));
                        painter.setBrush(QBrush(lineColor, Qt::Dense4Pattern));
                        painter.drawRect(lineRect);
                        drawLineLabelWithCount(painter, vectorLine, lineCount++, zoomLevel, drawPixmap.size());
                    }
                }

                // Now draw any unmatched pointer lines
                if (hasDetectedLinesPointer) {
                    for (int i = 0; i < m_detectedLinesPointer->rows; ++i) {
                        for (int j = 0; j < m_detectedLinesPointer->cols; ++j) {
                            // Skip if this line was already matched and drawn
                            if (std::find(matchedPointerLines.begin(), matchedPointerLines.end(),
                                          std::make_pair(i, j)) != matchedPointerLines.end()) {
                                continue;
                            }

                            const auto& line = m_detectedLinesPointer->lines[i][j];
                            QRect lineRect;
                            if (line.isVertical) {
                                int adjustedX = static_cast<int>(line.x * zoomLevel);
                                int adjustedWidth = std::max(1, static_cast<int>(line.width * zoomLevel));
                                int adjustedStartY = static_cast<int>(line.startY * zoomLevel);
                                int adjustedEndY = static_cast<int>(line.endY * zoomLevel);
                                lineRect = QRect(adjustedX, adjustedStartY, adjustedWidth, adjustedEndY - adjustedStartY);
                            } else {
                                int adjustedY = static_cast<int>(line.y * zoomLevel);
                                int adjustedHeight = std::max(1, static_cast<int>(line.width * zoomLevel));
                                int adjustedStartX = static_cast<int>(line.startX * zoomLevel);
                                int adjustedEndX = static_cast<int>(line.endX * zoomLevel);
                                lineRect = QRect(adjustedStartX, adjustedY, adjustedEndX - adjustedStartX, adjustedHeight);
                            }

                            QColor lineColor = line.inObject ?
                                                   QColor(0, 0, 255, 128) :
                                                   QColor(255, 0, 0, 128);
                            painter.setPen(QPen(lineColor, penWidth, Qt::SolidLine));
                            painter.setBrush(QBrush(lineColor, Qt::Dense4Pattern));
                            painter.drawRect(lineRect);
                            drawLineLabelWithCountPointer(painter, line, lineCount++, zoomLevel, drawPixmap.size());
                        }
                    }
                }

                painter.end();
                m_imageLabel->setPixmap(drawPixmap);
                m_imageLabel->setFixedSize(drawPixmap.size());
            } else {
                m_imageLabel->setPixmap(pixmap);
                m_imageLabel->setFixedSize(pixmap.size());
            }

            // 6. Update histogram if visible
            if (m_histogram && m_histogram->isVisible()) {
                m_histogram->updateHistogram(finalImage);
            }

        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to update display: %1").arg(e.what()));
        }
    } else {
        // Handle empty image case
        m_imageSizeLabel->setText("Image Size: No image loaded");
        m_imageLabel->clear();
        if (m_histogram && m_histogram->isVisible()) {
            m_histogram->setVisible(false);
        }
    }
}

// For vector implementation
void ControlPanel::drawLineLabelWithCount(QPainter& painter,
                                          const ImageProcessor::DarkLine& line,
                                          int count,
                                          float zoomLevel,
                                          const QSize& imageSize) {
    // Prepare label text
    QString labelText = QString("Line %1 - %2")
                            .arg(count + 1)
                            .arg(line.inObject ? "In Object" : "Isolated");

    // Calculate label position
    int labelMargin = static_cast<int>(10 * zoomLevel);
    int labelSpacing = static_cast<int>(std::max(30.0f, 40.0f * zoomLevel));

    int labelX, labelY;
    if (line.isVertical) {
        labelX = static_cast<int>(line.x * zoomLevel + labelMargin);
        labelY = static_cast<int>(30 * zoomLevel + count * labelSpacing);
    } else {
        labelX = labelMargin;
        labelY = static_cast<int>(line.y * zoomLevel + labelMargin);
        if (count > 0) {
            labelX += static_cast<int>(count * labelSpacing * 1.5); // Adjust spacing for horizontal lines
        }
    }

    drawLabelCommon(painter, labelText, labelX, labelY, labelMargin, zoomLevel, imageSize);
}

// For pointer implementation
// 实现也使用全局的 DarkLine
void ControlPanel::drawLineLabelWithCountPointer(QPainter& painter,
                                                 const DarkLine& line,
                                                 int count,
                                                 float zoomLevel,
                                                 const QSize& imageSize) {
    // Prepare label text
    QString labelText = QString("Line %1 - %2")
                            .arg(count + 1)
                            .arg(line.inObject ? "In Object" : "Isolated");

    // Calculate label position
    int labelMargin = static_cast<int>(10 * zoomLevel);
    int labelSpacing = static_cast<int>(std::max(30.0f, 40.0f * zoomLevel));

    int labelX, labelY;
    if (line.isVertical) {
        labelX = static_cast<int>(line.x * zoomLevel + labelMargin);
        labelY = static_cast<int>(30 * zoomLevel + count * labelSpacing);
    } else {
        labelX = labelMargin;
        labelY = static_cast<int>(line.y * zoomLevel + labelMargin);
        if (count > 0) {
            labelX += static_cast<int>(count * labelSpacing * 1.5); // Adjust spacing for horizontal lines
        }
    }

    drawLabelCommon(painter, labelText, labelX, labelY, labelMargin, zoomLevel, imageSize);
}


void ControlPanel::drawLabelCommon(QPainter& painter,
                                   const QString& labelText,
                                   int labelX,
                                   int labelY,
                                   int labelMargin,
                                   float zoomLevel,
                                   const QSize& imageSize) {
    // Calculate label background rectangle
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(labelText);
    textRect.moveTopLeft(QPoint(labelX, labelY - textRect.height()));
    textRect.adjust(-5, -2, 5, 2);

    // Ensure label stays within image bounds
    if (textRect.right() > imageSize.width()) {
        textRect.moveLeft(imageSize.width() - textRect.width() - labelMargin);
    }
    if (textRect.bottom() > imageSize.height()) {
        textRect.moveTop(imageSize.height() - textRect.height() - labelMargin);
    }
    if (textRect.left() < labelMargin) {
        textRect.moveLeft(labelMargin);
    }
    if (textRect.top() < labelMargin) {
        textRect.moveTop(labelMargin);
    }

    // Draw label with enhanced visibility
    QColor bgColor = QColor(255, 255, 255, 245);
    QColor borderColor = QColor(0, 0, 0, 160);

    painter.setPen(QPen(borderColor, std::max(1.0f, 1.5f * zoomLevel)));
    painter.setBrush(QBrush(bgColor));
    painter.drawRect(textRect);

    painter.setPen(Qt::black);
    painter.drawText(textRect, Qt::AlignCenter | Qt::TextDontClip, labelText);
}

void ControlPanel::clearAllDetectionResults() {
    resetDetectedLines();
    resetDetectedLinesPointer();
    m_darkLineInfoLabel->clear();
    m_darkLineInfoLabel->setVisible(false);

    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
        qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
        );
    if (darkLineScrollArea) {
        darkLineScrollArea->setVisible(false);
    }

    updateImageDisplay();
}

void ControlPanel::resetAllParameters() {
    // Reset calibration parameters
    InterlaceProcessor::resetCalibrationParams();
    updateCalibrationButtonText();

    // Reset timing information
    m_lastGpuTime = -1;
    m_lastCpuTime = -1;
    m_hasCpuClaheTime = false;
    m_hasGpuClaheTime = false;
    m_gpuTimingLabel->clear();
    m_gpuTimingLabel->setVisible(false);
    m_cpuTimingLabel->clear();
    m_cpuTimingLabel->setVisible(false);

    // Reset histogram
    if (m_histogram) {
        m_histogram->setVisible(false);
    }

    // Reset zoom settings and button states
    auto& zoomManager = m_imageProcessor.getZoomManager();
    zoomManager.resetZoom();
    zoomManager.toggleZoomMode(false);
    zoomManager.toggleFixedZoom(false);

    // Reset zoom controls group visibility
    if (m_zoomControlsGroup) {
        m_zoomControlsGroup->hide();
    }

    // Reset fix zoom button
    if (m_fixZoomButton) {
        m_fixZoomButton->setChecked(false);
        m_fixZoomButton->setText("Fix Zoom");
        m_fixZoomButton->setEnabled(false);
    }

    // Reset main zoom button state and appearance
    if (m_zoomButton) {
        m_zoomButton->setChecked(false);
        m_zoomButton->setText("Activate Zoom");
        m_zoomButton->setProperty("state", "");
        m_zoomButton->setEnabled(false);
        m_zoomButton->style()->unpolish(m_zoomButton);
        m_zoomButton->style()->polish(m_zoomButton);
    }

    // Reset zoom control buttons
    if (m_zoomInButton) m_zoomInButton->setEnabled(false);
    if (m_zoomOutButton) m_zoomOutButton->setEnabled(false);
    if (m_resetZoomButton) m_resetZoomButton->setEnabled(false);

    // Reset display windows
    if (m_lowEnergyWindow) {
        m_lowEnergyWindow->hide();
        m_lowEnergyWindow->clear();
    }
    if (m_highEnergyWindow) {
        m_highEnergyWindow->hide();
        m_highEnergyWindow->clear();
    }
    if (m_finalWindow) {
        m_finalWindow->hide();
        m_finalWindow->clear();
    }
    m_showDisplayWindows = false;

    // Reset labels
    m_imageSizeLabel->setText("Image Size: No image loaded");
    m_pixelInfoLabel->setText("Pixel Info: ");
    m_lastActionLabel->setText("Last Action: None");
    m_lastActionParamsLabel->clear();
    m_lastActionParamsLabel->setVisible(false);

    // Reset detection info
    clearAllDetectionResults();

    // Re-enable all buttons except Browse
    for (QPushButton* button : m_allButtons) {
        if (button && button->text() != "Browse") {
            button->setEnabled(false);
        }
    }
}

void ControlPanel::setupResetOperations() {
    createGroupBox("Reset Operations", {
                                           {"Reset Detection", [this]() {
                                                QMessageBox::StandardButton reply = QMessageBox::warning(
                                                    this,
                                                    "Reset Detection",
                                                    "Are you sure you want to clear all detected lines?\nThis action cannot be undone.",
                                                    QMessageBox::Yes | QMessageBox::No
                                                    );

                                                if (reply == QMessageBox::Yes) {
                                                    clearAllDetectionResults();
                                                    updateLastAction("Reset Detection");
                                                    QMessageBox::information(this, "Success", "All detection results have been cleared.");
                                                }
                                            }},
                                           {"Reset All", [this]() {
                                                QMessageBox::StandardButton reply = QMessageBox::warning(
                                                    this,
                                                    "Reset All",
                                                    "Are you sure you want to reset everything to initial state?\n"
                                                    "This will reset the image to its state right after loading and clear all parameters.\n"
                                                    "This action cannot be undone.",
                                                    QMessageBox::Yes | QMessageBox::No
                                                    );

                                                if (reply == QMessageBox::Yes) {
                                                    m_imageProcessor.resetToOriginal();
                                                    resetAllParameters();
                                                    m_imageLabel->clearSelection();
                                                    updateImageDisplay();
                                                    updateLastAction("Reset All");
                                                    QMessageBox::information(this, "Success",
                                                                             "Image has been reset to initial state and all parameters have been cleared.");
                                                }
                                            }},
                                           {"Clear Image", [this]() {
                                                QMessageBox::StandardButton reply = QMessageBox::warning(
                                                    this,
                                                    "Clear Image",
                                                    "Are you sure you want to clear the loaded image?\n"
                                                    "This will remove the image and reset all parameters.\n"
                                                    "This action cannot be undone.",
                                                    QMessageBox::Yes | QMessageBox::No
                                                    );

                                                if (reply == QMessageBox::Yes) {
                                                    m_imageProcessor.clearImage();
                                                    resetAllParameters();
                                                    m_imageLabel->clearSelection();
                                                    m_imageLabel->clear();
                                                    emit fileLoaded(false);
                                                    updateLastAction("Clear Image");
                                                    QMessageBox::information(this, "Success",
                                                                             "Image has been cleared and all parameters have been reset.");
                                                }
                                            }}
                                       });
}

void ControlPanel::setupCombinedAdjustments() {
    createGroupBox("Image Adjustments", {
                                            {"Gamma Adjustment", [this]() {
                                                 if (checkZoomMode()) return;

                                                 // Create dialog
                                                 QDialog dialog(this);
                                                 dialog.setWindowTitle("Gamma Adjustment");
                                                 dialog.setMinimumWidth(300);

                                                 // Create layout
                                                 QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                                 // Create mode selection
                                                 QGroupBox* modeBox = new QGroupBox("Adjustment Mode");
                                                 QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);
                                                 QRadioButton* overallRadio = new QRadioButton("Overall Adjustment");
                                                 QRadioButton* regionalRadio = new QRadioButton("Regional Adjustment");
                                                 overallRadio->setChecked(true);

                                                 modeLayout->addWidget(overallRadio);
                                                 modeLayout->addWidget(regionalRadio);
                                                 layout->addWidget(modeBox);

                                                 // Create gamma value input
                                                 QLabel* gammaLabel = new QLabel("Gamma Value:");
                                                 QDoubleSpinBox* gammaSpinBox = new QDoubleSpinBox();
                                                 gammaSpinBox->setRange(0.1, 10.0);
                                                 gammaSpinBox->setValue(1.0);
                                                 gammaSpinBox->setSingleStep(0.1);

                                                 layout->addWidget(gammaLabel);
                                                 layout->addWidget(gammaSpinBox);

                                                 // Create region selection info label
                                                 QLabel* regionLabel = new QLabel("For regional adjustment, select the region in the image first.");
                                                 regionLabel->setWordWrap(true);
                                                 regionLabel->setVisible(false);
                                                 layout->addWidget(regionLabel);

                                                 // Connect radio buttons to show/hide region label
                                                 connect(regionalRadio, &QRadioButton::toggled, regionLabel, &QLabel::setVisible);

                                                 // Add buttons
                                                 QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                     QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                                 layout->addWidget(buttonBox);

                                                 connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                 connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                 if (dialog.exec() == QDialog::Accepted) {
                                                     float gamma = gammaSpinBox->value();

                                                     if (regionalRadio->isChecked()) {
                                                         if (!m_imageLabel->isRegionSelected()) {
                                                             QMessageBox::information(this, "Region Selection",
                                                                                      "Please select a region first, then try again.");
                                                             return;
                                                         }
                                                         QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                         m_imageProcessor.adjustGammaForSelectedRegion(gamma, selectedRegion);
                                                         m_imageLabel->clearSelection();
                                                         updateImageDisplay();
                                                         updateLastAction("Regional Gamma", QString::number(gamma, 'f', 2));
                                                     } else {
                                                         m_imageProcessor.adjustGammaOverall(gamma);
                                                         updateImageDisplay();
                                                         updateLastAction("Overall Gamma", QString::number(gamma, 'f', 2));
                                                     }
                                                 }
                                             }},

                                            {"Contrast Adjustment", [this]() {
                                                 if (checkZoomMode()) return;

                                                 QDialog dialog(this);
                                                 dialog.setWindowTitle("Contrast Adjustment");
                                                 dialog.setMinimumWidth(300);

                                                 QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                                 // Mode selection
                                                 QGroupBox* modeBox = new QGroupBox("Adjustment Mode");
                                                 QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);
                                                 QRadioButton* overallRadio = new QRadioButton("Overall Adjustment");
                                                 QRadioButton* regionalRadio = new QRadioButton("Regional Adjustment");
                                                 overallRadio->setChecked(true);

                                                 modeLayout->addWidget(overallRadio);
                                                 modeLayout->addWidget(regionalRadio);
                                                 layout->addWidget(modeBox);

                                                 // Contrast factor input
                                                 QLabel* contrastLabel = new QLabel("Contrast Factor:");
                                                 QDoubleSpinBox* contrastSpinBox = new QDoubleSpinBox();
                                                 contrastSpinBox->setRange(0.1, 10.0);
                                                 contrastSpinBox->setValue(1.0);
                                                 contrastSpinBox->setSingleStep(0.1);

                                                 layout->addWidget(contrastLabel);
                                                 layout->addWidget(contrastSpinBox);

                                                 // Region selection info
                                                 QLabel* regionLabel = new QLabel("For regional adjustment, select the region in the image first.");
                                                 regionLabel->setWordWrap(true);
                                                 regionLabel->setVisible(false);
                                                 layout->addWidget(regionLabel);

                                                 connect(regionalRadio, &QRadioButton::toggled, regionLabel, &QLabel::setVisible);

                                                 QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                     QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                                 layout->addWidget(buttonBox);

                                                 connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                 connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                 if (dialog.exec() == QDialog::Accepted) {
                                                     float contrast = contrastSpinBox->value();

                                                     if (regionalRadio->isChecked()) {
                                                         if (!m_imageLabel->isRegionSelected()) {
                                                             QMessageBox::information(this, "Region Selection",
                                                                                      "Please select a region first, then try again.");
                                                             return;
                                                         }
                                                         QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                         m_imageProcessor.applyContrastToRegion(contrast, selectedRegion);
                                                         m_imageLabel->clearSelection();
                                                         updateImageDisplay();
                                                         updateLastAction("Regional Contrast", QString::number(contrast, 'f', 2));
                                                     } else {
                                                         m_imageProcessor.adjustContrast(contrast);
                                                         updateImageDisplay();
                                                         updateLastAction("Overall Contrast", QString::number(contrast, 'f', 2));
                                                     }
                                                 }
                                             }},

                                            {"Sharpen Adjustment", [this]() {
                                                 if (checkZoomMode()) return;

                                                 QDialog dialog(this);
                                                 dialog.setWindowTitle("Sharpen Adjustment");
                                                 dialog.setMinimumWidth(300);

                                                 QVBoxLayout* layout = new QVBoxLayout(&dialog);

                                                 // Mode selection
                                                 QGroupBox* modeBox = new QGroupBox("Adjustment Mode");
                                                 QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);
                                                 QRadioButton* overallRadio = new QRadioButton("Overall Adjustment");
                                                 QRadioButton* regionalRadio = new QRadioButton("Regional Adjustment");
                                                 overallRadio->setChecked(true);

                                                 modeLayout->addWidget(overallRadio);
                                                 modeLayout->addWidget(regionalRadio);
                                                 layout->addWidget(modeBox);

                                                 // Sharpen strength input
                                                 QLabel* sharpenLabel = new QLabel("Sharpen Strength:");
                                                 QDoubleSpinBox* sharpenSpinBox = new QDoubleSpinBox();
                                                 sharpenSpinBox->setRange(0.1, 10.0);
                                                 sharpenSpinBox->setValue(1.0);
                                                 sharpenSpinBox->setSingleStep(0.1);

                                                 layout->addWidget(sharpenLabel);
                                                 layout->addWidget(sharpenSpinBox);

                                                 // Region selection info
                                                 QLabel* regionLabel = new QLabel("For regional adjustment, select the region in the image first.");
                                                 regionLabel->setWordWrap(true);
                                                 regionLabel->setVisible(false);
                                                 layout->addWidget(regionLabel);

                                                 connect(regionalRadio, &QRadioButton::toggled, regionLabel, &QLabel::setVisible);

                                                 QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                     QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                                 layout->addWidget(buttonBox);

                                                 connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                 connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                 if (dialog.exec() == QDialog::Accepted) {
                                                     float strength = sharpenSpinBox->value();

                                                     if (regionalRadio->isChecked()) {
                                                         if (!m_imageLabel->isRegionSelected()) {
                                                             QMessageBox::information(this, "Region Selection",
                                                                                      "Please select a region first, then try again.");
                                                             return;
                                                         }
                                                         QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                                         m_imageProcessor.applySharpenToRegion(strength, selectedRegion);
                                                         m_imageLabel->clearSelection();
                                                         updateImageDisplay();
                                                         updateLastAction("Regional Sharpen", QString::number(strength, 'f', 2));
                                                     } else {
                                                         m_imageProcessor.sharpenImage(strength);
                                                         updateImageDisplay();
                                                         updateLastAction("Overall Sharpen", QString::number(strength, 'f', 2));
                                                     }
                                                 }
                                             }}
                                        });
}

void ControlPanel::processCalibration(int linesToProcessY, int linesToProcessX, CalibrationMode mode) {
    // 检查是否已经有存储的参数
    bool hasStoredParams = InterlaceProcessor::hasCalibrationParams();
    int storedY = hasStoredParams ? InterlaceProcessor::getStoredYParam() : 0;
    int storedX = hasStoredParams ? InterlaceProcessor::getStoredXParam() : 0;

    // 根据模式获取新的参数值
    int newY = linesToProcessY;
    int newX = linesToProcessX;
    QString actionDescription;

    // 如果已经有存储的参数
    if (hasStoredParams) {
        switch (mode) {
        case CalibrationMode::Y_AXIS_ONLY:
            if (storedY > 0) {
                newY = storedY;
                actionDescription = QString("Y-Axis (Used stored: %1 lines)").arg(newY);
            } else {
                // Remove limits from input dialog
                auto result = showInputDialog("Calibration", "Enter lines to process for Y-axis:", 10);
                if (!result.second) return;
                newY = result.first;
                actionDescription = QString("Y-Axis (New: %1 lines)").arg(newY);
            }
            newX = storedX;
            break;

        case CalibrationMode::X_AXIS_ONLY:
            if (storedX > 0) {
                newX = storedX;
                actionDescription = QString("X-Axis (Used stored: %1 lines)").arg(newX);
            } else {
                // Remove limits from input dialog
                auto result = showInputDialog("Calibration", "Enter lines to process for X-axis:", 10);
                if (!result.second) return;
                newX = result.first;
                actionDescription = QString("X-Axis (New: %1 lines)").arg(newX);
            }
            newY = storedY;
            break;

        case CalibrationMode::BOTH_AXES:
            QString yDescription, xDescription;
            if (storedY == 0) {
                // Remove limits from input dialog for Y-axis
                auto resultY = showInputDialog("Calibration", "Enter lines to process for Y-axis:", 10);
                if (!resultY.second) return;
                newY = resultY.first;
                yDescription = QString("Y:%1 (New)").arg(newY);
            } else {
                newY = storedY;
                yDescription = QString("Y:%1 (Stored)").arg(newY);
            }

            if (storedX == 0) {
                // Remove limits from input dialog for X-axis
                auto resultX = showInputDialog("Calibration", "Enter lines to process for X-axis:", 10);
                if (!resultX.second) return;
                newX = resultX.first;
                xDescription = QString("X:%1 (New)").arg(newX);
            } else {
                newX = storedX;
                xDescription = QString("X:%1 (Stored)").arg(newX);
            }
            actionDescription = QString("Both Axes (%1, %2)").arg(yDescription).arg(xDescription);
            break;
        }
    } else {
        switch (mode) {
        case CalibrationMode::Y_AXIS_ONLY:
        {
            // Remove limits from input dialog
            auto result = showInputDialog("Calibration", "Enter lines to process for Y-axis:", 10);
            if (!result.second) return;
            newY = result.first;
            newX = 0;
            actionDescription = QString("Y-Axis (New: %1 lines)").arg(newY);
        }
        break;

        case CalibrationMode::X_AXIS_ONLY:
        {
            // Remove limits from input dialog
            auto result = showInputDialog("Calibration", "Enter lines to process for X-axis:", 10);
            if (!result.second) return;
            newX = result.first;
            newY = 0;
            actionDescription = QString("X-Axis (New: %1 lines)").arg(newX);
        }
        break;

        case CalibrationMode::BOTH_AXES:
        {
            // Remove limits from input dialogs
            auto resultY = showInputDialog("Calibration", "Enter lines to process for Y-axis:", 10);
            if (!resultY.second) return;
            newY = resultY.first;

            auto resultX = showInputDialog("Calibration", "Enter lines to process for X-axis:", 10);
            if (!resultX.second) return;
            newX = resultX.first;
            actionDescription = QString("Both Axes (Y:%1, X:%2) (New)").arg(newY).arg(newX);
        }
        break;
        }
    }

    // Save current state before processing
    m_imageProcessor.saveCurrentState();

    // 执行校准
    auto& image = const_cast<std::vector<std::vector<uint16_t>>&>(m_imageProcessor.getFinalImage());

    // Y轴处理
    if (mode == CalibrationMode::Y_AXIS_ONLY || mode == CalibrationMode::BOTH_AXES) {
        if (newY > 0) {
            int height = image.size();
            int width = image[0].size();
            std::vector<float> referenceYMean(width, 0.0f);

            for (int y = 0; y < std::min(newY, height); ++y) {
                for (int x = 0; x < width; ++x) {
                    referenceYMean[x] += image[y][x];
                }
            }

            for (int x = 0; x < width; ++x) {
                referenceYMean[x] /= newY;
            }

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (referenceYMean[x] > 0) {
                        float normalizedValue = static_cast<float>(image[y][x]) / referenceYMean[x];
                        image[y][x] = static_cast<uint16_t>(std::min(normalizedValue * 65535.0f, 65535.0f));
                    }
                }
            }
        }
    }

    // X轴处理
    if (mode == CalibrationMode::X_AXIS_ONLY || mode == CalibrationMode::BOTH_AXES) {
        if (newX > 0) {
            int height = image.size();
            int width = image[0].size();
            std::vector<float> referenceXMean(height, 0.0f);

            for (int y = 0; y < height; ++y) {
                for (int x = width - newX; x < width; ++x) {
                    referenceXMean[y] += image[y][x];
                }
                referenceXMean[y] /= newX;
            }

            for (int x = 0; x < width; ++x) {
                for (int y = 0; y < height; ++y) {
                    if (referenceXMean[y] > 0) {
                        float normalizedValue = static_cast<float>(image[y][x]) / referenceXMean[y];
                        image[y][x] = static_cast<uint16_t>(std::min(normalizedValue * 65535.0f, 65535.0f));
                    }
                }
            }
        }
    }

    // 更新存储的校准参数
    InterlaceProcessor::setCalibrationParams(newY, newX);

    // 更新Last Action和参数显示
    QString paramString = QString("Mode: %1\nParameters: Y=%2, X=%3")
                              .arg(actionDescription)
                              .arg(newY)
                              .arg(newX);

    updateLastAction("Calibration", paramString);
}

std::pair<int, bool> ControlPanel::showInputDialog(const QString& title,
                                                   const QString& label,
                                                   int defaultValue) {
    bool ok;
    int value = QInputDialog::getInt(this, title, label, defaultValue,
                                     INT_MIN, INT_MAX, 1, &ok);
    return std::make_pair(value, ok);
}
