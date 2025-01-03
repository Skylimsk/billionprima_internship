#include "control_panel.h"
#include "pointer_operations.h"
#include "adjustments.h"
#include "darkline_pointer.h"
#include "graph_3d_processor.h"
#include "interlace.h"
#include <QPushButton>
#include <QGroupBox>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QPainter>
#include <set>

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
    setupZoomControls();
    setupBasicOperations();
    setupGraph();
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
    try {
        resetDetectedLinesPointer();

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
    } catch (...) {
    }
}

void ControlPanel::enableButtons(bool enable) {
    qDebug() << "\n=== Checking Button States ===";
    qDebug() << "File loaded state:" << enable;

    // Check zoom mode states
    auto& zoomManager = m_imageProcessor.getZoomManager();
    bool zoomModeActive = zoomManager.isZoomModeActive();
    bool zoomFixed = zoomManager.isZoomFixed();

    qDebug() << "Zoom mode active:" << zoomModeActive;
    qDebug() << "Zoom fixed:" << zoomFixed;

    // Process each button based on its type and current application state
    for (QPushButton* button : m_allButtons) {
        if (!button) continue;

        QString buttonText = button->text();
        bool shouldEnable = false;

        if (zoomModeActive && !zoomFixed) {
            // When zoom is active but not fixed, only enable zoom-related buttons
            if (buttonText == "Deactivate Zoom" ||
                    buttonText == "Zoom In" ||
                    buttonText == "Zoom Out" ||
                    buttonText == "Reset Zoom" ||
                    buttonText == "Fix Zoom") {
                shouldEnable = enable;
            } else {
                shouldEnable = false;
            }
        } else {
            // For all other states
            if (buttonText == "Browse") {
                // Browse is always disabled when file is loaded
                shouldEnable = !enable;
            }
            else if (buttonText == "Activate Zoom" || buttonText == "Deactivate Zoom") {
                // Main zoom button is enabled when file is loaded
                shouldEnable = enable;
            }
            else if (buttonText == "Zoom In" || buttonText == "Zoom Out" || buttonText == "Reset Zoom") {
                // Zoom control buttons are only enabled in unfixed zoom mode
                shouldEnable = enable && zoomModeActive && !zoomFixed;
            }
            else if (buttonText == "Fix Zoom" || buttonText == "Unfix Zoom") {
                // Fix/Unfix button is enabled in zoom mode
                shouldEnable = enable && zoomModeActive;
            }
            else {
                // All other buttons
                if (zoomModeActive) {
                    if (!zoomFixed) {
                        // When in unfixed zoom mode, disable all non-zoom buttons
                        shouldEnable = false;
                    } else {
                        // When zoom is fixed, enable all non-zoom buttons (except Browse)
                        shouldEnable = enable;
                    }
                } else {
                    // When zoom is not active, enable all non-zoom buttons (except Browse)
                    shouldEnable = enable;
                }
            }
        }

        button->setEnabled(shouldEnable);

        // Update button styles for zoom button
        if (buttonText == "Activate Zoom" || buttonText == "Deactivate Zoom") {
            if (!shouldEnable) {
                button->setStyleSheet(
                            "QPushButton {"
                            "    background-color: #f3f4f6;"
                            "    color: #999999;"  // Grayed out text
                            "    border: 1px solid #e5e7eb;"
                            "    border-radius: 4px;"
                            "    padding: 5px;"
                            "}"
                            );
            } else {
                button->setStyleSheet(
                            "QPushButton {"
                            "    background-color: #ffffff;"
                            "    color: #000000;"
                            "    border: 1px solid #e5e7eb;"
                            "    border-radius: 4px;"
                            "    padding: 5px;"
                            "}"
                            "QPushButton[state=\"deactivate-unfix\"] {"
                            "    background-color: #3b82f6;"
                            "    color: #ffffff;"
                            "    border: none;"
                            "}"
                            "QPushButton[state=\"deactivate-fix\"] {"
                            "    background-color: #ef4444;"
                            "    color: #ffffff;"
                            "    border: none;"
                            "}"
                            );
            }
        }
    }

    // Update histogram buttons if present
    if (m_histogram) {
        bool histogramEnabled = enable && (!zoomModeActive || zoomFixed);
        for (QPushButton* button : m_allButtons) {
            if (button && (button->text() == "Histogram" || button->text() == "Toggle CLAHE View")) {
                button->setEnabled(histogramEnabled);
                qDebug() << "Histogram button state:" << histogramEnabled;
            }
        }
    }

    // Special handling for zoom group visibility
    if (m_zoomControlsGroup) {
        m_zoomControlsGroup->setVisible(zoomModeActive);
    }

    // Force immediate update of the UI
    if (m_zoomButton) {
        m_zoomButton->style()->unpolish(m_zoomButton);
        m_zoomButton->style()->polish(m_zoomButton);
    }

    qDebug() << "=== Button State Update Complete ===\n";
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

    m_fileTypeLabel = new QLabel("File Type: None");
    m_fileTypeLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    infoLayout->addWidget(m_fileTypeLabel);

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
        } else if (action == "Undo") {
            prefix = "";
        } else if (action == "Zoom In" || action == "Zoom Out" || action == "Reset Zoom") {
            prefix = "Zoom Factor: ";
        } else if (action == "Zoom Mode") {
            prefix = "Status: ";
        } else if (action == "CLAHE"){
            prefix = "Type: ";
        }
        else {
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

void ControlPanel::updatePixelInfo(const QPoint& pos)
{
    const auto& finalImage = m_imageProcessor.getFinalImage();
    int height = m_imageProcessor.getFinalImageHeight();
    int width = m_imageProcessor.getFinalImageWidth();

    if (finalImage && height > 0 && width > 0) {
        int x = std::clamp(pos.x(), 0, width - 1);
        int y = std::clamp(pos.y(), 0, height - 1);
        double pixelValue = finalImage[y][x];
        QString info = QString("Pixel Info: X: %1, Y: %2, Value: %3")
                .arg(x).arg(y)
                .arg(static_cast<int>(pixelValue));
        m_pixelInfoLabel->setText(info);
    } else {
        m_pixelInfoLabel->setText("Pixel Info: No image loaded");
    }
}

void ControlPanel::setupZoomControls() {
    // Create a subgroup for zoom controls that will be added to Basic Operations
    m_zoomControlsGroup = new QGroupBox();
    m_zoomControlsGroup->setStyleSheet(
                "QGroupBox {"
                "    border: none;"
                "    margin-left: 15px;"  // Indent from main button
                "    margin-top: 5px;"    // Space from zoom button
                "    margin-bottom: 10px;" // Space before next control
                "    padding: 0px;"
                "}"
                );

    QVBoxLayout* zoomLayout = new QVBoxLayout(m_zoomControlsGroup);
    zoomLayout->setSpacing(5);
    zoomLayout->setContentsMargins(0, 0, 0, 0);

    // Create zoom control buttons
    QPushButton* zoomInBtn = new QPushButton("Zoom In");
    QPushButton* zoomOutBtn = new QPushButton("Zoom Out");
    m_fixZoomButton = new QPushButton("Fix Zoom");
    QPushButton* resetZoomBtn = new QPushButton("Reset Zoom");

    // Store pointers for later use
    m_zoomInButton = zoomInBtn;
    m_zoomOutButton = zoomOutBtn;
    m_resetZoomButton = resetZoomBtn;
    m_fixZoomButton->setCheckable(true);

    // Set fixed height and style for buttons
    const int buttonHeight = 35;
    const QString buttonStyle =
            "QPushButton {"
            "    background-color: #f3f4f6;"
            "    border: 1px solid #e5e7eb;"
            "    border-radius: 4px;"
            "    padding: 5px;"
            "}";

    // Update m_fixZoomButton style
    m_fixZoomButton->setStyleSheet(
                "QPushButton {"
                "    background-color: #ffffff;"
                "    color: #000000;"
                "    border: 1px solid #e5e7eb;"
                "    border-radius: 4px;"
                "    padding: 5px;"
                "}"
                "QPushButton:checked {"
                "    background-color: #3b82f6;"
                "    color: #ffffff;"
                "    border: none;"
                "}"
                );

    for (QPushButton* btn : {zoomInBtn, zoomOutBtn, m_fixZoomButton, resetZoomBtn}) {
        btn->setFixedHeight(buttonHeight);
        btn->setStyleSheet(buttonStyle);
        btn->setEnabled(false);
    }

    // Add to m_allButtons for automatic enabling/disabling
    m_allButtons.push_back(zoomInBtn);
    m_allButtons.push_back(zoomOutBtn);
    m_allButtons.push_back(resetZoomBtn);
    m_allButtons.push_back(m_fixZoomButton);

    // Add tooltips
    zoomInBtn->setToolTip("Increase zoom level by 20%");
    zoomOutBtn->setToolTip("Decrease zoom level by 20%");
    resetZoomBtn->setToolTip("Reset to original size (100%)");
    m_fixZoomButton->setToolTip("Fix current zoom level for image processing");

    // Add buttons to layout
    zoomLayout->addWidget(zoomInBtn);
    zoomLayout->addWidget(zoomOutBtn);
    zoomLayout->addWidget(m_fixZoomButton);
    zoomLayout->addWidget(resetZoomBtn);

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

        // Update fix zoom button text
        m_fixZoomButton->setText(checked ? "Unfix Zoom" : "Fix Zoom");

        // Update main zoom button state if it's in deactivate mode
        if (m_zoomButton->text() == "Deactivate Zoom") {
            m_zoomButton->setProperty("state", checked ? "deactivate-fix" : "deactivate-unfix");
            m_zoomButton->style()->unpolish(m_zoomButton);
            m_zoomButton->style()->polish(m_zoomButton);
        }

        // Handle button states
        for (QPushButton* button : m_allButtons) {
            if (button) {
                QString buttonText = button->text();
                if (!checked) {  // When unfixed
                    // Only enable zoom-related buttons
                    button->setEnabled(buttonText == "Deactivate Zoom" ||
                                       buttonText == "Zoom In" ||
                                       buttonText == "Zoom Out" ||
                                       buttonText == "Reset Zoom" ||
                                       buttonText == "Fix Zoom");
                } else {  // When fixed
                    if (buttonText == "Browse") {
                        button->setEnabled(false);  // Browse always disabled when image loaded
                    } else if (buttonText == "Zoom In" ||
                               buttonText == "Zoom Out" ||
                               buttonText == "Reset Zoom") {
                        button->setEnabled(false);  // Disable zoom controls when fixed
                    } else {
                        button->setEnabled(true);   // Enable all other buttons
                    }
                }
            }
        }

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

    if (active) {
        float currentZoomLevel = zoomManager.getZoomLevel();
        if (currentZoomLevel != 1.0f) {
            QMessageBox* warningBox = new QMessageBox(this);
            warningBox->setIcon(QMessageBox::Warning);
            warningBox->setWindowTitle("Zoom Level Warning");
            warningBox->setText(QString("Current zoom level is %1x.\nDo you want to proceed with this zoom level or reset to 1x?")
                                .arg(currentZoomLevel, 0, 'f', 2));
            QPushButton* proceedButton = warningBox->addButton("Proceed", QMessageBox::AcceptRole);
            QPushButton* resetButton = warningBox->addButton("Reset", QMessageBox::RejectRole);

            int result = warningBox->exec();

            // If user closes dialog or hits Escape (rejected), cancel zoom activation
            if (result == QMessageBox::Rejected) {
                delete warningBox;
                m_zoomButton->setChecked(false);  // Reset button state
                return;  // Return early - no zoom processing
            }

            if (warningBox->clickedButton() == resetButton) {
                zoomManager.resetZoom();
                currentZoomLevel = 1.0f;
            }

            delete warningBox;
        }

        // Continue with activation
        zoomManager.toggleZoomMode(true);
        m_zoomButton->setText("Deactivate Zoom");
        m_zoomButton->setProperty("state",
                                  m_fixZoomButton->isChecked() ? "deactivate-fix" : "deactivate-unfix");

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

        // Disable all buttons except zoom-related buttons
        for (QPushButton* button : m_allButtons) {
            if (button) {
                QString buttonText = button->text();
                bool isZoomRelated = (buttonText == "Deactivate Zoom" ||
                                      buttonText == "Zoom In" ||
                                      buttonText == "Zoom Out" ||
                                      buttonText == "Reset Zoom" ||
                                      buttonText == "Fix Zoom" ||
                                      buttonText == "Unfix Zoom");
                button->setEnabled(isZoomRelated);
            }
        }

    } else {
        // When deactivating zoom
        float currentZoom = zoomManager.getZoomLevel();
        zoomManager.toggleZoomMode(false);

        if (m_zoomControlsGroup) {
            m_zoomControlsGroup->hide();
            m_scrollLayout->removeWidget(m_zoomControlsGroup);
        }

        // Re-enable all buttons except Browse when deactivating zoom
        for (QPushButton* button : m_allButtons) {
            if (button) {
                QString buttonText = button->text();
                if (buttonText == "Browse") {
                    button->setEnabled(false);  // Keep Browse disabled
                } else {
                    button->setEnabled(true);
                }
            }
        }

        updateImageDisplay();
        updateLastAction("Zoom Mode", QString("Deactivated (Maintained %1x)").arg(currentZoom, 0, 'f', 2));

        m_zoomButton->setChecked(false);
        m_zoomButton->setText("Activate Zoom");
        m_zoomButton->setProperty("state", "");
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
                            // Create file dialog with all supported formats
                            QString filter = "All Supported Files (*.png *.jpg *.jpeg *.tiff *.tif *.bmp *.txt);;"
                            "Image Files (*.png *.jpg *.jpeg *.tiff *.tif *.bmp);;"
                            "Text Files (*.txt)";

                            QString fileName = QFileDialog::getOpenFileName(
                            this,
                            "Open File",
                            "",
                            filter);

                            if (fileName.isEmpty()) {
                                return;
                            }

                            try {
                                resetDetectedLinesPointer();
                                m_darkLineInfoLabel->hide();

                                QString extension = QFileInfo(fileName).suffix().toLower();
                                bool isTextFile = (extension == "txt");

                                if (isTextFile) {
                                    // Show dialog for text file format selection
                                    QDialog formatDialog(this);
                                    formatDialog.setWindowTitle("Text File Format");
                                    formatDialog.setMinimumWidth(300);

                                    QVBoxLayout* layout = new QVBoxLayout(&formatDialog);

                                    // Format selection
                                    QGroupBox* formatGroup = new QGroupBox("Select Format");
                                    QVBoxLayout* formatLayout = new QVBoxLayout(formatGroup);

                                    QRadioButton* format2DBtn = new QRadioButton("2D Format (Grid Layout)");
                                    QRadioButton* format1DBtn = new QRadioButton("1D Format (Single Column)");
                                    format2DBtn->setChecked(true);

                                    formatLayout->addWidget(format2DBtn);
                                    formatLayout->addWidget(format1DBtn);

                                    // Add format descriptions
                                    QLabel* formatInfo = new QLabel(
                                    "Format descriptions:\n"
                                    "• 2D Format: Text file with values arranged in a grid\n"
                                    "• 1D Format: Text file with values in a single column"
                                    );
                                    formatInfo->setStyleSheet("color: #666666; margin-top: 10px;");
                                    formatLayout->addWidget(formatInfo);

                                    formatGroup->setLayout(formatLayout);
                                    layout->addWidget(formatGroup);

                                    // Add OK/Cancel buttons
                                    QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                    layout->addWidget(buttonBox);

                                    connect(buttonBox, &QDialogButtonBox::accepted, &formatDialog, &QDialog::accept);
                                    connect(buttonBox, &QDialogButtonBox::rejected, &formatDialog, &QDialog::reject);

                                    if (formatDialog.exec() == QDialog::Accepted) {
                                        auto startLoad = std::chrono::high_resolution_clock::now();

                                        if (format2DBtn->isChecked()) {
                                            // Handle 2D text format
                                            double** matrix = nullptr;
                                            int rows = 0, cols = 0;
                                            ImageReader::ReadTextToU2D(fileName.toStdString(), matrix, rows, cols);

                                            if (matrix && rows > 0 && cols > 0) {
                                                double** finalMatrix = new double*[rows];
                                                for (int i = 0; i < rows; ++i) {
                                                    finalMatrix[i] = new double[cols];
                                                    for (int j = 0; j < cols; ++j) {
                                                        finalMatrix[i][j] = std::min(65535.0, std::max(0.0, matrix[i][j]));
                                                    }
                                                }

                                                m_imageProcessor.setOriginalImage(finalMatrix, rows, cols);
                                                m_imageProcessor.updateAndSaveFinalImage(finalMatrix, rows, cols);
                                                m_fileTypeLabel->setText("File Type: 2D Text");

                                                // Clean up matrices
                                                for (int i = 0; i < rows; ++i) {
                                                    delete[] matrix[i];
                                                    delete[] finalMatrix[i];
                                                }
                                                delete[] matrix;
                                                delete[] finalMatrix;
                                            }
                                        } else {
                                            // Handle 1D text format
                                            uint32_t* matrix = nullptr;
                                            int rows = 0, cols = 0;
                                            ImageReader::ReadTextToU1D(fileName.toStdString(), matrix, rows, cols);

                                            if (matrix && rows > 0 && cols > 0) {
                                                double** finalMatrix = new double*[rows];
                                                for (int i = 0; i < rows; ++i) {
                                                    finalMatrix[i] = new double[cols];
                                                    for (int j = 0; j < cols; ++j) {
                                                        finalMatrix[i][j] = std::min(65535.0, std::max(0.0,
                                                        static_cast<double>(matrix[i * cols + j])));
                                                    }
                                                }

                                                m_imageProcessor.setOriginalImage(finalMatrix, rows, cols);
                                                m_imageProcessor.updateAndSaveFinalImage(finalMatrix, rows, cols);
                                                m_fileTypeLabel->setText("File Type: 1D Text");

                                                // Clean up
                                                delete[] matrix;
                                                for (int i = 0; i < rows; ++i) {
                                                    delete[] finalMatrix[i];
                                                }
                                                delete[] finalMatrix;
                                            }
                                        }

                                        auto endLoad = std::chrono::high_resolution_clock::now();
                                        m_fileLoadTime = std::chrono::duration<double, std::milli>(endLoad - startLoad).count();

                                    }
                                } else {
                                    // Handle image file
                                    auto startLoad = std::chrono::high_resolution_clock::now();
                                    m_imageProcessor.loadImage(fileName.toStdString());
                                    m_fileTypeLabel->setText("File Type: Image");
                                    auto endLoad = std::chrono::high_resolution_clock::now();
                                    m_fileLoadTime = std::chrono::duration<double, std::milli>(endLoad - startLoad).count();
                                }

                                // Update display and status
                                m_imageSizeLabel->setText(QString("Image Size: %1 x %2")
                                .arg(m_imageProcessor.getFinalImageWidth())
                                .arg(m_imageProcessor.getFinalImageHeight()));

                                auto startHist = std::chrono::high_resolution_clock::now();
                                if (m_histogram) {
                                    m_histogram->updateHistogram(m_imageProcessor.getFinalImage(),
                                    m_imageProcessor.getFinalImageHeight(),
                                    m_imageProcessor.getFinalImageWidth());
                                }
                                auto endHist = std::chrono::high_resolution_clock::now();
                                m_histogramTime = std::chrono::duration<double, std::milli>(endHist - startHist).count();

                                m_imageLabel->clearSelection();
                                updateImageDisplay();

                                QFileInfo fileInfo(fileName);
                                QString timingInfo = QString("File: %1\nProcessing Time - Load: %2 ms, Histogram: %3 ms")
                                .arg(fileInfo.fileName())
                                .arg(m_fileLoadTime, 0, 'f', 2)
                                .arg(m_histogramTime, 0, 'f', 2);

                                updateLastAction("Load File", timingInfo);
                                emit fileLoaded(true);

                            } catch (const std::exception& e) {
                                QMessageBox::critical(this, "Error",
                                QString("Failed to load file: %1").arg(e.what()));
                                emit fileLoaded(false);
                            }
                        }},
                       {"Undo", [this]() {
                            qDebug() << "\n=== Undo Button Clicked ===";
                            qDebug() << "Current history size:" << m_imageProcessor.getHistorySize();
                            qDebug() << "Can undo:" << m_imageProcessor.canUndo();

                            if (checkZoomMode()) {
                                qDebug() << "Zoom mode active, cannot undo";
                                return;
                            }

                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();

                            bool canUndo = m_imageProcessor.canUndo();
                            qDebug() << "Attempting undo operation, canUndo:" << canUndo;

                            if (canUndo) {
                                qDebug() << "Performing undo operation...";
                                m_imageProcessor.undo();
                                qDebug() << "Undo operation complete, new history size:" << m_imageProcessor.getHistorySize();

                                m_imageLabel->clearSelection();
                                updateImageDisplay();

                                auto lastAction = m_imageProcessor.getLastActionRecord();
                                updateLastAction("Undo",
                                lastAction.action.isEmpty() ? QString() :
                                QString("Reverted: %1").arg(lastAction.toString()));

                                // Enable/disable undo button based on remaining history
                                bool canStillUndo = m_imageProcessor.canUndo();
                                qDebug() << "Can still undo after operation:" << canStillUndo;
                                for (QPushButton* button : m_allButtons) {
                                    if (button && button->text() == "Undo") {
                                        button->setEnabled(canStillUndo);
                                        qDebug() << "Updated undo button enabled state:" << canStillUndo;
                                        break;
                                    }
                                }
                            }
                        }},
                       {"Save", [this]() {
                            QString filePath = QFileDialog::getSaveFileName(
                            this,
                            "Save Image",
                            "",
                            "PNG Files (*.png);;JPEG Files (*.jpg *.jpeg);;TIFF Files (*.tiff *.tif);;BMP Files (*.bmp)"
                            );

                            if (!filePath.isEmpty()) {
                                const auto& finalImage = m_imageProcessor.getFinalImage();
                                if (!finalImage) {
                                    QMessageBox::warning(this, "Save Error", "No image data to save.");
                                    return;
                                }

                                if (m_imageProcessor.saveImage(filePath)) {
                                    QFileInfo fileInfo(filePath);
                                    updateLastAction("Save Image", fileInfo.fileName());
                                } else {
                                    QMessageBox::critical(this, "Error", "Failed to save image.");
                                }
                            }
                        }}
                   });
}

void ControlPanel::setupBasicOperations() {
    QGroupBox* groupBox = new QGroupBox("Basic Operations");
    QVBoxLayout* mainLayout = new QVBoxLayout(groupBox);
    mainLayout->setSpacing(10);

    // Create and setup zoom button if not exists
    if (!m_zoomButton) {
        m_zoomButton = new QPushButton("Activate Zoom");
        m_zoomButton->setCheckable(true);
        m_zoomButton->setFixedHeight(35);
        m_zoomButton->setEnabled(false);
        m_allButtons.push_back(m_zoomButton);

        // Updated button styles to match the requirements
        m_zoomButton->setStyleSheet(
                    "QPushButton {"
                    "    background-color: #ffffff;"  // White background by default
                    "    color: #6b7280;"            // Black text by default
                    "    border: 1px solid #e5e7eb;"
                    "    border-radius: 4px;"
                    "    padding: 5px;"
                    "}"
                    // When text is "Deactivate Zoom" without fix
                    "QPushButton[state=\"deactivate-unfix\"] {"
                    "    background-color: #3b82f6;"  // Blue background
                    "    color: #6b7280;"  // Darker gray for disabled state
                    "    color: #ffffff;"             // White text
                    "    border: none;"
                    "}"
                    // When text is "Deactivate Zoom" with fix
                    "QPushButton[state=\"deactivate-fix\"] {"
                    "    background-color: #ef4444;"  // Red background
                    "    color: #ffffff;"             // White text
                    "    border: none;"
                    "}"
                    );

        // Update fix zoom button style
        m_fixZoomButton->setStyleSheet(
                    "QPushButton {"
                    "    background-color: #ffffff;"
                    "    color: #000000;"
                    "    border: 1px solid #e5e7eb;"
                    "    border-radius: 4px;"
                    "    padding: 5px;"
                    "}"
                    "QPushButton:checked {"
                    "    background-color: #3b82f6;"  // Blue when showing "Unfix Zoom"
                    "    color: #ffffff;"
                    "    border: none;"
                    "}"
                    );

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

    // Add zoom button and controls group
    mainLayout->addWidget(m_zoomButton);
    mainLayout->addWidget(m_zoomControlsGroup);

    // Add crop button
    QPushButton* cropBtn = new QPushButton("Crop");
    cropBtn->setFixedHeight(35);
    cropBtn->setEnabled(false);
    m_allButtons.push_back(cropBtn);
    mainLayout->addWidget(cropBtn);

    // Create horizontal layout for rotate buttons
    QHBoxLayout* rotateLayout = new QHBoxLayout();
    rotateLayout->setSpacing(10);

    // Create rotate buttons
    QPushButton* rotateCCWBtn = new QPushButton("Rotate CCW");
    QPushButton* rotateCWBtn = new QPushButton("Rotate CW");

    // Set properties for rotate buttons
    for (QPushButton* btn : {rotateCCWBtn, rotateCWBtn}) {
        btn->setFixedHeight(35);
        btn->setEnabled(false);
        m_allButtons.push_back(btn);
        btn->setStyleSheet(
                    "QPushButton {"
                    "    background-color: #ffffff;"
                    "    border: 1px solid #e5e7eb;"
                    "    border-radius: 4px;"
                    "    padding: 5px;"
                    "}"
                    );
    }

    // Add rotate buttons to horizontal layout
    rotateLayout->addWidget(rotateCCWBtn);
    rotateLayout->addWidget(rotateCWBtn);

    // Add rotate layout to main layout
    mainLayout->addLayout(rotateLayout);

    // Connect button signals
    connect(cropBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;
        m_imageProcessor.saveCurrentState();
        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        if (m_imageLabel->isRegionSelected()) {
            QRect selectedRegion = m_imageLabel->getSelectedRegion();

            // 获取选取区域的坐标，不管方向如何
            int x1 = selectedRegion.left();
            int y1 = selectedRegion.top();
            int x2 = selectedRegion.right();
            int y2 = selectedRegion.bottom();

            // 标准化坐标，确保 left <= right 且 top <= bottom
            int cropLeft = std::min(x1, x2);
            int cropRight = std::max(x1, x2);
            int cropTop = std::min(y1, y2);
            int cropBottom = std::max(y1, y2);

            // 处理缩放
            auto& zoomManager = m_imageProcessor.getZoomManager();
            if (zoomManager.isZoomModeActive()) {
                float zoomLevel = zoomManager.getZoomLevel();
                cropLeft = static_cast<int>(cropLeft / zoomLevel);
                cropRight = static_cast<int>(cropRight / zoomLevel);
                cropTop = static_cast<int>(cropTop / zoomLevel);
                cropBottom = static_cast<int>(cropBottom / zoomLevel);
            }

            try {
                const auto& inputImage = m_imageProcessor.getFinalImage();
                int inputHeight = m_imageProcessor.getFinalImageHeight();
                int inputWidth = m_imageProcessor.getFinalImageWidth();

                if (!inputImage || inputHeight <= 0 || inputWidth <= 0) {
                    QMessageBox::warning(this, "Crop Error", "Invalid input image.");
                    return;
                }

                // 确保坐标在图像范围内
                cropLeft = std::clamp(cropLeft, 0, inputWidth - 1);
                cropRight = std::clamp(cropRight, 0, inputWidth - 1);
                cropTop = std::clamp(cropTop, 0, inputHeight - 1);
                cropBottom = std::clamp(cropBottom, 0, inputHeight - 1);

                // 执行裁剪
                CGData croppedData = m_imageProcessor.cropRegion(
                            inputImage, inputHeight, inputWidth,
                            cropLeft, cropTop, cropRight, cropBottom
                            );

                // Convert CGData to double** for updateAndSaveFinalImage
                double** resultPtr = nullptr;
                malloc2D(resultPtr, croppedData.Row, croppedData.Column);

                for (int y = 0; y < croppedData.Row; y++) {
                    for (int x = 0; x < croppedData.Column; x++) {
                        resultPtr[y][x] = std::clamp(croppedData.Data[y][x], 0.0, 65535.0);
                    }
                }

                // Clean up CGData
                for (int i = 0; i < croppedData.Row; i++) {
                    free(croppedData.Data[i]);
                }
                free(croppedData.Data);

                // Update image
                m_imageProcessor.updateAndSaveFinalImage(resultPtr, croppedData.Row, croppedData.Column);

                // Clean up result pointer
                for (int i = 0; i < croppedData.Row; i++) {
                    free(resultPtr[i]);
                }
                free(resultPtr);

                m_imageLabel->clearSelection();
                updateImageDisplay();

                // Update status with the normalized crop dimensions
                QString cropInfo = QString("Region: (%1,%2) to (%3,%4)")
                        .arg(cropLeft).arg(cropTop).arg(cropRight).arg(cropBottom);
                updateLastAction("Crop", cropInfo);

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Crop Error",
                                      QString("Failed to crop image: %1").arg(e.what()));
            }
        } else {
            QMessageBox::information(this, "Crop Info",
                                     "Please select a region first.");
        }
    });

    connect(rotateCCWBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;
        m_imageProcessor.saveCurrentState();
        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();
        m_imageProcessor.rotateImage(90);
        m_imageLabel->clearSelection();
        updateImageDisplay();
        updateLastAction("Rotate Clockwise");
    });

    connect(rotateCWBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;
        m_imageProcessor.saveCurrentState();
        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();
        m_imageProcessor.rotateImage(270);
        m_imageLabel->clearSelection();
        updateImageDisplay();
        updateLastAction("Rotate CCW");
    });

    // Add the group box to scroll layout
    m_scrollLayout->addWidget(groupBox);
}

void ControlPanel::setupPreProcessingOperations() {
    QGroupBox* groupBox = new QGroupBox("Pre-Processing Operations");
    QVBoxLayout* layout = new QVBoxLayout(groupBox);

    // Add Process Image button after the interlace button
    QPushButton* processBtn = new QPushButton("Process Image");
    processBtn->setFixedHeight(35);
    processBtn->setToolTip("Process image with automatic dual energy processing");
    processBtn->setEnabled(false);
    m_allButtons.push_back(processBtn);
    layout->addWidget(processBtn);

    connect(processBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;

        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        // Create dialog for parameter input
        QDialog dialog(this);
        dialog.setWindowTitle("Process Image Settings");
        dialog.setMinimumWidth(400);
        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        // Energy Mode display (non-interactive, fixed to Dual)
        QGroupBox* energyBox = new QGroupBox("Energy Mode");
        QVBoxLayout* energyLayout = new QVBoxLayout(energyBox);

        QLabel* energyModeLabel = new QLabel("Fixed Mode: Dual Energy");
        energyModeLabel->setStyleSheet(
                    "QLabel {"
                    "    color: #dc2626;"  // Red text
                    "    font-weight: bold;"
                    "    padding: 5px;"
                    "}"
                    );

        QLabel* energyNote = new QLabel(
                    "Image will be processed as dual energy with automatic calibration.\n"
                    "This setting cannot be changed."
                    );
        energyNote->setStyleSheet("color: #666666; font-size: 10px; margin-left: 20px;");

        energyLayout->addWidget(energyModeLabel);
        energyLayout->addWidget(energyNote);
        dialogLayout->addWidget(energyBox);

        // Row Mode display (non-interactive, fixed to Unfold)
        QGroupBox* rowBox = new QGroupBox("Row Mode");
        QVBoxLayout* rowLayout = new QVBoxLayout(rowBox);

        QLabel* rowModeLabel = new QLabel("Fixed Mode: Unfold");
        rowModeLabel->setStyleSheet(
                    "QLabel {"
                    "    color: #dc2626;"  // Red text
                    "    font-weight: bold;"
                    "    padding: 5px;"
                    "}"
                    );

        QLabel* rowNote = new QLabel(
                    "Unfold mode is fixed: Rows will be separated into distinct sections.\n"
                    "This setting cannot be changed."
                    );
        rowNote->setStyleSheet("color: #666666; font-size: 10px; margin-left: 20px;");

        rowLayout->addWidget(rowModeLabel);
        rowLayout->addWidget(rowNote);
        dialogLayout->addWidget(rowBox);

        // Add calibration info box
        QGroupBox* calibrationBox = new QGroupBox("Automatic Calibration");
        QVBoxLayout* calibrationLayout = new QVBoxLayout(calibrationBox);

        QLabel* calibrationNote = new QLabel(
                    "Data Calibration Parameters (Fixed):\n"
                    "• Air Sample Start: " + QString::number(CGImageCalculationVariables.AirSampleStart) + "\n"
                                                                                                           "• Air Sample End: " + QString::number(CGImageCalculationVariables.AirSampleEnd) + "\n"
                                                                                                                                                                                              "• Pixel Max Value: " + QString::number(CGImageCalculationVariables.PixelMaxValue)
                    );
        calibrationNote->setStyleSheet(
                    "QLabel {"
                    "    color: #2563eb;"  // Blue text
                    "    background-color: #eff6ff;"  // Light blue background
                    "    border: 1px solid #bfdbfe;"  // Light blue border
                    "    border-radius: 4px;"
                    "    padding: 8px;"
                    "    margin: 4px 0px;"
                    "    font-family: monospace;"  // For better alignment of numbers
                    "}"
                    );

        calibrationLayout->addWidget(calibrationNote);
        dialogLayout->addWidget(calibrationBox);

        // Energy Layout selection (user can choose)
        QGroupBox* layoutBox = new QGroupBox("Energy Layout (Optional)");
        QVBoxLayout* layoutLayout = new QVBoxLayout(layoutBox);
        QRadioButton* lowHighBtn = new QRadioButton("Low-High");
        QRadioButton* highLowBtn = new QRadioButton("High-Low");
        lowHighBtn->setChecked(true);

        QLabel* layoutNote = new QLabel(
                    "Choose the energy layout for your image:\n"
                    "Low-High: Low energy rows followed by high energy rows\n"
                    "High-Low: High energy rows followed by low energy rows"
                    );
        layoutNote->setStyleSheet("color: #666666; font-size: 10px; margin-left: 20px;");

        layoutLayout->addWidget(lowHighBtn);
        layoutLayout->addWidget(highLowBtn);
        layoutLayout->addWidget(layoutNote);
        dialogLayout->addWidget(layoutBox);

        // Add OK/Cancel buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            m_imageProcessor.saveCurrentState();

            try {
                const auto& finalImage = m_imageProcessor.getFinalImage();
                if (!finalImage) {
                    QMessageBox::warning(this, "Error", "No image data available");
                    return;
                }

                int height = m_imageProcessor.getFinalImageHeight();
                int width = m_imageProcessor.getFinalImageWidth();

                // Prepare input matrix
                double** inputMatrix = nullptr;
                malloc2D(inputMatrix, height, width);
                for(int i = 0; i < height; i++) {
                    for(int j = 0; j < width; j++) {
                        inputMatrix[i][j] = finalImage[i][j];
                    }
                }

                // Process with fixed settings
                CGProcessImage processor;
                processor.SetEnergyMode(EnergyMode::Dual);  // Fixed
                processor.SetDualRowMode(DualRowMode::Unfold);  // Fixed
                processor.SetHighLowLayout(lowHighBtn->isChecked() ? EnergyLayout::Low_High : EnergyLayout::High_Low);

                // Process image (includes data calibration)
                processor.Process(inputMatrix, height, width);

                double** mergedData = nullptr;
                int mergedRows = 0, mergedCols = 0;
                if (processor.GetMergedData(mergedData, mergedRows, mergedCols)) {
                    // Step 2: Merge halves
                    m_imageProcessor.mergeHalves(mergedData, mergedRows, mergedCols);

                    // Update the image
                    m_imageProcessor.updateAndSaveFinalImage(mergedData, mergedRows, mergedCols/2);

                    m_imageSizeLabel->setText(QString("Image Size: %1 x %2")
                                              .arg(mergedRows)
                                              .arg(mergedCols/2));

                    // Clean up merged data
                    for(int i = 0; i < mergedRows; i++) {
                        free(mergedData[i]);
                    }
                    free(mergedData);
                }

                // Clean up input matrix
                for(int i = 0; i < height; i++) {
                    free(inputMatrix[i]);
                }
                free(inputMatrix);

                m_imageLabel->clearSelection();
                updateImageDisplay();

                QString paramStr = QString("Mode: Dual (Fixed)\n"
                                           "Row Mode: Unfold (Fixed)\n"
                                           "Layout: %1\n"
                                           "Air Sample: %2-%3\n"
                                           "Max Value: %4\n"
                                           "Final Size: %5x%6")
                        .arg(lowHighBtn->isChecked() ? "Low-High" : "High-Low")
                        .arg(CGImageCalculationVariables.AirSampleStart)
                        .arg(CGImageCalculationVariables.AirSampleEnd)
                        .arg(CGImageCalculationVariables.PixelMaxValue)
                        .arg(mergedRows)
                        .arg(mergedCols/2);
                updateLastAction("Process Image", paramStr);

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error",
                                      QString("Failed to process image: %1").arg(e.what()));
            }
        }
    });

    // Create unified calibration button
    m_calibrationButton = new QPushButton("Calibration");
    m_calibrationButton->setFixedHeight(35);
    m_calibrationButton->setToolTip("Apply calibration to image data");
    m_calibrationButton->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(m_calibrationButton);
    layout->addWidget(m_calibrationButton);

    // Connect unified calibration button
    connect(m_calibrationButton, &QPushButton::clicked, this, [this]() {
        if (checkZoomMode()) return;
        m_imageProcessor.saveCurrentState();
        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        // Create method selection dialog
        QDialog methodDialog(this);
        methodDialog.setWindowTitle("Select Calibration Method");
        methodDialog.setMinimumWidth(350);
        QVBoxLayout* methodLayout = new QVBoxLayout(&methodDialog);

        QGroupBox* methodBox = new QGroupBox("Calibration Method");
        QVBoxLayout* radioLayout = new QVBoxLayout(methodBox);

        QRadioButton* axisCalibrationBtn = new QRadioButton("Axis Calibration");
        QRadioButton* dataCalibrationBtn = new QRadioButton("Data Calibration");
        axisCalibrationBtn->setChecked(true);

        // Add explanation labels
        QLabel* axisLabel = new QLabel("Use axis-based calibration with reference lines");
        QLabel* dataLabel = new QLabel("Use air sample values for data calibration");

        axisLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
        dataLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

        radioLayout->addWidget(axisCalibrationBtn);
        radioLayout->addWidget(axisLabel);
        radioLayout->addWidget(dataCalibrationBtn);
        radioLayout->addWidget(dataLabel);

        methodLayout->addWidget(methodBox);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        methodLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &methodDialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &methodDialog, &QDialog::reject);

        if (methodDialog.exec() == QDialog::Accepted) {
            if (dataCalibrationBtn->isChecked()) {
                // Handle Data Calibration
                try {
                    const auto& finalImage = m_imageProcessor.getFinalImage();
                    if (!finalImage) {
                        QMessageBox::warning(this, "Error", "No image data available");
                        return;
                    }

                    int rows = m_imageProcessor.getFinalImageHeight();
                    int cols = m_imageProcessor.getFinalImageWidth();

                    double** matrix;
                    malloc2D(matrix, rows, cols);
                    for(int i = 0; i < rows; i++) {
                        for(int j = 0; j < cols; j++) {
                            matrix[i][j] = finalImage[i][j];
                        }
                    }

                    CGData calibratedData = DataCalibration::CalibrateDataMatrix(
                                matrix,
                                rows,
                                cols,
                                CGImageCalculationVariables.PixelMaxValue,
                                CGImageCalculationVariables.AirSampleStart,
                                CGImageCalculationVariables.AirSampleEnd
                                );

                    double** calibratedMatrix = nullptr;
                    malloc2D(calibratedMatrix, calibratedData.Row, calibratedData.Column);

                    for(int i = 0; i < calibratedData.Row; i++) {
                        for(int j = 0; j < calibratedData.Column; j++) {
                            calibratedMatrix[i][j] = std::clamp(calibratedData.Data[i][j], 0.0, 65535.0);
                        }
                    }

                    m_imageProcessor.updateAndSaveFinalImage(calibratedMatrix, calibratedData.Row, calibratedData.Column);
                    m_imageProcessor.setCalibrationApplied(true);
                    updateImageDisplay();

                    // Clean up
                    for(int i = 0; i < rows; i++) {
                        free(matrix[i]);
                    }
                    free(matrix);

                    for(int i = 0; i < calibratedData.Row; i++) {
                        free(calibratedData.Data[i]);
                        free(calibratedMatrix[i]);
                    }
                    free(calibratedData.Data);
                    free(calibratedMatrix);

                    QString params = QString("Air Sample: %1-%2, Max Value: %3")
                            .arg(CGImageCalculationVariables.AirSampleStart)
                            .arg(CGImageCalculationVariables.AirSampleEnd)
                            .arg(CGImageCalculationVariables.PixelMaxValue);
                    updateLastAction("Data Calibration", params);

                } catch (const std::exception& e) {
                    QMessageBox::critical(this, "Error",
                                          QString("Calibration failed: %1").arg(e.what()));
                }
            } else {
                // Handle Axis Calibration - existing axis calibration code
                QDialog dialog(this);
                dialog.setWindowTitle("Axis Calibration");
                dialog.setMinimumWidth(350);
                QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

                // Add current parameters display if they exist
                if (InterlaceProcessor::hasCalibrationParams()) {
                    QLabel* currentParamsLabel = new QLabel(QString("Current Parameters: Y=%1, X=%2")
                                                            .arg(InterlaceProcessor::getStoredYParam())
                                                            .arg(InterlaceProcessor::getStoredXParam()));
                    currentParamsLabel->setStyleSheet(
                                "QLabel {"
                                "    color: #2563eb;"  // Blue color
                                "    font-weight: bold;"
                                "    background-color: #eff6ff;"  // Light blue background
                                "    border: 1px solid #bfdbfe;"
                                "    border-radius: 4px;"
                                "    padding: 8px;"
                                "    margin: 4px 0px;"
                                "}"
                                );
                    dialogLayout->addWidget(currentParamsLabel);
                }

                // Mode selection
                QGroupBox* modeBox = new QGroupBox("Calibration Mode");
                QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);

                QRadioButton* bothAxesRadio = new QRadioButton("Both Axes");
                QRadioButton* yAxisRadio = new QRadioButton("Y-Axis Only");
                QRadioButton* xAxisRadio = new QRadioButton("X-Axis Only");

                // Add explanations
                QLabel* bothAxesLabel = new QLabel("Calibrate using both vertical and horizontal reference lines");
                QLabel* yAxisLabel = new QLabel("Calibrate using only vertical reference lines");
                QLabel* xAxisLabel = new QLabel("Calibrate using only horizontal reference lines");

                QString explanationStyle = "color: gray; font-size: 10px; margin-left: 20px;";
                bothAxesLabel->setStyleSheet(explanationStyle);
                yAxisLabel->setStyleSheet(explanationStyle);
                xAxisLabel->setStyleSheet(explanationStyle);

                bothAxesRadio->setChecked(true);

                modeLayout->addWidget(bothAxesRadio);
                modeLayout->addWidget(bothAxesLabel);
                modeLayout->addWidget(yAxisRadio);
                modeLayout->addWidget(yAxisLabel);
                modeLayout->addWidget(xAxisRadio);
                modeLayout->addWidget(xAxisLabel);

                modeBox->setLayout(modeLayout);
                dialogLayout->addWidget(modeBox);

                QDialogButtonBox* axisButtonBox = new QDialogButtonBox(
                            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                dialogLayout->addWidget(axisButtonBox);

                // Add a note about parameters
                QLabel* noteLabel = new QLabel(
                            "Note: Parameters will be requested after selecting the calibration mode.\n"
                            "Previously stored parameters will be used if available for the selected axes.");
                noteLabel->setStyleSheet(
                            "QLabel {"
                            "    color: #666666;"
                            "    font-style: italic;"
                            "    font-size: 10px;"
                            "    padding: 4px;"
                            "    margin-top: 8px;"
                            "}"
                            );
                dialogLayout->addWidget(noteLabel);
                connect(axisButtonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                connect(axisButtonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                if (dialog.exec() == QDialog::Accepted) {
                    CalibrationMode mode;
                    if (yAxisRadio->isChecked()) {
                        mode = CalibrationMode::Y_AXIS_ONLY;
                    } else if (xAxisRadio->isChecked()) {
                        mode = CalibrationMode::X_AXIS_ONLY;
                    } else {
                        mode = CalibrationMode::BOTH_AXES;
                    }

                    processCalibration(0, 0, mode);
                    m_resetCalibrationButton->setEnabled(true);
                    m_resetCalibrationButton->setVisible(true);
                }
            }
        }
    });

    QPushButton* interlaceBtn = new QPushButton("Interlace");
    interlaceBtn->setFixedHeight(35);
    interlaceBtn->setToolTip("Apply interlacing process with dual energy mode");
    interlaceBtn->setEnabled(false);  // Initially disabled
    m_allButtons.push_back(interlaceBtn);
    layout->addWidget(interlaceBtn);

    connect(interlaceBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;

        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        // Create dialog for parameter input
        QDialog dialog(this);
        dialog.setWindowTitle("Interlace Settings");
        dialog.setMinimumWidth(400);
        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        // Energy Mode display (fixed to Dual)
        QGroupBox* energyBox = new QGroupBox("Energy Mode");
        QVBoxLayout* energyLayout = new QVBoxLayout(energyBox);

        QLabel* energyModeLabel = new QLabel("Fixed Mode: Dual Energy");
        energyModeLabel->setStyleSheet(
                    "QLabel {"
                    "    color: #dc2626;"  // Red text
                    "    font-weight: bold;"
                    "    padding: 5px;"
                    "}"
                    );

        QLabel* energyNote = new QLabel(
                    "Image will be processed as dual energy mode.\n"
                    "This setting cannot be changed."
                    );
        energyNote->setStyleSheet("color: #666666; font-size: 10px; margin-left: 20px;");

        energyLayout->addWidget(energyModeLabel);
        energyLayout->addWidget(energyNote);
        dialogLayout->addWidget(energyBox);

        // Row Mode display (fixed to Unfold)
        QGroupBox* rowBox = new QGroupBox("Row Mode");
        QVBoxLayout* rowLayout = new QVBoxLayout(rowBox);

        QLabel* rowModeLabel = new QLabel("Fixed Mode: Unfold");
        rowModeLabel->setStyleSheet(
                    "QLabel {"
                    "    color: #dc2626;"  // Red text
                    "    font-weight: bold;"
                    "    padding: 5px;"
                    "}"
                    );

        QLabel* rowNote = new QLabel(
                    "Unfold mode is fixed: Rows will be separated into distinct sections.\n"
                    "This setting cannot be changed."
                    );
        rowNote->setStyleSheet("color: #666666; font-size: 10px; margin-left: 20px;");

        rowLayout->addWidget(rowModeLabel);
        rowLayout->addWidget(rowNote);
        dialogLayout->addWidget(rowBox);

        // Calibration Options
        QGroupBox* calibrationBox = new QGroupBox("Calibration Settings");
        QVBoxLayout* calibrationLayout = new QVBoxLayout(calibrationBox);

        // Add calibration checkbox and status
        QCheckBox* applyCalibrationCheck = new QCheckBox("Apply Data Calibration");

        if (m_imageProcessor.hasAppliedCalibration()) {
            // If calibration was already applied, allow user to choose
            applyCalibrationCheck->setChecked(false);
            applyCalibrationCheck->setEnabled(true);

            QLabel* calibrationStatus = new QLabel("Data Calibration has already been applied\nYou can choose to apply it again if needed");
            calibrationStatus->setStyleSheet(
                        "QLabel {"
                        "    color: #059669;"  // Green text
                        "    font-weight: bold;"
                        "    background-color: #ecfdf5;"  // Light green background
                        "    border: 1px solid #6ee7b7;"
                        "    border-radius: 4px;"
                        "    padding: 8px;"
                        "    margin: 4px 0px;"
                        "}"
                        );
            calibrationLayout->addWidget(calibrationStatus);
        } else {
            // If calibration hasn't been applied, check by default and disable
            applyCalibrationCheck->setChecked(true);
            applyCalibrationCheck->setEnabled(false);

            QLabel* calibrationStatus = new QLabel("Data Calibration will be automatically applied");
            calibrationStatus->setStyleSheet(
                        "QLabel {"
                        "    color: #2563eb;"  // Blue text
                        "    font-weight: bold;"
                        "    background-color: #eff6ff;"  // Light blue background
                        "    border: 1px solid #bfdbfe;"
                        "    border-radius: 4px;"
                        "    padding: 8px;"
                        "    margin: 4px 0px;"
                        "}"
                        );
            calibrationLayout->addWidget(calibrationStatus);
        }

        // Show current calibration parameters
        QLabel* calibrationParamsLabel = new QLabel(
                    "Calibration Parameters:\n"
                    "• Air Sample Start: " + QString::number(CGImageCalculationVariables.AirSampleStart) + "\n"
                                                                                                           "• Air Sample End: " + QString::number(CGImageCalculationVariables.AirSampleEnd) + "\n"
                                                                                                                                                                                              "• Pixel Max Value: " + QString::number(CGImageCalculationVariables.PixelMaxValue)
                    );
        calibrationParamsLabel->setStyleSheet(
                    "QLabel {"
                    "    color: #2563eb;"
                    "    background-color: #eff6ff;"
                    "    border: 1px solid #bfdbfe;"
                    "    border-radius: 4px;"
                    "    padding: 8px;"
                    "    margin: 4px 0px;"
                    "    font-family: monospace;"
                    "}"
                    );
        calibrationLayout->addWidget(calibrationParamsLabel);
        calibrationLayout->addWidget(applyCalibrationCheck);
        dialogLayout->addWidget(calibrationBox);

        // Energy Layout selection
        QGroupBox* layoutBox = new QGroupBox("Energy Layout (Optional)");
        QVBoxLayout* layoutLayout = new QVBoxLayout(layoutBox);
        QRadioButton* lowHighBtn = new QRadioButton("Low-High");
        QRadioButton* highLowBtn = new QRadioButton("High-Low");
        lowHighBtn->setChecked(true);

        QLabel* layoutNote = new QLabel(
                    "Choose the energy layout for your image:\n"
                    "Low-High: Low energy rows followed by high energy rows\n"
                    "High-Low: High energy rows followed by low energy rows"
                    );
        layoutNote->setStyleSheet("color: #666666; font-size: 10px; margin-left: 20px;");

        layoutLayout->addWidget(lowHighBtn);
        layoutLayout->addWidget(highLowBtn);
        layoutLayout->addWidget(layoutNote);
        dialogLayout->addWidget(layoutBox);

        // Add OK/Cancel buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            m_imageProcessor.saveCurrentState();

            try {
                const auto& finalImage = m_imageProcessor.getFinalImage();
                if (!finalImage) {
                    QMessageBox::warning(this, "Error", "No image data available");
                    return;
                }

                int height = m_imageProcessor.getFinalImageHeight();
                int width = m_imageProcessor.getFinalImageWidth();

                // Prepare input matrix
                double** inputMatrix = nullptr;
                malloc2D(inputMatrix, height, width);
                for(int i = 0; i < height; i++) {
                    for(int j = 0; j < width; j++) {
                        inputMatrix[i][j] = finalImage[i][j];
                    }
                }

                // Apply data calibration if needed
                if (!m_imageProcessor.hasAppliedCalibration()) {
                    // Auto-apply calibration if it hasn't been applied yet
                    QMessageBox::information(this, "Data Calibration",
                                             "Data calibration has not been applied yet. It will be automatically applied before interlacing.");

                    CGData calibratedData = DataCalibration::CalibrateDataMatrix(
                                inputMatrix,
                                height,
                                width,
                                CGImageCalculationVariables.PixelMaxValue,
                                CGImageCalculationVariables.AirSampleStart,
                                CGImageCalculationVariables.AirSampleEnd
                                );

                    // Copy calibrated data back to input matrix
                    for(int i = 0; i < height; i++) {
                        for(int j = 0; j < width; j++) {
                            inputMatrix[i][j] = std::clamp(calibratedData.Data[i][j], 0.0, 65535.0);
                        }
                    }

                    // Clean up calibrated data
                    for(int i = 0; i < calibratedData.Row; i++) {
                        free(calibratedData.Data[i]);
                    }
                    free(calibratedData.Data);

                    m_imageProcessor.setCalibrationApplied(true);
                }
                else if (applyCalibrationCheck->isChecked()) {
                    // Apply calibration if user chose to reapply it
                    CGData calibratedData = DataCalibration::CalibrateDataMatrix(
                                inputMatrix,
                                height,
                                width,
                                CGImageCalculationVariables.PixelMaxValue,
                                CGImageCalculationVariables.AirSampleStart,
                                CGImageCalculationVariables.AirSampleEnd
                                );

                    // Copy calibrated data back to input matrix
                    for(int i = 0; i < height; i++) {
                        for(int j = 0; j < width; j++) {
                            inputMatrix[i][j] = std::clamp(calibratedData.Data[i][j], 0.0, 65535.0);
                        }
                    }

                    // Clean up calibrated data
                    for(int i = 0; i < calibratedData.Row; i++) {
                        free(calibratedData.Data[i]);
                    }
                    free(calibratedData.Data);
                }

                // Process with fixed settings
                CGProcessImage processor;
                processor.SetEnergyMode(EnergyMode::Dual);  // Fixed to Dual
                processor.SetDualRowMode(DualRowMode::Unfold);  // Fixed to Unfold
                processor.SetHighLowLayout(lowHighBtn->isChecked() ? EnergyLayout::Low_High : EnergyLayout::High_Low);

                // Process image
                processor.Process(inputMatrix, height, width);

                double** mergedData = nullptr;
                int mergedRows = 0, mergedCols = 0;
                if (processor.GetMergedData(mergedData, mergedRows, mergedCols)) {
                    // Update the image
                    m_imageProcessor.updateAndSaveFinalImage(mergedData, mergedRows, mergedCols);

                    m_imageSizeLabel->setText(QString("Image Size: %1 x %2")
                                              .arg(mergedRows)
                                              .arg(mergedCols));

                    // Clean up merged data
                    for(int i = 0; i < mergedRows; i++) {
                        free(mergedData[i]);
                    }
                    free(mergedData);
                }

                // Clean up input matrix
                for(int i = 0; i < height; i++) {
                    free(inputMatrix[i]);
                }
                free(inputMatrix);

                m_imageLabel->clearSelection();
                updateImageDisplay();

                // Update the status message to include calibration information
                QString calibrationStatus = !m_imageProcessor.hasAppliedCalibration() ?
                            "Applied" :
                            (applyCalibrationCheck->isChecked() ? "Reapplied" : "Previously Applied");

                QString paramStr = QString("Mode: Dual (Fixed)\n"
                                           "Row Mode: Unfold (Fixed)\n"
                                           "Layout: %1\n"
                                           "Calibration: %2\n"
                                           "Final Size: %3x%4")
                        .arg(lowHighBtn->isChecked() ? "Low-High" : "High-Low")
                        .arg(calibrationStatus)
                        .arg(mergedRows)
                        .arg(mergedCols);
                updateLastAction("Interlace", paramStr);

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error",
                                      QString("Failed to process image: %1").arg(e.what()));
            }
        }
    });

    QPushButton* mergeBtn = new QPushButton("Merge Halves");
    mergeBtn->setFixedHeight(35);
    mergeBtn->setToolTip("Merge left and right halves of the image");
    mergeBtn->setEnabled(false);
    m_allButtons.push_back(mergeBtn);
    layout->addWidget(mergeBtn);

    connect(mergeBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;
        m_imageProcessor.saveCurrentState();
        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        try {
            const auto& finalImage = m_imageProcessor.getFinalImage();
            if (!finalImage) {
                QMessageBox::warning(this, "Error", "No image data available");
                return;
            }

            int height = m_imageProcessor.getFinalImageHeight();
            int width = m_imageProcessor.getFinalImageWidth();

            // Create a copy of the image for processing
            double** imgCopy = nullptr;
            malloc2D(imgCopy, height, width);

            // Copy the image data
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    imgCopy[y][x] = finalImage[y][x];
                }
            }

            // Perform the merge
            m_imageProcessor.mergeHalves(imgCopy, height, width);

            // Update the image with merged result
            m_imageProcessor.updateAndSaveFinalImage(imgCopy, height, width/2);

            m_imageSizeLabel->setText(QString("Image Size: %1 x %2")
                                      .arg(height)
                                      .arg(width/2));

            m_imageLabel->clearSelection();
            updateImageDisplay();
            updateLastAction("Merge Halves", "Width: " + QString::number(width/2));

        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to merge image halves: %1").arg(e.what()));
        }
    });

    layout->setSpacing(10);
    layout->addStretch();
    m_scrollLayout->addWidget(groupBox);
}



void ControlPanel::setupFilteringOperations() {

    createGroupBox("Image Enhancement", {
                       {"CLAHE", [this]() {
                            if (checkZoomMode()) return;

                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();

                            // Use non-const pointer to allow modifications
                            QDialog* dialog = new QDialog(this);
                            dialog->setWindowTitle("CLAHE Settings");
                            dialog->setMinimumWidth(350);
                            QVBoxLayout* mainLayout = new QVBoxLayout(dialog);

                            // CLAHE Type Selection
                            QGroupBox* typeBox = new QGroupBox("CLAHE Type");
                            QVBoxLayout* typeLayout = new QVBoxLayout(typeBox);
                            QRadioButton* standardRadio = new QRadioButton("Standard CLAHE");
                            QRadioButton* thresholdRadio = new QRadioButton("Threshold CLAHE");
                            QRadioButton* combinedRadio = new QRadioButton("Combined CLAHE");
                            standardRadio->setChecked(true);

                            typeLayout->addWidget(standardRadio);
                            typeLayout->addWidget(thresholdRadio);
                            typeLayout->addWidget(combinedRadio);

                            // Add explanation labels for each type
                            QLabel* standardLabel = new QLabel("Applies standard CLAHE processing to the entire image");
                            QLabel* thresholdLabel = new QLabel("Applies CLAHE only to pixels below a threshold value");
                            QLabel* combinedLabel = new QLabel("Applies standard CLAHE first, then adjusts based on pixel brightness");

                            standardLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            thresholdLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            combinedLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

                            typeLayout->addWidget(standardLabel);
                            typeLayout->addWidget(thresholdLabel);
                            typeLayout->addWidget(combinedLabel);
                            mainLayout->addWidget(typeBox);

                            // Processing Mode Selection
                            QGroupBox* processingBox = new QGroupBox("Processing Hardware");
                            QVBoxLayout* processingLayout = new QVBoxLayout(processingBox);
                            QRadioButton* gpuRadio = new QRadioButton("GPU Processing");
                            QRadioButton* cpuRadio = new QRadioButton("CPU Processing");
                            gpuRadio->setChecked(true);
                            processingLayout->addWidget(gpuRadio);
                            processingLayout->addWidget(cpuRadio);
                            mainLayout->addWidget(processingBox);

                            // Parameters Section
                            QGroupBox* paramsBox = new QGroupBox("Parameters");
                            QVBoxLayout* paramsLayout = new QVBoxLayout(paramsBox);

                            // Standard CLAHE parameters (always visible)
                            QLabel* clipLabel = new QLabel("Clip Limit:");
                            QDoubleSpinBox* clipSpinBox = new QDoubleSpinBox();
                            clipSpinBox->setRange(0.1, 1000.0);
                            clipSpinBox->setValue(2.0);
                            clipSpinBox->setSingleStep(0.1);

                            QLabel* tileLabel = new QLabel("Tile Size:");
                            QSpinBox* tileSpinBox = new QSpinBox();
                            tileSpinBox->setRange(2, 1000);
                            tileSpinBox->setValue(8);

                            paramsLayout->addWidget(clipLabel);
                            paramsLayout->addWidget(clipSpinBox);
                            paramsLayout->addWidget(tileLabel);
                            paramsLayout->addWidget(tileSpinBox);

                            // Threshold value (only for threshold CLAHE)
                            QLabel* thresholdValueLabel = new QLabel("Threshold Value:");
                            QSpinBox* thresholdValueSpinBox = new QSpinBox();
                            thresholdValueSpinBox->setRange(0, 65535);
                            thresholdValueSpinBox->setValue(5000);
                            thresholdValueSpinBox->setVisible(false);
                            thresholdValueLabel->setVisible(false);

                            paramsLayout->addWidget(thresholdValueLabel);
                            paramsLayout->addWidget(thresholdValueSpinBox);

                            mainLayout->addWidget(paramsBox);

                            // Connect radio button changes to show/hide threshold value
                            // Use resize() instead of adjustSize() to avoid const issues
                            connect(standardRadio, &QRadioButton::toggled, dialog, [=](bool checked) {
                                thresholdValueLabel->setVisible(!checked);
                                thresholdValueSpinBox->setVisible(!checked);
                                dialog->resize(dialog->sizeHint());
                            });

                            connect(thresholdRadio, &QRadioButton::toggled, dialog, [=](bool checked) {
                                thresholdValueLabel->setVisible(checked);
                                thresholdValueSpinBox->setVisible(checked);
                                dialog->resize(dialog->sizeHint());
                            });

                            connect(combinedRadio, &QRadioButton::toggled, dialog, [=](bool checked) {
                                thresholdValueLabel->setVisible(false);
                                thresholdValueSpinBox->setVisible(false);
                                dialog->resize(dialog->sizeHint());
                            });

                            QDialogButtonBox* buttonBox = new QDialogButtonBox(
                            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                            mainLayout->addWidget(buttonBox);

                            connect(buttonBox, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
                            connect(buttonBox, &QDialogButtonBox::rejected, dialog, &QDialog::reject);

                            if (dialog->exec() == QDialog::Accepted) {
                                try {
                                    const auto& finalImage = m_imageProcessor.getFinalImage();
                                    if (!finalImage) {
                                        QMessageBox::warning(this, "Error", "No image data available");
                                        return;
                                    }

                                    int height = m_imageProcessor.getFinalImageHeight();
                                    int width = m_imageProcessor.getFinalImageWidth();

                                    // Allocate buffers directly for 16-bit values
                                    double** inputBuffer = CLAHEProcessor::allocateImageBuffer(height, width);
                                    double** outputBuffer = CLAHEProcessor::allocateImageBuffer(height, width);

                                    // Copy input data (already in 16-bit range)
                                    for (int y = 0; y < height; ++y) {
                                        for (int x = 0; x < width; ++x) {
                                            inputBuffer[y][x] = finalImage[y][x];
                                        }
                                    }

                                    CLAHEProcessor claheProcessor;
                                    bool useGPU = gpuRadio->isChecked();
                                    QString statusMsg;

                                    auto startTime = std::chrono::high_resolution_clock::now();

                                    if (standardRadio->isChecked()) {
                                        // Standard CLAHE
                                        if (useGPU) {
                                            claheProcessor.applyCLAHE(outputBuffer, inputBuffer, height, width,
                                            clipSpinBox->value(),
                                            cv::Size(tileSpinBox->value(), tileSpinBox->value()));
                                        } else {
                                            claheProcessor.applyCLAHE_CPU(outputBuffer, inputBuffer, height, width,
                                            clipSpinBox->value(),
                                            cv::Size(tileSpinBox->value(), tileSpinBox->value()));
                                        }
                                        statusMsg = QString("Standard (%1)")
                                        .arg(useGPU ? "GPU" : "CPU");
                                    }
                                    else if (thresholdRadio->isChecked()) {
                                        // Threshold CLAHE
                                        if (useGPU) {
                                            claheProcessor.applyThresholdCLAHE(inputBuffer, height, width,
                                            thresholdValueSpinBox->value(),
                                            clipSpinBox->value(),
                                            cv::Size(tileSpinBox->value(), tileSpinBox->value()));
                                        } else {
                                            claheProcessor.applyThresholdCLAHE_CPU(inputBuffer, height, width,
                                            thresholdValueSpinBox->value(),
                                            clipSpinBox->value(),
                                            cv::Size(tileSpinBox->value(), tileSpinBox->value()));
                                        }
                                        // Copy results
                                        for (int y = 0; y < height; ++y) {
                                            for (int x = 0; x < width; ++x) {
                                                outputBuffer[y][x] = inputBuffer[y][x];
                                            }
                                        }
                                        statusMsg = QString("Threshold (%1)")
                                        .arg(useGPU ? "GPU" : "CPU");
                                    }
                                    else {
                                        // Combined CLAHE
                                        if (useGPU) {
                                            claheProcessor.applyCombinedCLAHE(outputBuffer, inputBuffer, height, width,
                                            clipSpinBox->value(),
                                            cv::Size(tileSpinBox->value(), tileSpinBox->value()));
                                        } else {
                                            claheProcessor.applyCombinedCLAHE_CPU(outputBuffer, inputBuffer, height, width,
                                            clipSpinBox->value(),
                                            cv::Size(tileSpinBox->value(), tileSpinBox->value()));
                                        }
                                        statusMsg = QString("Combined (%1)")
                                        .arg(useGPU ? "GPU" : "CPU");
                                    }

                                    // End timing
                                    auto endTime = std::chrono::high_resolution_clock::now();
                                    double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

                                    // Add processing time to status message
                                    statusMsg += QString("\nProcessing Time: %1 ms").arg(processingTime, 0, 'f', 2);

                                    // Update image (values already in 16-bit range)
                                    m_imageProcessor.updateAndSaveFinalImage(outputBuffer, height, width);

                                    // Cleanup
                                    CLAHEProcessor::deallocateImageBuffer(inputBuffer, height);
                                    CLAHEProcessor::deallocateImageBuffer(outputBuffer, height);

                                    // Update display
                                    updateImageDisplay();
                                    updateLastAction("CLAHE", statusMsg);

                                    // Update timing labels
                                    if (useGPU) {
                                        m_gpuTimingLabel->setText(QString("GPU Time: %1 ms").arg(processingTime, 0, 'f', 2));
                                        m_gpuTimingLabel->setVisible(true);
                                        m_lastGpuTime = processingTime;
                                    } else {
                                        m_cpuTimingLabel->setText(QString("CPU Time: %1 ms").arg(processingTime, 0, 'f', 2));
                                        m_cpuTimingLabel->setVisible(true);
                                        m_lastCpuTime = processingTime;
                                    }

                                } catch (const cv::Exception& e) {
                                    QMessageBox::critical(this, "Error",
                                    QString("CLAHE processing failed: %1").arg(e.what()));
                                }
                            }
                        }},
                       {"Median Filter", [this]() {
                            if (checkZoomMode()) return;
                            m_imageProcessor.saveCurrentState();
                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();
                            auto [filterKernelSize, ok] = showInputDialog("Median Filter", "Enter kernel size:", 3, 1, 21);

                            if (ok) {
                                try {
                                    const auto& currentImage = m_imageProcessor.getFinalImage();
                                    if (!currentImage) {
                                        QMessageBox::warning(this, "Error", "No image data available");
                                        return;
                                    }

                                    int height = m_imageProcessor.getFinalImageHeight();
                                    int width = m_imageProcessor.getFinalImageWidth();

                                    // Allocate input image
                                    double** inputImage = CLAHEProcessor::allocateImageBuffer(height, width);

                                    // Copy data
                                    for (int y = 0; y < height; y++) {
                                        for (int x = 0; x < width; x++) {
                                            inputImage[y][x] = currentImage[y][x];
                                        }
                                    }

                                    // Apply filter
                                    m_imageProcessor.applyMedianFilter(inputImage, height, width, filterKernelSize);

                                    // Update the image directly
                                    m_imageProcessor.updateAndSaveFinalImage(inputImage, height, width);

                                    // Cleanup
                                    CLAHEProcessor::deallocateImageBuffer(inputImage, height);

                                    m_imageLabel->clearSelection();
                                    updateImageDisplay();
                                    updateLastAction("Median Filter", QString("Size: %1").arg(filterKernelSize));

                                } catch (const std::exception& e) {
                                    QMessageBox::critical(this, "Error",
                                    QString("Failed to apply median filter: %1").arg(e.what()));
                                }
                            }
                        }},
                       {"High-Pass Filter", [this]() {
                            if (checkZoomMode()) return;
                            m_imageProcessor.saveCurrentState();
                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();

                            try {
                                const auto& currentImage = m_imageProcessor.getFinalImage();
                                if (!currentImage) {  // Check for null pointer instead of empty()
                                                      QMessageBox::warning(this, "Error", "No image data available");
                                                      return;
                                }

                                int height = m_imageProcessor.getFinalImageHeight();
                                int width = m_imageProcessor.getFinalImageWidth();

                                // Allocate input image
                                double** inputImage = nullptr;
                                malloc2D(inputImage, height, width);

                                // Copy data
                                for (int y = 0; y < height; y++) {
                                    for (int x = 0; x < width; x++) {
                                        inputImage[y][x] = currentImage[y][x];
                                    }
                                }

                                // Apply filter
                                m_imageProcessor.applyHighPassFilter(inputImage, height, width);

                                // Update display directly with double** format
                                m_imageProcessor.updateAndSaveFinalImage(inputImage, height, width);

                                // Clean up
                                for (int i = 0; i < height; i++) {
                                    free(inputImage[i]);
                                }
                                free(inputImage);

                                m_imageLabel->clearSelection();
                                updateImageDisplay();
                                updateLastAction("High-Pass Filter");

                            } catch (const std::exception& e) {
                                QMessageBox::critical(this, "Error",
                                QString("Failed to apply high-pass filter: %1").arg(e.what()));
                            }
                        }},
                       {"Edge Enhancement", [this]() {
                            if (checkZoomMode()) return;
                            m_imageProcessor.saveCurrentState();
                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();

                            auto [strength, ok] = showInputDialog(
                            "Edge Enhancement",
                            "Enter enhancement strength (0.1-2.0):",
                            0.5, 0.1, 2.0);

                            if (ok) {
                                try {
                                    const auto& currentImage = m_imageProcessor.getFinalImage();
                                    if (!currentImage) {  // Check for null pointer instead of empty()
                                                          QMessageBox::warning(this, "Error", "No image data available");
                                                          return;
                                    }

                                    int height = m_imageProcessor.getFinalImageHeight();
                                    int width = m_imageProcessor.getFinalImageWidth();

                                    // Allocate input image
                                    double** inputImage = nullptr;
                                    malloc2D(inputImage, height, width);

                                    // Copy data
                                    for (int y = 0; y < height; y++) {
                                        for (int x = 0; x < width; x++) {
                                            inputImage[y][x] = currentImage[y][x];
                                        }
                                    }

                                    // Apply enhancement
                                    m_imageProcessor.applyEdgeEnhancement(inputImage, height, width, strength);

                                    // Update display directly with double** format
                                    m_imageProcessor.updateAndSaveFinalImage(inputImage, height, width);

                                    // Clean up
                                    for (int i = 0; i < height; i++) {
                                        free(inputImage[i]);
                                    }
                                    free(inputImage);

                                    m_imageLabel->clearSelection();
                                    updateImageDisplay();
                                    updateLastAction("Edge Enhancement",
                                    QString("Strength: %1").arg(strength, 0, 'f', 2));

                                } catch (const std::exception& e) {
                                    QMessageBox::critical(this, "Error",
                                    QString("Failed to apply edge enhancement: %1").arg(e.what()));
                                }
                            }
                        }}
                   });
}

void ControlPanel::setupAdvancedOperations() {
    QGroupBox* groupBox = new QGroupBox("Advanced Operations");
    QVBoxLayout* mainLayout = new QVBoxLayout(groupBox);

    // Stretch Operations
    QPushButton* stretchBtn = new QPushButton("Stretch Image");
    stretchBtn->setFixedHeight(35);
    stretchBtn->setEnabled(false);
    mainLayout->addWidget(stretchBtn);

    connect(stretchBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;

        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        // Show input dialogs
        QStringList directions = {"Vertical", "Horizontal"};
        bool ok1, ok2;
        QString direction = QInputDialog::getItem(this, "Stretch Direction",
                                                  "Select Stretch Direction:", directions, 0, false, &ok1);

        if (!ok1) return; // User cancelled

        double factor = QInputDialog::getDouble(this, "Stretch Factor",
                                                "Enter Stretch Factor:", 1.5, 0.1, 10.0, 2, &ok2);

        if (!ok2) return; // User cancelled

        m_imageProcessor.saveCurrentState();
        resetDetectedLinesPointer();

        const auto& finalImage = m_imageProcessor.getFinalImage();
        int height = m_imageProcessor.getFinalImageHeight();
        int width = m_imageProcessor.getFinalImageWidth();

        // Allocate input image
        double** inputImage = nullptr;
        malloc2D(inputImage, height, width);

        // Copy data
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                inputImage[y][x] = finalImage[y][x];
            }
        }

        try {
            if (direction == "Vertical") {
                m_imageProcessor.stretchImageY(inputImage, height, width, factor);
            } else {
                m_imageProcessor.stretchImageX(inputImage, height, width, factor);
            }

            // Update display with the stretched image
            m_imageProcessor.updateAndSaveFinalImage(inputImage, height, width);

            // Clean up
            for(int i = 0; i < height; i++) {
                free(inputImage[i]);
            }
            free(inputImage);

            m_imageLabel->clearSelection();
            updateImageDisplay();
            updateLastAction("Stretch Image", QString("%1 - %2").arg(direction).arg(factor, 0, 'f', 2));

        } catch (const std::exception& e) {
            // Clean up on error
            for(int i = 0; i < height; i++) {
                free(inputImage[i]);
            }
            free(inputImage);
            QMessageBox::critical(this, "Error", QString("Failed to stretch image: %1").arg(e.what()));
        }
    });

    // Padding Operations
    QPushButton* applyPaddingBtn = new QPushButton("Apply Padding");
    applyPaddingBtn->setFixedHeight(35);
    applyPaddingBtn->setEnabled(false);
    mainLayout->addWidget(applyPaddingBtn);

    connect(applyPaddingBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;

        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        // Show input dialog
        bool ok;
        int paddingSize = QInputDialog::getInt(this, "Padding",
                                               "Enter Padding Size:", 10, 1, 1000, 1, &ok);

        if (!ok) return; // User cancelled

        m_imageProcessor.saveCurrentState();
        resetDetectedLinesPointer();

        const auto& finalImage = m_imageProcessor.getFinalImage();
        int height = m_imageProcessor.getFinalImageHeight();
        int width = m_imageProcessor.getFinalImageWidth();

        double** inputImage = new double*[height];
        for(int i = 0; i < height; i++) {
            inputImage[i] = new double[width];
            for(int j = 0; j < width; j++) {
                inputImage[i][j] = finalImage[i][j];
            }
        }

        try {
            m_imageProcessor.addPadding(inputImage, height, width, paddingSize);

            // Update image with the new padded dimensions
            m_imageProcessor.updateAndSaveFinalImage(inputImage, height, width);

            // Cleanup
            for(int i = 0; i < height; i++) {
                delete[] inputImage[i];
            }
            delete[] inputImage;

            m_imageLabel->clearSelection();
            updateImageDisplay();
            updateLastAction("Add Padding", QString::number(paddingSize));

        } catch (const std::exception& e) {
            // Cleanup on error
            for(int i = 0; i < height; i++) {
                delete[] inputImage[i];
            }
            delete[] inputImage;
            QMessageBox::critical(this, "Error", QString("Failed to add padding: %1").arg(e.what()));
        }
    });

    // Distortion Operations
    QPushButton* applyDistortionBtn = new QPushButton("Apply Distortion");
    applyDistortionBtn->setFixedHeight(35);
    applyDistortionBtn->setEnabled(false);
    mainLayout->addWidget(applyDistortionBtn);

    connect(applyDistortionBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;

        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

        // Show input dialogs
        QStringList directions = {"Left", "Right", "Top", "Bottom"};
        bool ok1, ok2;
        QString direction = QInputDialog::getItem(this, "Distortion Direction",
                                                  "Select Distortion Direction:", directions, 0, false, &ok1);

        if (!ok1) return; // User cancelled

        double factor = QInputDialog::getDouble(this, "Distortion Factor",
                                                "Enter Distortion Factor:", 1.5, 0.1, 100.0, 2, &ok2);

        if (!ok2) return; // User cancelled

        m_imageProcessor.saveCurrentState();
        resetDetectedLinesPointer();

        const auto& finalImage = m_imageProcessor.getFinalImage();
        int height = m_imageProcessor.getFinalImageHeight();
        int width = m_imageProcessor.getFinalImageWidth();

        double** inputImage = new double*[height];
        for(int i = 0; i < height; i++) {
            inputImage[i] = new double[width];
            for(int j = 0; j < width; j++) {
                inputImage[i][j] = finalImage[i][j];
            }
        }

        try {
            m_imageProcessor.distortImage(inputImage, height, width, factor, direction.toStdString());

            std::vector<std::vector<uint16_t>> result(height, std::vector<uint16_t>(width));
            for(int i = 0; i < height; i++) {
                for(int j = 0; j < width; j++) {
                    result[i][j] = static_cast<uint16_t>(std::clamp(inputImage[i][j], 0.0, 65535.0));
                }
            }

            m_imageProcessor.updateAndSaveFinalImage(inputImage, height, width);

            // Cleanup
            for(int i = 0; i < height; i++) {
                delete[] inputImage[i];
            }
            delete[] inputImage;

            m_imageLabel->clearSelection();
            updateImageDisplay();
            updateLastAction("Apply Distortion", QString("%1 - %2").arg(direction).arg(factor, 0, 'f', 2));

        } catch (const std::exception& e) {
            // Cleanup on error
            for(int i = 0; i < height; i++) {
                delete[] inputImage[i];
            }
            delete[] inputImage;
            QMessageBox::critical(this, "Error", QString("Failed to apply distortion: %1").arg(e.what()));
        }
    });

    mainLayout->addStretch();

    m_allButtons.push_back(stretchBtn);
    m_allButtons.push_back(applyPaddingBtn);
    m_allButtons.push_back(applyDistortionBtn);

    m_scrollLayout->addWidget(groupBox);
}

std::pair<double, bool> ControlPanel::showInputDialog(
        const QString& title,
        const QString& label,
        double defaultValue,
        double min,
        double max)
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

    // Initially disable buttons
    detectBtn->setEnabled(false);
    removeBtn->setEnabled(false);

    // Set fixed height for buttons
    const int buttonHeight = 35;
    detectBtn->setFixedHeight(buttonHeight);
    removeBtn->setFixedHeight(buttonHeight);

    // Add buttons to layout
    layout->addWidget(detectBtn);
    layout->addWidget(removeBtn);

    // Inside ControlPanel::setupBlackLineDetection()
    connect(detectBtn, &QPushButton::clicked, [this, removeBtn]() {
        if (checkZoomMode()) return;
        m_imageProcessor.saveCurrentState();

        // Create method selection dialog
        QDialog dialog(this);
        dialog.setWindowTitle("Dark Line Detection");
        dialog.setMinimumWidth(350);
        QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);

        // Line type selection
        QGroupBox* lineTypeBox = new QGroupBox("Line Types to Detect");
        QVBoxLayout* lineTypeLayout = new QVBoxLayout(lineTypeBox);

        QCheckBox* verticalCheck = new QCheckBox("Detect Vertical Lines");
        QCheckBox* horizontalCheck = new QCheckBox("Detect Horizontal Lines");

        QLabel* lineTypeExplanation = new QLabel(
                    "Select at least one line type to detect.\n"
                    "Detection will only process selected types."
                    );
        lineTypeExplanation->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

        lineTypeLayout->addWidget(verticalCheck);
        lineTypeLayout->addWidget(horizontalCheck);
        lineTypeLayout->addWidget(lineTypeExplanation);
        lineTypeBox->setLayout(lineTypeLayout);
        dialogLayout->addWidget(lineTypeBox);

        // Add OK/Cancel buttons
        QDialogButtonBox* buttonBox = new QDialogButtonBox(
                    QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            // Check if at least one option is selected
            if (!verticalCheck->isChecked() && !horizontalCheck->isChecked()) {
                QMessageBox::warning(this, "Detection Error",
                                     "Please select at least one line type to detect (Vertical and/or Horizontal).");
                return;
            }

            try {
                int height, width;
                ImageData imageData;
                ImageDataGuard guard(imageData);

                if (!initializeImageData(imageData, height, width)) {
                    return;
                }

                DarkLineArray* outLines = nullptr;
                bool success = false;

                bool detectVertical = verticalCheck->isChecked();
                bool detectHorizontal = horizontalCheck->isChecked();

                // Execute detection based on selection
                try {
                    if (detectVertical && detectHorizontal) {
                        success = DarkLinePointerProcessor::checkforBoth(imageData, outLines);
                    } else if (detectVertical) {
                        success = DarkLinePointerProcessor::checkforVertical(imageData, outLines);
                    } else if (detectHorizontal) {
                        success = DarkLinePointerProcessor::checkforHorizontal(imageData, outLines);
                    }

                    // Process detection results
                    if (processDetectedLines(outLines, success)) {
                        QString detectionInfo = PointerOperations::generateDarkLineInfo(m_detectedLinesPointer);
                        updateDarkLineDisplay(detectionInfo);

                        // Enable remove button
                        removeBtn->setEnabled(true);
                        updateImageDisplay();

                        // Generate detection type string
                        QStringList types;
                        if (detectVertical) types << "Vertical";
                        if (detectHorizontal) types << "Horizontal";
                        QString typeStr = types.join(" and ");
                        updateLastAction("Detect Dark Lines", typeStr);
                    } else {
                        QMessageBox::information(this, "Detection Result", "No dark lines detected.");
                    }

                } catch (const std::exception& e) {
                    handleDetectionError(e, outLines);
                }

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error",
                                      QString("Failed to initialize image data: %1").arg(e.what()));
            }
        }
    });

    // Connect remove button
    connect(removeBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;

        if (!m_detectedLinesPointer || m_detectedLinesPointer->rows == 0) {
            QMessageBox::information(this, "Remove Lines", "Please detect lines first.");
            return;
        }

        try {
            m_imageProcessor.saveCurrentState();
            PointerOperations::handleRemoveLinesDialog(this);
        } catch (const std::exception& e) {
            QMessageBox::critical(this, "Error",
                                  QString("Failed to remove lines: %1").arg(e.what()));
        }
    });

    // Add buttons to the list for general enable/disable management
    m_allButtons.push_back(detectBtn);
    m_allButtons.push_back(removeBtn);

    // Add tooltips
    detectBtn->setToolTip("Detect dark lines in the image");
    removeBtn->setToolTip("Remove detected dark lines");

    // Add the group box to the main layout
    m_scrollLayout->addWidget(groupBox);
}

void ControlPanel::resetDetectedLinesPointer() {
    if (m_detectedLinesPointer) {
        DarkLinePointerProcessor::destroyDarkLineArray(m_detectedLinesPointer);
        m_detectedLinesPointer = nullptr;
    }

    m_imageProcessor.clearDetectedLines();

    // Find the reset detection button and hide it
    for (QPushButton* button : m_allButtons) {
        if (button && button->text() == "Reset Detection") {
            button->setVisible(false);
            button->setEnabled(false);
            break;
        }
    }

    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
                qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
                );

    m_darkLineInfoLabel->clear();
    m_darkLineInfoLabel->setVisible(false);
    darkLineScrollArea->setVisible(false);

    updateImageDisplay();
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

void ControlPanel::updateImageDisplay(double** image, int height, int width) {
    qDebug() << "\n=== Starting updateImageDisplay ===";

    if (!m_imageLabel) {
        qDebug() << "Error: Image label is null";
        return;
    }

    // 1. Get image data if not provided
    double** finalImage = image;
    int finalHeight = height;
    int finalWidth = width;

    if (!finalImage) {
        finalImage = m_imageProcessor.getFinalImage();
        finalHeight = m_imageProcessor.getFinalImageHeight();
        finalWidth = m_imageProcessor.getFinalImageWidth();
    }

    if (!finalImage || finalHeight <= 0 || finalWidth <= 0) {
        qDebug() << "No valid image data";
        m_imageSizeLabel->setText("Image Size: No image loaded");
        m_imageLabel->clear();
        return;
    }

    m_imageSizeLabel->setText(QString("Image Size: %1 x %2")
                              .arg(finalWidth)
                              .arg(finalHeight));

    qDebug() << "Image dimensions:" << finalWidth << "x" << finalHeight;

    try {
        // 2. Create base image with safety checks
        std::unique_ptr<QImage> image;
        try {
            image = std::make_unique<QImage>(finalWidth, finalHeight, QImage::Format_Grayscale16);
            if (image->isNull()) {
                throw std::runtime_error("Failed to create QImage");
            }
        } catch (const std::exception& e) {
            qDebug() << "Error creating image:" << e.what();
            return;
        }

        // 3. Fill image data with bounds checking
        for (int y = 0; y < finalHeight; ++y) {
            if (!finalImage[y]) continue;  // Skip if row pointer is null
            for (int x = 0; x < finalWidth; ++x) {
                uint16_t pixelValue = std::clamp(static_cast<uint16_t>(finalImage[y][x]),
                                                 static_cast<uint16_t>(0),
                                                 static_cast<uint16_t>(65535));
                image->setPixel(x, y, qRgb(pixelValue >> 8, pixelValue >> 8, pixelValue >> 8));
            }
        }

        // 4. Create pixmap from image
        std::unique_ptr<QPixmap> pixmap;
        try {
            pixmap = std::make_unique<QPixmap>(QPixmap::fromImage(*image));
            if (pixmap->isNull()) {
                throw std::runtime_error("Failed to create QPixmap");
            }
        } catch (const std::exception& e) {
            qDebug() << "Error creating pixmap:" << e.what();
            return;
        }

        // 5. Handle zoom with safety checks
        const auto& zoomManager = m_imageProcessor.getZoomManager();
        float zoomLevel = zoomManager.getZoomLevel();

        if (zoomManager.isZoomModeActive() && zoomLevel != 1.0f) {
            QSize zoomedSize = zoomManager.getZoomedSize(pixmap->size());
            if (zoomedSize.isValid() && !zoomedSize.isNull()) {
                QPixmap scaledPixmap = pixmap->scaled(zoomedSize,
                                                      Qt::KeepAspectRatio,
                                                      Qt::SmoothTransformation);
                if (!scaledPixmap.isNull()) {
                    *pixmap = scaledPixmap;
                }
            }
        }

        // 6. Draw detected lines if any
        bool hasLines = m_detectedLinesPointer &&
                m_detectedLinesPointer->rows > 0 &&
                m_detectedLinesPointer->cols > 0 &&
                m_detectedLinesPointer->lines;

        if (hasLines) {
            qDebug() << "Processing detected lines...";
            qDebug() << "Number of rows:" << m_detectedLinesPointer->rows;
            qDebug() << "Number of columns:" << m_detectedLinesPointer->cols;

            // Create a copy of pixmap for drawing
            std::unique_ptr<QPixmap> drawPixmap = std::make_unique<QPixmap>(*pixmap);

            // Create painter
            {
                QPainter painter(drawPixmap.get());
                painter.setRenderHint(QPainter::Antialiasing);

                float penWidth = zoomManager.isZoomModeActive() ?
                            std::max(1.0f, 2.0f * zoomLevel) : 2.0f;

                QFont labelFont = painter.font();
                float scaledSize = std::min(11.0f * zoomLevel, 24.0f);
                labelFont.setPixelSize(static_cast<int>(std::max(11.0f, scaledSize)));
                labelFont.setFamily("Arial");
                painter.setFont(labelFont);

                int lineCount = 0;
                for (int i = 0; i < m_detectedLinesPointer->rows && m_detectedLinesPointer->lines; ++i) {
                    if (!m_detectedLinesPointer->lines[i]) continue;

                    for (int j = 0; j < m_detectedLinesPointer->cols; ++j) {
                        const auto& line = m_detectedLinesPointer->lines[i][j];

                        // Calculate line rectangle
                        QRect lineRect;
                        if (line.isVertical) {
                            if (line.x < 0 || line.x >= finalWidth || line.width <= 0) continue;

                            int adjustedX = static_cast<int>(line.x * zoomLevel);
                            int adjustedWidth = std::max(1, static_cast<int>(line.width * zoomLevel));
                            int adjustedStartY = static_cast<int>(line.startY * zoomLevel);
                            int adjustedEndY = static_cast<int>(line.endY * zoomLevel);

                            adjustedX = qBound(0, adjustedX, drawPixmap->width() - 1);
                            adjustedEndY = qMin(adjustedEndY, drawPixmap->height());

                            lineRect = QRect(adjustedX, adjustedStartY,
                                             adjustedWidth, adjustedEndY - adjustedStartY);
                        } else {
                            if (line.y < 0 || line.y >= finalHeight || line.width <= 0) continue;

                            int adjustedY = static_cast<int>(line.y * zoomLevel);
                            int adjustedHeight = std::max(1, static_cast<int>(line.width * zoomLevel));
                            int adjustedStartX = static_cast<int>(line.startX * zoomLevel);
                            int adjustedEndX = static_cast<int>(line.endX * zoomLevel);

                            adjustedY = qBound(0, adjustedY, drawPixmap->height() - 1);
                            adjustedEndX = qMin(adjustedEndX, drawPixmap->width());

                            lineRect = QRect(adjustedStartX, adjustedY,
                                             adjustedEndX - adjustedStartX, adjustedHeight);
                        }

                        if (lineRect.isEmpty() || !lineRect.isValid()) continue;

                        // Draw line
                        QColor lineColor = line.inObject ?
                                    QColor(0, 0, 255, 128) : QColor(255, 0, 0, 128);
                        painter.setPen(QPen(lineColor, penWidth, Qt::SolidLine));
                        painter.setBrush(QBrush(lineColor, Qt::Dense4Pattern));
                        painter.drawRect(lineRect);

                        // Draw label
                        drawLineLabelWithCountPointer(painter, line, lineCount, zoomLevel, drawPixmap->size());

                        lineCount++;
                    }
                }
                qDebug() << "Drew" << lineCount << "lines";
            }

            // Update display with the drawn pixmap
            if (m_imageLabel) {
                m_imageLabel->setPixmap(*drawPixmap);
                m_imageLabel->setFixedSize(drawPixmap->size());
            }
        } else {
            // Update display with the original pixmap
            if (m_imageLabel && !pixmap->isNull()) {
                m_imageLabel->setPixmap(*pixmap);
                m_imageLabel->setFixedSize(pixmap->size());
            }
        }

        // 7. Update histogram if needed
        if (m_histogram && m_histogram->isVisible()) {
            m_histogram->updateHistogram(finalImage, finalHeight, finalWidth);
        }

        qDebug() << "=== Display update completed successfully ===\n";

    } catch (const std::exception& e) {
        qDebug() << "Error in updateImageDisplay:" << e.what();
        QMessageBox::critical(this, "Error",
                              QString("Failed to update display: %1").arg(e.what()));
    }
}

void ControlPanel::clearAllDetectionResults() {
    resetDetectedLinesPointer();
    m_darkLineInfoLabel->clear();
    m_darkLineInfoLabel->setVisible(false);

    // Hide and disable the reset detection button
    for (QPushButton* button : m_allButtons) {
        if (button && button->text() == "Reset Detection") {
            button->setVisible(false);
            button->setEnabled(false);
            break;
        }
    }

    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
                qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
                );
    if (darkLineScrollArea) {
        darkLineScrollArea->setVisible(false);
    }

    updateImageDisplay();
}

void ControlPanel::setupResetOperations() {
    // Create reset detection button but don't add it to the group box yet
    QPushButton* resetDetectionBtn = new QPushButton("Reset Detection");
    resetDetectionBtn->setFixedHeight(35);
    resetDetectionBtn->setVisible(false); // Initially hidden
    resetDetectionBtn->setEnabled(false);

    connect(resetDetectionBtn, &QPushButton::clicked, [this]() {
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
    });

    // Create reset calibration button
    m_resetCalibrationButton = new QPushButton("Reset Calibration Parameters");
    m_resetCalibrationButton->setFixedHeight(35);
    m_resetCalibrationButton->setEnabled(false);
    m_resetCalibrationButton->setVisible(false);
    m_resetCalibrationButton->setToolTip("Reset stored calibration parameters");

    // Connect reset calibration button signal - THIS IS THE NEW PART
    connect(m_resetCalibrationButton, &QPushButton::clicked, this, [this]() {
        InterlaceProcessor::resetCalibrationParams();
        m_resetCalibrationButton->setEnabled(false);
        m_resetCalibrationButton->setVisible(false);
        QMessageBox::information(this, "Calibration Reset",
                                 "Calibration parameters have been reset. Next calibration will require new parameters.");
    });

    // Store buttons for management
    m_allButtons.push_back(resetDetectionBtn);
    m_allButtons.push_back(m_resetCalibrationButton);

    // Connect to line detection changes to show/hide reset detection button
    connect(this, &ControlPanel::fileLoaded, [resetDetectionBtn](bool loaded) {
        if (!loaded) {
            resetDetectionBtn->setVisible(false);
            resetDetectionBtn->setEnabled(false);
        }
    });

    createGroupBox("Reset Operations", {
                       {QString(), resetDetectionBtn},
                       {QString(), m_resetCalibrationButton},
                       {"Clear All", [this]() {
                            QMessageBox::StandardButton reply = QMessageBox::warning(
                            this,
                            "Clear All",
                            "Are you sure you want to clear everything?\nThis will remove the image and all settings.",
                            QMessageBox::Yes | QMessageBox::No
                            );

                            if (reply == QMessageBox::Yes) {
                                // Clear history and parameters
                                InterlaceProcessor::resetCalibrationParams();
                                m_resetCalibrationButton->setEnabled(false);

                                // Clear detection results
                                clearAllDetectionResults();

                                // Reset zoom if active
                                if (m_imageProcessor.getZoomManager().isZoomModeActive()) {
                                    m_imageProcessor.getZoomManager().resetZoom();
                                    m_fixZoomButton->setChecked(false);
                                    m_imageProcessor.getZoomManager().toggleFixedZoom(false);
                                    toggleZoomMode(false);
                                }

                                // Clear all image data
                                m_imageProcessor.clearImageData();
                                m_imageProcessor.clearHistory();

                                // Clear display and labels
                                m_imageLabel->clear();
                                m_imageSizeLabel->setText("Image Size: No image loaded");
                                m_lastActionLabel->setText("Last Action: None");
                                m_lastActionParamsLabel->clear();
                                m_lastActionParamsLabel->setVisible(false);
                                m_pixelInfoLabel->setText("Pixel Info: No image loaded");
                                m_fileTypeLabel->setText("File Type: None");

                                // Disable all buttons except Browse
                                for (QPushButton* button : m_allButtons) {
                                    if (button) {
                                        button->setEnabled(button->text() == "Browse");
                                    }
                                }

                                m_resetCalibrationButton->setEnabled(false);
                                m_resetCalibrationButton->setVisible(false);

                                updateLastAction("Clear All");
                                QMessageBox::information(this, "Success", "All data has been cleared.");
                            }
                        }}
                   });
}


void ControlPanel::setupCombinedAdjustments() {
    createGroupBox("Image Adjustments", {
                       {"Gamma Adjustment", [this]() {
                            if (checkZoomMode()) return;
                            m_imageProcessor.saveCurrentState();
                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();

                            QDialog dialog(this);
                            dialog.setWindowTitle("Gamma Adjustment");
                            dialog.setMinimumWidth(350);

                            QVBoxLayout* layout = new QVBoxLayout(&dialog);

                            // Mode selection
                            QGroupBox* modeBox = new QGroupBox("Adjustment Mode");
                            QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);

                            QRadioButton* overallRadio = new QRadioButton("Overall Adjustment");
                            QRadioButton* regionalRadio = new QRadioButton("Regional Adjustment");
                            QRadioButton* thresholdRadio = new QRadioButton("Threshold-based Adjustment");

                            overallRadio->setChecked(true);

                            // Add explanatory labels
                            QLabel* overallLabel = new QLabel("Apply gamma correction to the entire image");
                            QLabel* regionalLabel = new QLabel("Apply gamma correction to a selected region only");
                            QLabel* thresholdLabel = new QLabel("Apply gamma correction to pixels below threshold only");

                            overallLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            regionalLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            thresholdLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

                            modeLayout->addWidget(overallRadio);
                            modeLayout->addWidget(overallLabel);
                            modeLayout->addWidget(regionalRadio);
                            modeLayout->addWidget(regionalLabel);
                            modeLayout->addWidget(thresholdRadio);
                            modeLayout->addWidget(thresholdLabel);

                            layout->addWidget(modeBox);

                            // Parameter inputs group
                            QGroupBox* paramBox = new QGroupBox("Parameters");
                            QVBoxLayout* paramLayout = new QVBoxLayout(paramBox);

                            // Style for spinboxes
                            QString spinBoxStyle =
                            "QSpinBox, QDoubleSpinBox {"
                            "    background-color: white;"
                            "    border: 1px solid #c0c0c0;"
                            "    border-radius: 3px;"
                            "    padding: 1px;"
                            "}"
                            "QSpinBox:focus, QDoubleSpinBox:focus {"
                            "    border: 1px solid #0078d7;"
                            "}"
                            "QSpinBox::up-button, QSpinBox::down-button,"
                            "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
                            "    width: 0px;"
                            "    border: none;"
                            "}";

                            // Gamma value input
                            QLabel* gammaLabel = new QLabel("Gamma Value:");
                            QDoubleSpinBox* gammaSpinBox = new QDoubleSpinBox();
                            gammaSpinBox->setRange(0.01, 10.0);
                            gammaSpinBox->setValue(1.0);
                            gammaSpinBox->setDecimals(2);
                            gammaSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
                            gammaSpinBox->setStyleSheet(spinBoxStyle);
                            gammaSpinBox->setMinimumWidth(150);  // Increased width
                            gammaSpinBox->setToolTip("Enter gamma value (0.01-10.00)");

                            paramLayout->addWidget(gammaLabel);
                            paramLayout->addWidget(gammaSpinBox);

                            // Threshold value input
                            QWidget* thresholdWidget = new QWidget();
                            QVBoxLayout* thresholdLayout = new QVBoxLayout(thresholdWidget);

                            QLabel* thresholdValueLabel = new QLabel("Threshold Value:");
                            QSpinBox* thresholdSpinBox = new QSpinBox();
                            thresholdSpinBox->setRange(0, 65535);
                            thresholdSpinBox->setValue(32767);
                            thresholdSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
                            thresholdSpinBox->setStyleSheet(spinBoxStyle);
                            thresholdSpinBox->setToolTip("Enter threshold value (0-65535)");

                            thresholdLayout->addWidget(thresholdValueLabel);
                            thresholdLayout->addWidget(thresholdSpinBox);
                            thresholdWidget->setVisible(false);

                            paramLayout->addWidget(thresholdWidget);
                            // Region selection status and info
                            QLabel* regionStatusLabel = new QLabel();
                            QLabel* regionInfoLabel = new QLabel(
                            "Please select a region in the image before applying the adjustment.\n"
                            "Click and drag to define the region of interest.");

                            regionStatusLabel->setStyleSheet("font-weight: bold;");
                            regionInfoLabel->setStyleSheet("color: #666666; font-size: 10px; padding: 5px;");
                            regionInfoLabel->setWordWrap(true);

                            regionStatusLabel->setVisible(false);
                            regionInfoLabel->setVisible(false);

                            paramLayout->addWidget(regionStatusLabel);
                            paramLayout->addWidget(regionInfoLabel);

                            layout->addWidget(paramBox);

                            // Update region status when selection changes
                            auto updateRegionStatus = [=]() {
                                if (regionalRadio->isChecked()) {
                                    if (m_imageLabel->isRegionSelected()) {
                                        QRect region = m_imageLabel->getSelectedRegion();
                                        regionStatusLabel->setText(
                                        QString("Selected Region: (%1,%2) - %3x%4")
                                        .arg(region.x())
                                        .arg(region.y())
                                        .arg(region.width())
                                        .arg(region.height()));
                                        regionStatusLabel->setStyleSheet("color: green; font-weight: bold;");
                                        regionInfoLabel->setVisible(false);
                                    } else {
                                        regionStatusLabel->setText("No region selected");
                                        regionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
                                        regionInfoLabel->setVisible(true);
                                    }
                                    regionStatusLabel->setVisible(true);
                                } else {
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                }
                            };

                            // Connect radio buttons to update control visibility
                            connect(overallRadio, &QRadioButton::toggled, [&dialog, regionStatusLabel, regionInfoLabel, thresholdWidget](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(false);
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                    dialog.adjustSize();
                                }
                            });

                            connect(regionalRadio, &QRadioButton::toggled, [&dialog, thresholdWidget, updateRegionStatus](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(false);
                                    updateRegionStatus();
                                    dialog.adjustSize();
                                }
                            });

                            connect(thresholdRadio, &QRadioButton::toggled, [&dialog, regionStatusLabel, regionInfoLabel, thresholdWidget](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(true);
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                    dialog.adjustSize();
                                }
                            });

                            // Create and add button box
                            QDialogButtonBox* buttonBox = new QDialogButtonBox(
                            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                            layout->addWidget(buttonBox);

                            // Connect button box
                            connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                            connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                            // Initial status update
                            updateRegionStatus();

                            if (dialog.exec() == QDialog::Accepted) {
                                float gamma = gammaSpinBox->value();
                                try {
                                    const auto& finalImage = m_imageProcessor.getFinalImage();
                                    if (!finalImage) {
                                        QMessageBox::warning(this, "Error", "No image data available");
                                        return;
                                    }

                                    int height = m_imageProcessor.getFinalImageHeight();
                                    int width = m_imageProcessor.getFinalImageWidth();

                                    // Allocate memory using malloc2D from pch.h
                                    double** imgData = nullptr;
                                    malloc2D(imgData, height, width);

                                    // Copy data
                                    for (int y = 0; y < height; y++) {
                                        for (int x = 0; x < width; x++) {
                                            imgData[y][x] = finalImage[y][x];
                                        }
                                    }

                                    QString modeStr;
                                    QString paramStr;

                                    if (thresholdRadio->isChecked()) {
                                        float threshold = thresholdSpinBox->value();
                                        ImageAdjustments::adjustGammaWithThreshold(imgData, height, width, gamma, threshold);
                                        modeStr = "Threshold";
                                        paramStr = QString("Gamma: %1, Threshold: %2")
                                        .arg(gamma, 0, 'f', 2)
                                        .arg(threshold);
                                    }
                                    else if (regionalRadio->isChecked()) {
                                        if (!m_imageLabel->isRegionSelected()) {
                                            // Clean up allocated memory
                                            for (int i = 0; i < height; i++) {
                                                free(imgData[i]);
                                            }
                                            free(imgData);

                                            QMessageBox::information(this, "Region Selection",
                                            "Please select a region first, then try again.");
                                            return;
                                        }

                                        QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                        ImageAdjustments::adjustGammaForSelectedRegion(
                                        imgData, height, width, gamma, selectedRegion);

                                        modeStr = "Regional";
                                        paramStr = QString("Gamma: %1\nRegion: (%2,%3,%4,%5)")
                                        .arg(gamma, 0, 'f', 2)
                                        .arg(selectedRegion.x())
                                        .arg(selectedRegion.y())
                                        .arg(selectedRegion.width())
                                        .arg(selectedRegion.height());
                                    }
                                    else {
                                        ImageAdjustments::adjustGammaOverall(imgData, height, width, gamma);
                                        modeStr = "Overall";
                                        paramStr = QString("Gamma: %1").arg(gamma, 0, 'f', 2);
                                    }

                                    // Update image
                                    m_imageProcessor.updateAndSaveFinalImage(imgData, height, width);

                                    // Clean up
                                    for (int i = 0; i < height; i++) {
                                        free(imgData[i]);
                                    }
                                    free(imgData);

                                    // Update display
                                    m_imageLabel->clearSelection();
                                    updateImageDisplay();
                                    updateLastAction("Gamma Adjustment",
                                    QString("Mode: %1\n%2").arg(modeStr).arg(paramStr));

                                } catch (const std::exception& e) {
                                    QMessageBox::critical(this, "Error",
                                    QString("Failed to apply gamma adjustment: %1").arg(e.what()));
                                }
                            }
                        }},
                       {"Contrast Adjustment", [this]() {
                            if (checkZoomMode()) return;
                            m_imageProcessor.saveCurrentState();
                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();

                            QDialog dialog(this);
                            dialog.setWindowTitle("Contrast Adjustment");
                            dialog.setMinimumWidth(350);

                            QVBoxLayout* layout = new QVBoxLayout(&dialog);

                            // Mode selection
                            QGroupBox* modeBox = new QGroupBox("Adjustment Mode");
                            QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);

                            QRadioButton* overallRadio = new QRadioButton("Overall Adjustment");
                            QRadioButton* regionalRadio = new QRadioButton("Regional Adjustment");
                            QRadioButton* thresholdRadio = new QRadioButton("Threshold-based Adjustment");

                            overallRadio->setChecked(true);

                            // Add explanatory labels
                            QLabel* overallLabel = new QLabel("Adjust contrast across the entire image");
                            QLabel* regionalLabel = new QLabel("Adjust contrast in a selected region only");
                            QLabel* thresholdLabel = new QLabel("Adjust contrast for pixels below threshold only");

                            overallLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            regionalLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            thresholdLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

                            modeLayout->addWidget(overallRadio);
                            modeLayout->addWidget(overallLabel);
                            modeLayout->addWidget(regionalRadio);
                            modeLayout->addWidget(regionalLabel);
                            modeLayout->addWidget(thresholdRadio);
                            modeLayout->addWidget(thresholdLabel);

                            layout->addWidget(modeBox);

                            // Parameter inputs group
                            QGroupBox* paramBox = new QGroupBox("Parameters");
                            QVBoxLayout* paramLayout = new QVBoxLayout(paramBox);

                            // Define spinbox style
                            QString spinBoxStyle =
                            "QSpinBox, QDoubleSpinBox {"
                            "    background-color: white;"
                            "    border: 1px solid #c0c0c0;"
                            "    border-radius: 3px;"
                            "    padding: 1px;"
                            "    min-width: 100px;"
                            "}"
                            "QSpinBox:focus, QDoubleSpinBox:focus {"
                            "    border: 1px solid #0078d7;"
                            "}"
                            "QSpinBox::up-button, QSpinBox::down-button,"
                            "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
                            "    width: 0px;"
                            "    border: none;"
                            "}";

                            // Contrast factor input
                            QLabel* contrastLabel = new QLabel("Contrast Factor:");
                            QDoubleSpinBox* contrastSpinBox = new QDoubleSpinBox();
                            contrastSpinBox->setRange(0.01, 5.0);
                            contrastSpinBox->setValue(1.0);
                            contrastSpinBox->setDecimals(2);
                            contrastSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
                            contrastSpinBox->setStyleSheet(spinBoxStyle);
                            contrastSpinBox->setToolTip("Enter contrast value (0.01-5.00)");

                            paramLayout->addWidget(contrastLabel);
                            paramLayout->addWidget(contrastSpinBox);

                            // Threshold value input
                            QWidget* thresholdWidget = new QWidget();
                            QVBoxLayout* thresholdLayout = new QVBoxLayout(thresholdWidget);

                            QLabel* thresholdValueLabel = new QLabel("Threshold Value:");
                            QSpinBox* thresholdSpinBox = new QSpinBox();
                            thresholdSpinBox->setRange(0, 65535);
                            thresholdSpinBox->setValue(32767);
                            thresholdSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
                            thresholdSpinBox->setStyleSheet(spinBoxStyle);
                            thresholdSpinBox->setToolTip("Enter threshold value (0-65535)");

                            thresholdLayout->addWidget(thresholdValueLabel);
                            thresholdLayout->addWidget(thresholdSpinBox);
                            thresholdWidget->setVisible(false);

                            paramLayout->addWidget(thresholdWidget);

                            // Region selection status and info
                            QLabel* regionStatusLabel = new QLabel();
                            QLabel* regionInfoLabel = new QLabel(
                            "Please select a region in the image before applying the adjustment.\n"
                            "Click and drag to define the region of interest.");

                            regionStatusLabel->setStyleSheet("font-weight: bold;");
                            regionInfoLabel->setStyleSheet("color: #666666; font-size: 10px; padding: 5px;");
                            regionInfoLabel->setWordWrap(true);

                            regionStatusLabel->setVisible(false);
                            regionInfoLabel->setVisible(false);

                            paramLayout->addWidget(regionStatusLabel);
                            paramLayout->addWidget(regionInfoLabel);

                            layout->addWidget(paramBox);

                            // Add this update region status lambda before the radio button connections
                            auto updateRegionStatus = [=]() {
                                if (regionalRadio->isChecked()) {
                                    if (m_imageLabel->isRegionSelected()) {
                                        QRect region = m_imageLabel->getSelectedRegion();
                                        regionStatusLabel->setText(
                                        QString("Selected Region: (%1,%2) - %3x%4")
                                        .arg(region.x())
                                        .arg(region.y())
                                        .arg(region.width())
                                        .arg(region.height()));
                                        regionStatusLabel->setStyleSheet("color: green; font-weight: bold;");
                                        regionInfoLabel->setVisible(false);
                                    } else {
                                        regionStatusLabel->setText("No region selected");
                                        regionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
                                        regionInfoLabel->setVisible(true);
                                    }
                                    regionStatusLabel->setVisible(true);
                                } else {
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                }
                            };

                            // Connect radio buttons to update control visibility
                            connect(overallRadio, &QRadioButton::toggled, [&dialog, regionStatusLabel, regionInfoLabel, thresholdWidget](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(false);
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                    dialog.adjustSize();
                                }
                            });

                            connect(regionalRadio, &QRadioButton::toggled, [&dialog, thresholdWidget, updateRegionStatus](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(false);
                                    updateRegionStatus();
                                    dialog.adjustSize();
                                }
                            });

                            connect(thresholdRadio, &QRadioButton::toggled, [&dialog, regionStatusLabel, regionInfoLabel, thresholdWidget](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(true);
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                    dialog.adjustSize();
                                }
                            });

                            // Create and add button box
                            QDialogButtonBox* buttonBox = new QDialogButtonBox(
                            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                            layout->addWidget(buttonBox);

                            connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                            connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                            if (dialog.exec() == QDialog::Accepted) {
                                // Process the adjustment
                                float contrastFactor = contrastSpinBox->value();

                                try {
                                    const auto& finalImage = m_imageProcessor.getFinalImage();
                                    if (!finalImage) {
                                        QMessageBox::warning(this, "Error", "No image data available");
                                        return;
                                    }

                                    int height = m_imageProcessor.getFinalImageHeight();
                                    int width = m_imageProcessor.getFinalImageWidth();

                                    // Allocate memory
                                    double** imgData = nullptr;
                                    malloc2D(imgData, height, width);

                                    // Copy data
                                    for (int y = 0; y < height; y++) {
                                        for (int x = 0; x < width; x++) {
                                            imgData[y][x] = finalImage[y][x];
                                        }
                                    }

                                    QString modeStr;
                                    QString paramStr;

                                    if (thresholdRadio->isChecked()) {
                                        float threshold = thresholdSpinBox->value();
                                        ImageAdjustments::adjustContrastWithThreshold(imgData, height, width, contrastFactor, threshold);
                                        modeStr = "Threshold";
                                        paramStr = QString("Factor: %1, Threshold: %2")
                                        .arg(contrastFactor, 0, 'f', 2)
                                        .arg(threshold);
                                    }
                                    else if (regionalRadio->isChecked()) {
                                        if (!m_imageLabel->isRegionSelected()) {
                                            // Clean up allocated memory
                                            for (int i = 0; i < height; i++) {
                                                free(imgData[i]);
                                            }
                                            free(imgData);

                                            QMessageBox::information(this, "Region Selection",
                                            "Please select a region first, then try again.");
                                            return;
                                        }

                                        QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                        ImageAdjustments::applyContrastToRegion(
                                        imgData, height, width, contrastFactor, selectedRegion);

                                        modeStr = "Regional";
                                        paramStr = QString("Factor: %1\nRegion: (%2,%3,%4,%5)")
                                        .arg(contrastFactor, 0, 'f', 2)
                                        .arg(selectedRegion.x())
                                        .arg(selectedRegion.y())
                                        .arg(selectedRegion.width())
                                        .arg(selectedRegion.height());
                                    }
                                    else {
                                        ImageAdjustments::adjustContrast(imgData, height, width, contrastFactor);
                                        modeStr = "Overall";
                                        paramStr = QString("Factor: %1").arg(contrastFactor, 0, 'f', 2);
                                    }

                                    // Update image
                                    m_imageProcessor.updateAndSaveFinalImage(imgData, height, width);

                                    // Clean up
                                    for (int i = 0; i < height; i++) {
                                        free(imgData[i]);
                                    }
                                    free(imgData);

                                    // Update display
                                    m_imageLabel->clearSelection();
                                    updateImageDisplay();
                                    updateLastAction("Contrast Adjustment",
                                    QString("Mode: %1\n%2").arg(modeStr).arg(paramStr));

                                } catch (const std::exception& e) {
                                    QMessageBox::critical(this, "Error",
                                    QString("Failed to apply contrast adjustment: %1").arg(e.what()));
                                }
                            }
                        }},
                       {"Sharpen Adjustment", [this]() {
                            if (checkZoomMode()) return;
                            m_imageProcessor.saveCurrentState();
                            m_darkLineInfoLabel->hide();
                            resetDetectedLinesPointer();

                            QDialog dialog(this);
                            dialog.setWindowTitle("Sharpen Adjustment");
                            dialog.setMinimumWidth(350);

                            QVBoxLayout* layout = new QVBoxLayout(&dialog);

                            // Mode selection
                            QGroupBox* modeBox = new QGroupBox("Adjustment Mode");
                            QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);

                            QRadioButton* overallRadio = new QRadioButton("Overall Adjustment");
                            QRadioButton* regionalRadio = new QRadioButton("Regional Adjustment");
                            QRadioButton* thresholdRadio = new QRadioButton("Threshold-based Adjustment");

                            overallRadio->setChecked(true);

                            // Add explanatory labels
                            QLabel* overallLabel = new QLabel("Apply sharpening to the entire image");
                            QLabel* regionalLabel = new QLabel("Apply sharpening to a selected region only");
                            QLabel* thresholdLabel = new QLabel("Apply sharpening to pixels below threshold only");

                            overallLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            regionalLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");
                            thresholdLabel->setStyleSheet("color: gray; font-size: 10px; margin-left: 20px;");

                            modeLayout->addWidget(overallRadio);
                            modeLayout->addWidget(overallLabel);
                            modeLayout->addWidget(regionalRadio);
                            modeLayout->addWidget(regionalLabel);
                            modeLayout->addWidget(thresholdRadio);
                            modeLayout->addWidget(thresholdLabel);

                            layout->addWidget(modeBox);

                            // Parameter inputs group
                            QGroupBox* paramBox = new QGroupBox("Parameters");
                            QVBoxLayout* paramLayout = new QVBoxLayout(paramBox);

                            // Define spinbox style
                            QString spinBoxStyle =
                            "QSpinBox, QDoubleSpinBox {"
                            "    background-color: white;"
                            "    border: 1px solid #c0c0c0;"
                            "    border-radius: 3px;"
                            "    padding: 1px;"
                            "    min-width: 100px;"
                            "}"
                            "QSpinBox:focus, QDoubleSpinBox:focus {"
                            "    border: 1px solid #0078d7;"
                            "}"
                            "QSpinBox::up-button, QSpinBox::down-button,"
                            "QDoubleSpinBox::up-button, QDoubleSpinBox::down-button {"
                            "    width: 0px;"
                            "    border: none;"
                            "}";

                            // Sharpen strength input
                            QLabel* strengthLabel = new QLabel("Sharpening Strength:");
                            QDoubleSpinBox* strengthSpinBox = new QDoubleSpinBox();
                            strengthSpinBox->setRange(0.01, 3.0);
                            strengthSpinBox->setValue(1.0);
                            strengthSpinBox->setDecimals(2);
                            strengthSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
                            strengthSpinBox->setStyleSheet(spinBoxStyle);
                            strengthSpinBox->setToolTip("Enter strength value (0.01-3.00)");

                            paramLayout->addWidget(strengthLabel);
                            paramLayout->addWidget(strengthSpinBox);

                            // Threshold value input
                            QWidget* thresholdWidget = new QWidget();
                            QVBoxLayout* thresholdLayout = new QVBoxLayout(thresholdWidget);

                            QLabel* thresholdValueLabel = new QLabel("Threshold Value:");
                            QSpinBox* thresholdSpinBox = new QSpinBox();
                            thresholdSpinBox->setRange(0, 65535);
                            thresholdSpinBox->setValue(32767);
                            thresholdSpinBox->setButtonSymbols(QAbstractSpinBox::NoButtons);
                            thresholdSpinBox->setStyleSheet(spinBoxStyle);
                            thresholdSpinBox->setToolTip("Enter threshold value (0-65535)");

                            thresholdLayout->addWidget(thresholdValueLabel);
                            thresholdLayout->addWidget(thresholdSpinBox);
                            thresholdWidget->setVisible(false);

                            paramLayout->addWidget(thresholdWidget);

                            // Region selection status and info
                            QLabel* regionStatusLabel = new QLabel();
                            QLabel* regionInfoLabel = new QLabel(
                            "Please select a region in the image before applying the adjustment.\n"
                            "Click and drag to define the region of interest.");

                            regionStatusLabel->setStyleSheet("font-weight: bold;");
                            regionInfoLabel->setStyleSheet("color: #666666; font-size: 10px; padding: 5px;");
                            regionInfoLabel->setWordWrap(true);

                            regionStatusLabel->setVisible(false);
                            regionInfoLabel->setVisible(false);

                            paramLayout->addWidget(regionStatusLabel);
                            paramLayout->addWidget(regionInfoLabel);

                            layout->addWidget(paramBox);

                            // Update region status when selection changes
                            auto updateRegionStatus = [=]() {
                                if (regionalRadio->isChecked()) {
                                    if (m_imageLabel->isRegionSelected()) {
                                        QRect region = m_imageLabel->getSelectedRegion();
                                        regionStatusLabel->setText(
                                        QString("Selected Region: (%1,%2) - %3x%4")
                                        .arg(region.x())
                                        .arg(region.y())
                                        .arg(region.width())
                                        .arg(region.height()));
                                        regionStatusLabel->setStyleSheet("color: green; font-weight: bold;");
                                        regionInfoLabel->setVisible(false);
                                    } else {
                                        regionStatusLabel->setText("No region selected");
                                        regionStatusLabel->setStyleSheet("color: red; font-weight: bold;");
                                        regionInfoLabel->setVisible(true);
                                    }
                                    regionStatusLabel->setVisible(true);
                                } else {
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                }
                            };

                            // Connect radio buttons to update control visibility
                            connect(overallRadio, &QRadioButton::toggled, [&dialog, regionStatusLabel, regionInfoLabel, thresholdWidget](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(false);
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                    dialog.adjustSize();
                                }
                            });

                            connect(regionalRadio, &QRadioButton::toggled, [&dialog, thresholdWidget, updateRegionStatus](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(false);
                                    updateRegionStatus();
                                    dialog.adjustSize();
                                }
                            });

                            connect(thresholdRadio, &QRadioButton::toggled, [&dialog, regionStatusLabel, regionInfoLabel, thresholdWidget](bool checked) {
                                if (checked) {
                                    thresholdWidget->setVisible(true);
                                    regionStatusLabel->setVisible(false);
                                    regionInfoLabel->setVisible(false);
                                    dialog.adjustSize();
                                }
                            });

                            // Create and add button box
                            QDialogButtonBox* buttonBox = new QDialogButtonBox(
                            QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                            layout->addWidget(buttonBox);

                            connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                            connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                            // Initial status update
                            updateRegionStatus();

                            if (dialog.exec() == QDialog::Accepted) {
                                float strength = strengthSpinBox->value();

                                try {
                                    const auto& finalImage = m_imageProcessor.getFinalImage();
                                    if (!finalImage) {
                                        QMessageBox::warning(this, "Error", "No image data available");
                                        return;
                                    }

                                    int height = m_imageProcessor.getFinalImageHeight();
                                    int width = m_imageProcessor.getFinalImageWidth();

                                    // Allocate memory
                                    double** imgData = nullptr;
                                    malloc2D(imgData, height, width);

                                    // Copy data
                                    for (int y = 0; y < height; y++) {
                                        for (int x = 0; x < width; x++) {
                                            imgData[y][x] = finalImage[y][x];
                                        }
                                    }

                                    QString modeStr;
                                    QString paramStr;

                                    if (thresholdRadio->isChecked()) {
                                        float threshold = thresholdSpinBox->value();
                                        ImageAdjustments::sharpenWithThreshold(imgData, height, width, strength, threshold);
                                        modeStr = "Threshold";
                                        paramStr = QString("Strength: %1, Threshold: %2")
                                        .arg(strength, 0, 'f', 2)
                                        .arg(threshold);
                                    }
                                    else if (regionalRadio->isChecked()) {
                                        if (!m_imageLabel->isRegionSelected()) {
                                            // Clean up allocated memory
                                            for (int i = 0; i < height; i++) {
                                                free(imgData[i]);
                                            }
                                            free(imgData);

                                            QMessageBox::information(this, "Region Selection",
                                            "Please select a region first, then try again.");
                                            return;
                                        }

                                        QRect selectedRegion = m_imageLabel->getSelectedRegion();
                                        ImageAdjustments::applySharpenToRegion(
                                        imgData, height, width, strength, selectedRegion);

                                        modeStr = "Regional";
                                        paramStr = QString("Strength: %1\nRegion: (%2,%3,%4,%5)")
                                        .arg(strength, 0, 'f', 2)
                                        .arg(selectedRegion.x())
                                        .arg(selectedRegion.y())
                                        .arg(selectedRegion.width())
                                        .arg(selectedRegion.height());
                                    }
                                    else {
                                        ImageAdjustments::sharpenImage(imgData, height, width, strength);
                                        modeStr = "Overall";
                                        paramStr = QString("Strength: %1").arg(strength, 0, 'f', 2);
                                    }

                                    // Update image
                                    m_imageProcessor.updateAndSaveFinalImage(imgData, height, width);

                                    // Clean up
                                    for (int i = 0; i < height; i++) {
                                        free(imgData[i]);
                                    }
                                    free(imgData);

                                    // Update display
                                    m_imageLabel->clearSelection();
                                    updateImageDisplay();
                                    updateLastAction("Sharpen Adjustment",
                                    QString("Mode: %1\n%2").arg(modeStr).arg(paramStr));

                                } catch (const std::exception& e) {
                                    QMessageBox::critical(this, "Error",
                                    QString("Failed to apply sharpening: %1").arg(e.what()));
                                }
                            }
                        }}
                   });
}

void ControlPanel::setupGraph() {

    // Create histogram first
    m_histogram = new GraphProcessor(this);
    m_histogram->setMinimumWidth(250);
    m_histogram->setVisible(false);

    createGroupBox("Graph Operations", {
                       {"Histogram", [this]() {
                            bool isCurrentlyVisible = m_histogram->isVisible();
                            m_histogram->setVisible(!isCurrentlyVisible);

                            if (!isCurrentlyVisible) {
                                const auto& finalImage = m_imageProcessor.getFinalImage();
                                int height = m_imageProcessor.getFinalImageHeight();
                                int width = m_imageProcessor.getFinalImageWidth();

                                if (finalImage) {
                                    m_histogram->updateHistogram(finalImage, height, width);
                                }
                            }

                            m_mainLayout->invalidate();
                            updateGeometry();
                        }}
                   });

    // Find the Graph Operations group box and add histogram to it
    for (int i = 0; i < m_scrollLayout->count(); ++i) {
        QGroupBox* box = qobject_cast<QGroupBox*>(m_scrollLayout->itemAt(i)->widget());
        if (box && box->title() == "Graph Operations") {
            box->layout()->addWidget(m_histogram);
            break;
        }
    }

    // Connect fileLoaded signal to enable buttons
    connect(this, &ControlPanel::fileLoaded, this, [this](bool loaded) {
        for (QPushButton* button : m_allButtons) {
            if (button && (button->text() == "Histogram" ||
                           button->text() == "3D Graph" ||
                           button->text() == "Toggle CLAHE View")) {
                button->setEnabled(loaded);
            }
        }
    });
}

void ControlPanel::processCalibration(int linesToProcessY, int linesToProcessX, CalibrationMode mode) {
    bool hasStoredParams = InterlaceProcessor::hasCalibrationParams();
    int storedY = hasStoredParams ? InterlaceProcessor::getStoredYParam() : 0;
    int storedX = hasStoredParams ? InterlaceProcessor::getStoredXParam() : 0;

    int newY = linesToProcessY;
    int newX = linesToProcessX;
    QString actionDescription;

    try {
        // Get current dimensions
        int height = m_imageProcessor.getFinalImageHeight();
        int width = m_imageProcessor.getFinalImageWidth();
        const double** currentImage = const_cast<const double**>(m_imageProcessor.getFinalImage());

        if (!currentImage || height <= 0 || width <= 0) {
            throw std::runtime_error("Invalid image data for calibration");
        }

        // Create a working copy of the image
        double** workingImage = nullptr;
        malloc2D(workingImage, height, width);

        // Copy data
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                workingImage[y][x] = currentImage[y][x];
            }
        }

        switch (mode) {
        case CalibrationMode::Y_AXIS_ONLY:
            if (hasStoredParams && storedY > 0) {
                newY = storedY;
                actionDescription = QString("Y-Axis (Used stored: %1 lines)").arg(newY);
            } else {
                auto result = showInputDialog("Y-Axis Calibration", "Enter lines to process for Y-axis:", 10, 1, 1000);
                if (!result.second) {
                    // Clean up on cancel
                    for (int i = 0; i < height; i++) {
                        free(workingImage[i]);
                    }
                    free(workingImage);
                    return;
                }
                newY = result.first;
                actionDescription = QString("Y-Axis (New: %1 lines)").arg(newY);
            }
            newX = 0; // Don't use X-axis for Y-only calibration
            break;

        case CalibrationMode::X_AXIS_ONLY:
            if (hasStoredParams && storedX > 0) {
                newX = storedX;
                actionDescription = QString("X-Axis (Used stored: %1 lines)").arg(newX);
            } else {
                auto result = showInputDialog("X-Axis Calibration", "Enter lines to process for X-axis:", 10, 1, 1000);
                if (!result.second) {
                    // Clean up on cancel
                    for (int i = 0; i < height; i++) {
                        free(workingImage[i]);
                    }
                    free(workingImage);
                    return;
                }
                newX = result.first;
                actionDescription = QString("X-Axis (New: %1 lines)").arg(newX);
            }
            newY = 0; // Don't use Y-axis for X-only calibration
            break;

        case CalibrationMode::BOTH_AXES:
            // Y-axis input
            if (hasStoredParams && storedY > 0) {
                newY = storedY;
                actionDescription = QString("Y:%1 (Stored)").arg(newY);
            } else {
                auto resultY = showInputDialog("Y-Axis Calibration", "Enter lines to process for Y-axis:", 10, 1, 1000);
                if (!resultY.second) {
                    // Clean up on cancel
                    for (int i = 0; i < height; i++) {
                        free(workingImage[i]);
                    }
                    free(workingImage);
                    return;
                }
                newY = resultY.first;
                actionDescription = QString("Y:%1 (New)").arg(newY);
            }

            // X-axis input
            QString xDescription;
            if (hasStoredParams && storedX > 0) {
                newX = storedX;
                xDescription = QString("X:%1 (Stored)").arg(newX);
            } else {
                auto resultX = showInputDialog("X-Axis Calibration", "Enter lines to process for X-axis:", 10, 1, 1000);
                if (!resultX.second) {
                    // Clean up on cancel
                    for (int i = 0; i < height; i++) {
                        free(workingImage[i]);
                    }
                    free(workingImage);
                    return;
                }
                newX = resultX.first;
                xDescription = QString("X:%1 (New)").arg(newX);
            }
            actionDescription = QString("Both Axes (%1, %2)").arg(actionDescription).arg(xDescription);
            break;
        }

        // Y-axis processing
        if (mode == CalibrationMode::Y_AXIS_ONLY || mode == CalibrationMode::BOTH_AXES) {
            if (newY > 0) {
                std::vector<double> referenceYMean(width, 0.0);

                // Calculate mean for reference lines
                for (int x = 0; x < width; ++x) {
                    double sum = 0.0;
                    int count = 0;
                    for (int y = 0; y < std::min(newY, height); ++y) {
                        sum += workingImage[y][x];
                        count++;
                    }
                    referenceYMean[x] = count > 0 ? (sum / count) : 1.0;
                }

                // Apply normalization
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (referenceYMean[x] > 0) {
                            double normalizedValue = workingImage[y][x] / referenceYMean[x];
                            workingImage[y][x] = std::clamp(normalizedValue * 65535.0, 0.0, 65535.0);
                        }
                    }
                }
            }
        }

        // X-axis processing
        if (mode == CalibrationMode::X_AXIS_ONLY || mode == CalibrationMode::BOTH_AXES) {
            if (newX > 0) {
                std::vector<double> referenceXMean(height, 0.0);

                // Calculate mean for reference lines
                for (int y = 0; y < height; ++y) {
                    double sum = 0.0;
                    int count = 0;
                    for (int x = width - newX; x < width; ++x) {
                        sum += workingImage[y][x];
                        count++;
                    }
                    referenceXMean[y] = count > 0 ? (sum / count) : 1.0;
                }

                // Apply normalization
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (referenceXMean[y] > 0) {
                            double normalizedValue = workingImage[y][x] / referenceXMean[y];
                            workingImage[y][x] = std::clamp(normalizedValue * 65535.0, 0.0, 65535.0);
                        }
                    }
                }
            }
        }

        // Update the image processor with the modified image
        m_imageProcessor.updateAndSaveFinalImage(workingImage, height, width);

        updateImageDisplay();

        QString paramString = QString("Mode: %1\nParameters: Y=%2, X=%3")
                .arg(actionDescription)
                .arg(newY)
                .arg(newX);

        updateLastAction("Axis Calibration", paramString);

        InterlaceProcessor::setCalibrationParams(newY, newX);

        if (newY > 0 || newX > 0) {
            InterlaceProcessor::setCalibrationParams(newY, newX);
            m_resetCalibrationButton->setEnabled(true);
            m_resetCalibrationButton->setVisible(true);
        }

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Calibration Error",
                              QString("Failed to apply calibration: %1").arg(e.what()));
    }
}

void ControlPanel::drawLineLabelWithCountPointer(QPainter& painter, const DarkLine& line, int count, float zoomLevel, const QSize& imageSize) {
    // 基础安全检查
    if (!imageSize.isValid() || count < 0) {
        qDebug() << "Invalid parameters in drawLineLabelWithCountPointer";
        return;
    }

    // 准备标签文本
    QString labelText = QString("Line %1 - %2")
            .arg(count + 1)
            .arg(line.inObject ? "In Object" : "Isolated");

    // 计算标签位置，确保不会超出图像边界
    const int labelMargin = 10;
    const int labelSpacing = 30;

    int labelX, labelY;

    if (line.isVertical) {
        // 垂直线的标签位置
        labelX = qBound(labelMargin,
                        static_cast<int>(line.x * zoomLevel + labelMargin),
                        imageSize.width() - 150); // 确保有足够空间显示文本
        labelY = qBound(labelMargin,
                        static_cast<int>(labelSpacing + count * labelSpacing),
                        imageSize.height() - 50);
    } else {
        // 水平线的标签位置
        labelX = qBound(labelMargin,
                        labelMargin + (count * 100), // 横向分散标签
                        imageSize.width() - 150);
        labelY = qBound(labelMargin,
                        static_cast<int>(line.y * zoomLevel + labelMargin),
                        imageSize.height() - 50);
    }

    // 获取文本大小
    QFontMetrics fm(painter.font());
    QRect textRect = fm.boundingRect(labelText);

    // 调整文本框位置，确保完全在图像内
    textRect.moveTopLeft(QPoint(labelX, labelY));

    // 添加边距
    textRect.adjust(-5, -2, 5, 2);

    // 确保文本框完全在图像范围内
    if (textRect.right() > imageSize.width()) {
        textRect.moveLeft(imageSize.width() - textRect.width() - 10);
    }
    if (textRect.bottom() > imageSize.height()) {
        textRect.moveTop(imageSize.height() - textRect.height() - 10);
    }

    // 绘制背景
    painter.setPen(QPen(QColor(0, 0, 0, 160), 1));
    painter.setBrush(QColor(255, 255, 255, 230));
    painter.drawRect(textRect);

    // 绘制文本
    painter.setPen(Qt::black);
    painter.drawText(textRect, Qt::AlignCenter, labelText);
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

bool ControlPanel::initializeImageData(ImageData& imageData, int& height, int& width) {
    const auto& currentImage = m_imageProcessor.getFinalImage();
    if (!currentImage) {
        QMessageBox::warning(this, "Error", "No image data available");
        return false;
    }

    height = m_imageProcessor.getFinalImageHeight();
    width = m_imageProcessor.getFinalImageWidth();

    // Convert to double pointer format
    imageData.data = new double*[height];
    for (int y = 0; y < height; y++) {
        imageData.data[y] = new double[width];
        for (int x = 0; x < width; x++) {
            imageData.data[y][x] = currentImage[y][x];
        }
    }

    imageData.rows = height;
    imageData.cols = width;
    return true;
}

void ControlPanel::cleanupImageData(ImageData& imageData) {
    if (imageData.data) {
        for (int i = 0; i < imageData.rows; i++) {
            delete[] imageData.data[i];
        }
        delete[] imageData.data;
        imageData.data = nullptr;
    }
    imageData.rows = 0;
    imageData.cols = 0;
}

bool ControlPanel::processDetectedLines(DarkLineArray* outLines, bool success) {
    if (success && outLines && outLines->rows > 0) {
        // Clean up old detection results
        resetDetectedLinesPointer();

        // Set new detection results
        m_detectedLinesPointer = outLines;

        // Show reset detection button
        for (QPushButton* button : m_allButtons) {
            if (button && button->text() == "Reset Detection") {
                button->setVisible(true);
                button->setEnabled(true);
                break;
            }
        }

        return true;
    } else {
        if (outLines) {
            DarkLinePointerProcessor::destroyDarkLineArray(outLines);
        }
        return false;
    }
}

void ControlPanel::updateDarkLineDisplay(const QString& detectionInfo) {
    if (m_darkLineInfoLabel) {
        m_darkLineInfoLabel->setText(detectionInfo);
        m_darkLineInfoLabel->setVisible(true);
        updateDarkLineInfoDisplayPointer();
    }
}

void ControlPanel::handleDetectionError(const std::exception& e, DarkLineArray* outLines) {
    if (outLines) {
        DarkLinePointerProcessor::destroyDarkLineArray(outLines);
    }
    QMessageBox::critical(this, "Error", QString("Error in line detection: %1").arg(e.what()));
}

ImageData ControlPanel::convertToImageData(double** image, int height, int width) {
    ImageData imageData;
    if (!image || height <= 0 || width <= 0) {
        throw std::runtime_error("Invalid image provided");
    }

    imageData.rows = height;
    imageData.cols = width;
    imageData.data = new double*[imageData.rows];

    for (int i = 0; i < imageData.rows; i++) {
        imageData.data[i] = new double[imageData.cols];
        for (int j = 0; j < imageData.cols; j++) {
            imageData.data[i][j] = image[i][j];
        }
    }

    return imageData;
}

double** ControlPanel::convertFromImageData(const ImageData& imageData) {
    if (!imageData.data || imageData.rows <= 0 || imageData.cols <= 0) {
        throw std::runtime_error("Invalid image data");
    }

    double** result = new double*[imageData.rows];
    for (int i = 0; i < imageData.rows; i++) {
        result[i] = new double[imageData.cols];
        for (int j = 0; j < imageData.cols; j++) {
            result[i][j] = std::clamp(imageData.data[i][j], 0.0, 65535.0);
        }
    }

    return result;
}

// Helper method for cleanup
void ControlPanel::cleanupImageArray(double** array, int rows) {
    if (array) {
        for (int i = 0; i < rows; i++) {
            delete[] array[i];
        }
        delete[] array;
    }
}

