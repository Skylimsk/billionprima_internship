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
    qDebug() << "General enable state:" << enable;
    qDebug() << "Current history size:" << m_imageProcessor.getHistorySize();
    qDebug() << "Can undo:" << m_imageProcessor.canUndo();

    // Check if zoom mode is active
    bool zoomModeActive = m_imageProcessor.getZoomManager().isZoomModeActive();
    bool zoomFixed = m_imageProcessor.getZoomManager().isZoomFixed();

    // Process each button based on its type and current application state
    for (QPushButton* button : m_allButtons) {
        if (!button) continue;

        QString buttonText = button->text();
        bool shouldEnable = false;

        // Special case handling for different button types
        if (buttonText == "Browse") {
            // Browse is enabled when no file is loaded
            shouldEnable = !enable;
        }
        else if (buttonText == "Undo") {
            // Undo is enabled only when there's history and not in unfixed zoom mode
            shouldEnable = enable || m_imageProcessor.canUndo() || (!zoomModeActive || zoomFixed);
            qDebug() << "Setting Undo button state:" << shouldEnable;
        }
        else if (buttonText == "Activate Zoom" || buttonText == "Deactivate Zoom") {
            // Zoom activation button follows file loaded state
            shouldEnable = enable;

            // Update button appearance
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
                    "QPushButton:hover {"
                    "    background-color: #f3f4f6;"
                    "}"
                    // State-specific styles for deactivate mode
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
        else if (buttonText == "Zoom In" || buttonText == "Zoom Out" ||
                 buttonText == "Reset Zoom" || buttonText == "Fix Zoom" ||
                 buttonText == "Unfix Zoom") {
            // Zoom control buttons are enabled in zoom mode and not fixed
            shouldEnable = enable && zoomModeActive && !zoomFixed;
            if (buttonText == "Fix Zoom" || buttonText == "Unfix Zoom") {
                shouldEnable = enable && zoomModeActive;
            }
        }
        else {
            // All other operation buttons
            if (zoomModeActive && !zoomFixed) {
                // Disable operations during unfixed zoom mode
                shouldEnable = false;
            } else {
                // Enable based on file loaded state
                shouldEnable = enable;
            }
        }

        // Apply the calculated state
        button->setEnabled(shouldEnable);
    }

    // Update histogram buttons if present
    if (m_histogram) {
        bool histogramEnabled = enable && (!zoomModeActive || zoomFixed);
        for (QPushButton* button : m_allButtons) {
            if (button && (button->text() == "Histogram" || button->text() == "Toggle CLAHE View")) {
                button->setEnabled(histogramEnabled);
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
        m_pixelInfoLabel->setText("No image loaded");
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
        "    background-color: #f3f4f6;"  // Matching mockup gray background
        "    border: 1px solid #e5e7eb;"
        "    border-radius: 4px;"
        "    padding: 5px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #e5e7eb;"
        "}";

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

    if (active) {
        // When activating zoom
        zoomManager.toggleZoomMode(true);
        m_zoomButton->setText("Deactivate Zoom");
        // Set state based on whether zoom is fixed
        m_zoomButton->setProperty("state",
                                  m_fixZoomButton->isChecked() ? "deactivate-fix" : "deactivate-unfix");
    } else {
        // When deactivating zoom
        zoomManager.toggleZoomMode(false);
        m_zoomButton->setText("Activate Zoom");
        m_zoomButton->setProperty("state", "");  // Remove state property
    }

    // Force style refresh
    m_zoomButton->style()->unpolish(m_zoomButton);
    m_zoomButton->style()->polish(m_zoomButton);

    if (active && std::abs(zoomManager.getZoomLevel() - 1.0f) > 0.001f) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Zoom Level Reminder");
        msgBox.setText(QString("Zoom mode will be activated at %1x zoom level.")
                           .arg(zoomManager.getZoomLevel(), 0, 'f', 2));
        msgBox.setIcon(QMessageBox::Information);

        // Add custom buttons
        QPushButton* okButton = msgBox.addButton(QMessageBox::Ok);
        QPushButton* resetButton = msgBox.addButton("Reset", QMessageBox::ActionRole);

        msgBox.exec();

        if (msgBox.clickedButton() == resetButton) {
            zoomManager.resetZoom();
            m_fixZoomButton->setChecked(false);
            zoomManager.toggleFixedZoom(false);
            updateImageDisplay();
            updateLastAction("Reset Zoom", "1x");
        }
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
                                               // 创建格式选择对话框,优化布局
                                               QDialog formatDialog(this);
                                               formatDialog.setWindowTitle("Select File Type");
                                               formatDialog.setMinimumWidth(400);
                                               QVBoxLayout* mainLayout = new QVBoxLayout(&formatDialog);

                                               // 添加头部标签
                                               QLabel* headerLabel = new QLabel("Select the type of file to load:");
                                               headerLabel->setStyleSheet("font-weight: bold; margin-bottom: 10px;");
                                               mainLayout->addWidget(headerLabel);

                                               // 创建单选按钮组
                                               QGroupBox* formatGroup = new QGroupBox("File Format");
                                               QVBoxLayout* formatLayout = new QVBoxLayout(formatGroup);

                                               // 创建并设置单选按钮样式
                                               QRadioButton* loadImageBtn = new QRadioButton("Image File Format");
                                               QRadioButton* loadTextBtn = new QRadioButton("Text File Format");
                                               loadImageBtn->setChecked(true);

                                               formatLayout->addWidget(loadImageBtn);
                                               formatLayout->addWidget(loadTextBtn);
                                               mainLayout->addWidget(formatGroup);

                                               // 文本文件选项组
                                               QGroupBox* textOptionsGroup = new QGroupBox("Text File Options");
                                               QVBoxLayout* textOptionsLayout = new QVBoxLayout(textOptionsGroup);

                                               QRadioButton* load2DTextBtn = new QRadioButton("2D Format");
                                               QRadioButton* load1DTextBtn = new QRadioButton("1D Format");
                                               load2DTextBtn->setChecked(true);

                                               textOptionsLayout->addWidget(load2DTextBtn);
                                               textOptionsLayout->addWidget(load1DTextBtn);

                                               QLabel* textFormatInfo = new QLabel("Format descriptions:");
                                               textFormatInfo->setStyleSheet("color: #666666; margin-top: 10px;");
                                               QLabel* formatDescriptions = new QLabel(
                                                   "• 2D Format: Text file with values arranged in a grid\n"
                                                   "• 1D Format: Text file with values in a single column"
                                                   );
                                               formatDescriptions->setStyleSheet("margin-left: 20px; color: #666666;");

                                               textOptionsLayout->addWidget(textFormatInfo);
                                               textOptionsLayout->addWidget(formatDescriptions);
                                               mainLayout->addWidget(textOptionsGroup);

                                               // 图像文件选项组
                                               QGroupBox* imageOptionsGroup = new QGroupBox("Image File Options");
                                               QVBoxLayout* imageOptionsLayout = new QVBoxLayout(imageOptionsGroup);

                                               QLabel* imageFormatLabel = new QLabel("Supported formats:");
                                               imageFormatLabel->setStyleSheet("color: #666666;");
                                               QLabel* imageFormatsSupported = new QLabel("• PNG (*.png)\n• JPEG (*.jpg, *.jpeg)\n• TIFF (*.tiff, *.tif)\n• BMP (*.bmp)");
                                               imageFormatsSupported->setStyleSheet("margin-left: 20px; color: #666666;");

                                               imageOptionsLayout->addWidget(imageFormatLabel);
                                               imageOptionsLayout->addWidget(imageFormatsSupported);
                                               mainLayout->addWidget(imageOptionsGroup);

                                               // 控制选项组的启用/禁用状态
                                               textOptionsGroup->setEnabled(false);
                                               connect(loadImageBtn, &QRadioButton::toggled, [&](bool checked) {
                                                   imageOptionsGroup->setEnabled(checked);
                                                   textOptionsGroup->setEnabled(!checked);
                                               });
                                               connect(loadTextBtn, &QRadioButton::toggled, [&](bool checked) {
                                                   textOptionsGroup->setEnabled(checked);
                                                   imageOptionsGroup->setEnabled(!checked);
                                               });

                                               // 添加确定和取消按钮
                                               QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                   QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                               mainLayout->addWidget(buttonBox);

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
                                                           resetDetectedLinesPointer();
                                                           m_darkLineInfoLabel->hide();

                                                           QString loadType;
                                                           if (loadImageBtn->isChecked()) {
                                                               loadType = "Image";
                                                           } else {
                                                               loadType = load2DTextBtn->isChecked() ? "2D Text" : "1D Text";
                                                           }
                                                           auto startLoad = std::chrono::high_resolution_clock::now();
                                                           if (loadImageBtn->isChecked()) {
                                                               m_imageProcessor.loadImage(fileName.toStdString());
                                                           } else {
                                                               if (load2DTextBtn->isChecked()) {
                                                                   double** matrix = nullptr;
                                                                   int rows = 0, cols = 0;
                                                                   ImageReader::ReadTextToU2D(fileName.toStdString(), matrix, rows, cols);

                                                                   if (matrix && rows > 0 && cols > 0) {
                                                                       // Convert to double** since that's what updateAndSaveFinalImage expects
                                                                       double** finalMatrix = new double*[rows];
                                                                       for (int i = 0; i < rows; ++i) {
                                                                           finalMatrix[i] = new double[cols];
                                                                           for (int j = 0; j < cols; ++j) {
                                                                               finalMatrix[i][j] = std::min(65535.0, std::max(0.0, matrix[i][j]));
                                                                           }
                                                                       }

                                                                       // Store the original image first
                                                                       m_imageProcessor.setOriginalImage(finalMatrix, rows, cols);

                                                                       // Update the working image
                                                                       m_imageProcessor.updateAndSaveFinalImage(finalMatrix, rows, cols);

                                                                       // Clean up both matrices
                                                                       for (int i = 0; i < rows; ++i) {
                                                                           delete[] matrix[i];
                                                                           delete[] finalMatrix[i];
                                                                       }
                                                                       delete[] matrix;
                                                                       delete[] finalMatrix;
                                                                   }
                                                               }else {
                                                                   // 1D text handling
                                                                   uint32_t* matrix = nullptr;
                                                                   int rows = 0, cols = 0;
                                                                   ImageReader::ReadTextToU1D(fileName.toStdString(), matrix, rows, cols);

                                                                   if (matrix && rows > 0 && cols > 0) {
                                                                       // Convert to double** since that's what updateAndSaveFinalImage expects
                                                                       double** finalMatrix = new double*[rows];
                                                                       for (int i = 0; i < rows; ++i) {
                                                                           finalMatrix[i] = new double[cols];
                                                                           for (int j = 0; j < cols; ++j) {
                                                                               finalMatrix[i][j] = std::min(65535.0, std::max(0.0, static_cast<double>(matrix[i * cols + j])));
                                                                           }
                                                                       }

                                                                       // Store the original image first
                                                                       m_imageProcessor.setOriginalImage(finalMatrix, rows, cols);

                                                                       // Update the working image
                                                                       m_imageProcessor.updateAndSaveFinalImage(finalMatrix, rows, cols);

                                                                       // Clean up
                                                                       delete[] matrix;
                                                                       for (int i = 0; i < rows; ++i) {
                                                                           delete[] finalMatrix[i];
                                                                       }
                                                                       delete[] finalMatrix;
                                                                   }
                                                               }
                                                           }

                                                           auto endLoad = std::chrono::high_resolution_clock::now();
                                                           m_fileLoadTime = std::chrono::duration<double, std::milli>(endLoad - startLoad).count();

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
                                                           // Enable Browse button again if loading fails
                                                           emit fileLoaded(false);
                                                       }
                                                   }
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
            "QPushButton:hover {"
            "    background-color: #f3f4f6;"
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
            "QPushButton:hover {"
            "    background-color: #f3f4f6;"
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
            "QPushButton:hover {"
            "    background-color: #e5e7eb;"
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
            QRect normalizedRegion = selectedRegion.normalized();

            // Handle zoom mode
            auto& zoomManager = m_imageProcessor.getZoomManager();
            if (zoomManager.isZoomModeActive()) {
                float zoomLevel = zoomManager.getZoomLevel();
                // Convert zoomed coordinates back to original image coordinates
                normalizedRegion = QRect(
                    static_cast<int>(normalizedRegion.x() / zoomLevel),
                    static_cast<int>(normalizedRegion.y() / zoomLevel),
                    static_cast<int>(normalizedRegion.width() / zoomLevel),
                    static_cast<int>(normalizedRegion.height() / zoomLevel)
                    );
            }

            if (!normalizedRegion.isEmpty()) {
                try {
                    const auto& inputImage = m_imageProcessor.getFinalImage();
                    // Get dimensions using the getter methods instead of vector size
                    int inputHeight = m_imageProcessor.getFinalImageHeight();
                    int inputWidth = m_imageProcessor.getFinalImageWidth();

                    if (!inputImage || inputHeight <= 0 || inputWidth <= 0) {
                        QMessageBox::warning(this, "Crop Error", "Invalid input image.");
                        return;
                    }

                    // Convert vector to double pointer using malloc2D
                    double** inputPtr = nullptr;
                    malloc2D(inputPtr, inputHeight, inputWidth);

                    // Copy data to inputPtr
                    for (int y = 0; y < inputHeight; y++) {
                        for (int x = 0; x < inputWidth; x++) {
                            inputPtr[y][x] = inputImage[y][x];
                        }
                    }

                    // Perform crop operation
                    CGData croppedData = m_imageProcessor.cropRegion(
                        inputPtr, inputHeight, inputWidth,
                        normalizedRegion.left(), normalizedRegion.top(),
                        normalizedRegion.right(), normalizedRegion.bottom()
                        );

                    // Convert CGData to double** for updateAndSaveFinalImage
                    double** resultPtr = nullptr;
                    malloc2D(resultPtr, croppedData.Row, croppedData.Column);

                    for (int y = 0; y < croppedData.Row; y++) {
                        for (int x = 0; x < croppedData.Column; x++) {
                            resultPtr[y][x] = std::clamp(croppedData.Data[y][x], 0.0, 65535.0);
                        }
                    }

                    // Clean up allocated memory
                    for (int i = 0; i < inputHeight; i++) {
                        free(inputPtr[i]);
                    }
                    free(inputPtr);

                    for (int i = 0; i < croppedData.Row; i++) {
                        free(croppedData.Data[i]);
                    }
                    free(croppedData.Data);

                    // Update image with double** version
                    m_imageProcessor.updateAndSaveFinalImage(resultPtr, croppedData.Row, croppedData.Column);

                    // Clean up result pointer after updating
                    for (int i = 0; i < croppedData.Row; i++) {
                        free(resultPtr[i]);
                    }
                    free(resultPtr);

                    m_imageLabel->clearSelection();
                    updateImageDisplay();
                    updateLastAction("Crop");

                } catch (const std::exception& e) {
                    QMessageBox::critical(this, "Crop Error",
                                          QString("Failed to crop image: %1").arg(e.what()));
                }
            } else {
                QMessageBox::warning(this, "Crop Error",
                                     "Invalid region selected for cropping.");
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
        m_imageProcessor.saveCurrentState();
        m_darkLineInfoLabel->hide();
        resetDetectedLinesPointer();

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
        m_imageProcessor.saveCurrentState();
        resetDetectedLinesPointer();

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
                // Get dimensions using getter methods instead of vector size
                int rows = m_imageProcessor.getFinalImageHeight();
                int cols = m_imageProcessor.getFinalImageWidth();

                // Convert to double**
                double** matrix;
                malloc2D(matrix, rows, cols);
                for(int i = 0; i < rows; i++) {
                    for(int j = 0; j < cols; j++) {
                        matrix[i][j] = finalImage[i][j];
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

                // Convert CGData directly to double** for updateAndSaveFinalImage
                double** calibratedMatrix = nullptr;
                malloc2D(calibratedMatrix, calibratedData.Row, calibratedData.Column);

                for(int i = 0; i < calibratedData.Row; i++) {
                    for(int j = 0; j < calibratedData.Column; j++) {
                        calibratedMatrix[i][j] = std::clamp(calibratedData.Data[i][j], 0.0, 65535.0);
                    }
                }

                // Update the image using double** version
                m_imageProcessor.updateAndSaveFinalImage(calibratedMatrix, calibratedData.Row, calibratedData.Column);
                m_imageLabel->clearSelection();
                updateImageDisplay();

                // Free allocated memory
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

    connect(enhancedInterlaceBtn, &QPushButton::clicked, [this]() {
        if (checkZoomMode()) return;
        m_imageProcessor.saveCurrentState();
        m_darkLineInfoLabel->hide();
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

        mergeLayout->addWidget(weightedAvgRadio);
        mergeLayout->addWidget(minValueRadio);
        dialogLayout->addWidget(mergeBox);

        // Auto-calibration checkbox
        QCheckBox* autoCalibrationCheck = new QCheckBox("Auto-calibrate after interlace and merge");
        autoCalibrationCheck->setChecked(false);
        autoCalibrationCheck->setToolTip("Automatically apply calibration after interlace and merge process");
        dialogLayout->addWidget(autoCalibrationCheck);

        // Show Display Windows checkbox
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
            currentParamsLabel->setStyleSheet("color: #0066cc; font-size: 10px; margin-bottom: 8px;");
            dialogLayout->addWidget(currentParamsLabel);
        }

        QDialogButtonBox* buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Ok | QDialogButtonBox::Cancel
            );
        dialogLayout->addWidget(buttonBox);

        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            m_showDisplayWindows = showWindowsCheck->isChecked();

            // Hide existing windows if not needed
            if (!m_showDisplayWindows) {
                if (m_lowEnergyWindow) m_lowEnergyWindow->hide();
                if (m_highEnergyWindow) m_highEnergyWindow->hide();
                if (m_finalWindow) m_finalWindow->hide();
            }

            // Get current image data
            const auto& currentImage = m_imageProcessor.getFinalImage();
            if (!currentImage) {
                QMessageBox::warning(this, "Error", "No image data available");
                return;
            }

            int height = m_imageProcessor.getFinalImageHeight();
            int width = m_imageProcessor.getFinalImageWidth();

            // Convert to double pointer format
            double** inputImage = nullptr;
            malloc2D(inputImage, height, width);

            // Copy data
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    inputImage[y][x] = currentImage[y][x];
                }
            }

            // Set interlace parameters
            InterlaceProcessor::StartPoint lowEnergyStart =
                leftLeftLowRadio->isChecked() ?
                    InterlaceProcessor::StartPoint::LEFT_LEFT :
                    InterlaceProcessor::StartPoint::LEFT_RIGHT;

            InterlaceProcessor::StartPoint highEnergyStart =
                rightLeftHighRadio->isChecked() ?
                    InterlaceProcessor::StartPoint::RIGHT_LEFT :
                    InterlaceProcessor::StartPoint::RIGHT_RIGHT;

            InterlaceProcessor::MergeParams mergeParams(
                weightedAvgRadio->isChecked() ?
                    InterlaceProcessor::MergeMethod::WEIGHTED_AVERAGE :
                    InterlaceProcessor::MergeMethod::MINIMUM_VALUE
                );

            try {
                // Process the image
                auto result = InterlaceProcessor::processEnhancedInterlacedSections(
                    inputImage,
                    height,
                    width,
                    lowEnergyStart,
                    highEnergyStart,
                    mergeParams
                    );

                if (m_showDisplayWindows) {
                    // Create and update display windows
                    if (!m_lowEnergyWindow) {
                        m_lowEnergyWindow = std::make_unique<DisplayWindow>(
                            "Low Energy Interlaced", nullptr,
                            QPoint(this->x() + this->width() + 10, this->y())
                            );
                    }
                    if (!m_highEnergyWindow) {
                        m_highEnergyWindow = std::make_unique<DisplayWindow>(
                            "High Energy Interlaced", nullptr,
                            QPoint(this->x() + this->width() + 10, this->y() + 300)
                            );
                    }
                    if (!m_finalWindow) {
                        m_finalWindow = std::make_unique<DisplayWindow>(
                            "Final Merged Result", nullptr,
                            QPoint(this->x() + this->width() + 10, this->y() + 600)
                            );
                    }

                    m_lowEnergyWindow->updateImage(result->lowEnergyImage, height * 2, width / 4);
                    m_highEnergyWindow->updateImage(result->highEnergyImage, height * 2, width / 4);
                    m_finalWindow->updateImage(result->combinedImage, height * 2, width / 4);

                    m_lowEnergyWindow->show();
                    m_highEnergyWindow->show();
                    m_finalWindow->show();
                }

                // Update main display with combined image
                m_imageProcessor.updateAndSaveFinalImage(result->combinedImage, height * 2, width / 4);

                // Apply auto-calibration if checked
                if (autoCalibrationCheck->isChecked()) {
                    processCalibration(0, 0, CalibrationMode::BOTH_AXES);
                }

                // Clean up
                delete result;

                m_imageLabel->clearSelection();
                updateImageDisplay();

                // Generate status message
                QString lowEnergyStr = leftLeftLowRadio->isChecked() ? "LeftLeft" : "LeftRight";
                QString highEnergyStr = rightLeftHighRadio->isChecked() ? "RightLeft" : "RightRight";
                QString mergeMethodStr = weightedAvgRadio->isChecked() ? "Weighted Average" : "Minimum Value";

                QString statusMsg = QString("Sections: Low=%1, High=%2\nMerge Method: %3")
                                        .arg(lowEnergyStr)
                                        .arg(highEnergyStr)
                                        .arg(mergeMethodStr);

                if (autoCalibrationCheck->isChecked()) {
                    statusMsg += QString("\nAuto-calibrated using %1")
                    .arg(InterlaceProcessor::hasCalibrationParams() ?
                             InterlaceProcessor::getCalibrationParamsString() :
                             "default parameters");
                }

                updateLastAction("Enhanced Interlace", statusMsg);

            } catch (const std::exception& e) {
                QMessageBox::critical(this, "Error",
                                      QString("Failed to process interlaced sections: %1").arg(e.what()));
            }

            // Clean up input image
            for (int i = 0; i < height; i++) {
                free(inputImage[i]);
            }
            free(inputImage);
        }
    });

    layout->setSpacing(10);
    layout->addStretch();
    m_scrollLayout->addWidget(groupBox);
    updateCalibrationButtonText();
}



void ControlPanel::setupFilteringOperations() {

    createGroupBox("Image Enhancement", {
                                            {"CLAHE", [this]() {
                                                 if (checkZoomMode()) return;

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
                                                         // Get current image data
                                                         const auto& finalImage = m_imageProcessor.getFinalImage();
                                                         if (!finalImage) {
                                                             QMessageBox::warning(this, "Error", "No image data available");
                                                             return;
                                                         }

                                                         int height = m_imageProcessor.getFinalImageHeight();
                                                         int width = m_imageProcessor.getFinalImageWidth();

                                                         // Allocate buffers
                                                         double** inputBuffer = CLAHEProcessor::allocateImageBuffer(height, width);
                                                         double** outputBuffer = CLAHEProcessor::allocateImageBuffer(height, width);

                                                         // Copy input data and normalize to 0-1 range
                                                         for (int y = 0; y < height; ++y) {
                                                             for (int x = 0; x < width; ++x) {
                                                                 inputBuffer[y][x] = finalImage[y][x] / 65535.0;
                                                             }
                                                         }

                                                         CLAHEProcessor claheProcessor;
                                                         bool useGPU = gpuRadio->isChecked();
                                                         QString statusMsg;

                                                         // Start timing
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
                                                             statusMsg = QString("Standard (%1)\nClip: %2, Tile: %3")
                                                                             .arg(useGPU ? "GPU" : "CPU")
                                                                             .arg(clipSpinBox->value(), 0, 'f', 2)
                                                                             .arg(tileSpinBox->value());
                                                         } else if (thresholdRadio->isChecked()) {
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
                                                             statusMsg = QString("Threshold (%1)\nThreshold: %2, Clip: %3, Tile: %4")
                                                                             .arg(useGPU ? "GPU" : "CPU")
                                                                             .arg(thresholdValueSpinBox->value())
                                                                             .arg(clipSpinBox->value(), 0, 'f', 2)
                                                                             .arg(tileSpinBox->value());
                                                         } else {
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
                                                             statusMsg = QString("Combined (%1)\nClip: %2, Tile: %3")
                                                                             .arg(useGPU ? "GPU" : "CPU")
                                                                             .arg(clipSpinBox->value(), 0, 'f', 2)
                                                                             .arg(tileSpinBox->value());
                                                         }

                                                         // End timing
                                                         auto endTime = std::chrono::high_resolution_clock::now();
                                                         double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

                                                         // Add processing time to status message
                                                         statusMsg += QString("\nProcessing Time: %1 ms").arg(processingTime, 0, 'f', 2);

                                                         // Scale back to 16-bit range
                                                         double** finalOutput = CLAHEProcessor::allocateImageBuffer(height, width);
                                                         for (int y = 0; y < height; ++y) {
                                                             for (int x = 0; x < width; ++x) {
                                                                 finalOutput[y][x] = std::clamp(outputBuffer[y][x] * 65535.0, 0.0, 65535.0);
                                                             }
                                                         }

                                                         // Update image
                                                         m_imageProcessor.updateAndSaveFinalImage(finalOutput, height, width);

                                                         // Cleanup
                                                         CLAHEProcessor::deallocateImageBuffer(inputBuffer, height);
                                                         CLAHEProcessor::deallocateImageBuffer(outputBuffer, height);
                                                         CLAHEProcessor::deallocateImageBuffer(finalOutput, height);

                                                         // Update display
                                                         m_imageLabel->clearSelection();
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

    QScrollArea* darkLineScrollArea = qobject_cast<QScrollArea*>(
        qobject_cast<QVBoxLayout*>(m_mainLayout->itemAt(0)->layout())->itemAt(3)->widget()
        );
    if (darkLineScrollArea) {
        darkLineScrollArea->setVisible(false);
    }

    updateImageDisplay();
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
                                                    updateCalibrationButtonText();

                                                    // Clear detection results
                                                    clearAllDetectionResults();

                                                    // Reset zoom if active
                                                    if (m_imageProcessor.getZoomManager().isZoomModeActive()) {
                                                        m_imageProcessor.getZoomManager().resetZoom();
                                                        m_fixZoomButton->setChecked(false);
                                                        m_imageProcessor.getZoomManager().toggleFixedZoom(false);
                                                        toggleZoomMode(false);
                                                    }

                                                    // Clear image and display
                                                    m_imageLabel->clear();
                                                    m_imageSizeLabel->setText("Image Size: No image loaded");
                                                    m_lastActionLabel->setText("Last Action: None");
                                                    m_lastActionParamsLabel->clear();
                                                    m_lastActionParamsLabel->setVisible(false);
                                                    m_imageProcessor.clearHistory(); // Clear the image processing history

                                                    // Disable all buttons including Undo
                                                    for (QPushButton* button : m_allButtons) {
                                                        if (button) {
                                                            // Only enable Browse button
                                                            button->setEnabled(button->text() == "Browse");
                                                        }
                                                    }

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

                                                 QDialog dialog(this);
                                                 dialog.setWindowTitle("Gamma Adjustment");
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

                                                 // Gamma value input
                                                 QLabel* gammaLabel = new QLabel("Gamma Value:");
                                                 QDoubleSpinBox* gammaSpinBox = new QDoubleSpinBox();
                                                 gammaSpinBox->setRange(0.1, 10.0);
                                                 gammaSpinBox->setValue(1.0);
                                                 gammaSpinBox->setSingleStep(0.1);

                                                 layout->addWidget(gammaLabel);
                                                 layout->addWidget(gammaSpinBox);

                                                 // Region selection info label
                                                 QLabel* regionLabel = new QLabel("For regional adjustment, select the region in the image first.");
                                                 regionLabel->setWordWrap(true);
                                                 regionLabel->setVisible(false);
                                                 layout->addWidget(regionLabel);

                                                 // Connect radio buttons to show/hide region label
                                                 connect(regionalRadio, &QRadioButton::toggled, regionLabel, &QLabel::setVisible);

                                                 QDialogButtonBox* buttonBox = new QDialogButtonBox(
                                                     QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
                                                 layout->addWidget(buttonBox);

                                                 connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
                                                 connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

                                                 if (dialog.exec() == QDialog::Accepted) {
                                                     float gamma = gammaSpinBox->value();

                                                     try {
                                                         // Get current image data
                                                         const auto& finalImage = m_imageProcessor.getFinalImage();
                                                         if (!finalImage) {
                                                             QMessageBox::warning(this, "Error", "No image data available");
                                                             return;
                                                         }

                                                         int height = m_imageProcessor.getFinalImageHeight();
                                                         int width = m_imageProcessor.getFinalImageWidth();

                                                         // Convert to double pointer
                                                         double** imgData = nullptr;
                                                         malloc2D(imgData, height, width);

                                                         // Copy data
                                                         for (int y = 0; y < height; y++) {
                                                             for (int x = 0; x < width; x++) {
                                                                 imgData[y][x] = finalImage[y][x];
                                                             }
                                                         }

                                                         if (regionalRadio->isChecked()) {
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
                                                             ImageAdjustments::adjustGammaForSelectedRegion(imgData, height, width, gamma, selectedRegion);
                                                         } else {
                                                             ImageAdjustments::adjustGammaOverall(imgData, height, width, gamma);
                                                         }

                                                         m_imageProcessor.updateAndSaveFinalImage(imgData, height, width);

                                                         // Clean up
                                                         for (int i = 0; i < height; i++) {
                                                             free(imgData[i]);
                                                         }
                                                         free(imgData);

                                                         m_imageLabel->clearSelection();
                                                         updateImageDisplay();
                                                         updateLastAction(
                                                             regionalRadio->isChecked() ? "Regional Gamma" : "Overall Gamma",
                                                             QString::number(gamma, 'f', 2)
                                                             );

                                                     } catch (const std::exception& e) {
                                                         QMessageBox::critical(this, "Error",
                                                                               QString("Failed to apply gamma adjustment: %1").arg(e.what()));
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

                                                     try {
                                                         // Get current image data
                                                         const auto& finalImage = m_imageProcessor.getFinalImage();
                                                         if (!finalImage) {
                                                             QMessageBox::warning(this, "Error", "No image data available");
                                                             return;
                                                         }

                                                         int height = m_imageProcessor.getFinalImageHeight();
                                                         int width = m_imageProcessor.getFinalImageWidth();

                                                         // Convert to double pointer
                                                         double** imgData = nullptr;
                                                         malloc2D(imgData, height, width);

                                                         // Copy data
                                                         for (int y = 0; y < height; y++) {
                                                             for (int x = 0; x < width; x++) {
                                                                 imgData[y][x] = finalImage[y][x];
                                                             }
                                                         }

                                                         if (regionalRadio->isChecked()) {
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
                                                             ImageAdjustments::applyContrastToRegion(imgData, height, width, contrast, selectedRegion);
                                                         } else {
                                                             ImageAdjustments::adjustContrast(imgData, height, width, contrast);
                                                         }

                                                         m_imageProcessor.updateAndSaveFinalImage(imgData, height, width);

                                                         // Clean up
                                                         for (int i = 0; i < height; i++) {
                                                             free(imgData[i]);
                                                         }
                                                         free(imgData);

                                                         m_imageLabel->clearSelection();
                                                         updateImageDisplay();
                                                         updateLastAction(
                                                             regionalRadio->isChecked() ? "Regional Contrast" : "Overall Contrast",
                                                             QString::number(contrast, 'f', 2)
                                                             );

                                                     } catch (const std::exception& e) {
                                                         QMessageBox::critical(this, "Error",
                                                                               QString("Failed to apply contrast adjustment: %1").arg(e.what()));
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

                                                     try {
                                                         // Get current image data
                                                         const auto& finalImage = m_imageProcessor.getFinalImage();
                                                         if (!finalImage) {
                                                             QMessageBox::warning(this, "Error", "No image data available");
                                                             return;
                                                         }

                                                         int height = m_imageProcessor.getFinalImageHeight();
                                                         int width = m_imageProcessor.getFinalImageWidth();

                                                         // Convert to double pointer
                                                         double** imgData = nullptr;
                                                         malloc2D(imgData, height, width);

                                                         // Copy data
                                                         for (int y = 0; y < height; y++) {
                                                             for (int x = 0; x < width; x++) {
                                                                 imgData[y][x] = finalImage[y][x];
                                                             }
                                                         }

                                                         if (regionalRadio->isChecked()) {
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
                                                             ImageAdjustments::applySharpenToRegion(imgData, height, width, strength, selectedRegion);
                                                         } else {
                                                             ImageAdjustments::sharpenImage(imgData, height, width, strength);
                                                         }

                                                         m_imageProcessor.updateAndSaveFinalImage(imgData, height, width);

                                                         // Clean up
                                                         for (int i = 0; i < height; i++) {
                                                             free(imgData[i]);
                                                         }
                                                         free(imgData);

                                                         m_imageLabel->clearSelection();
                                                         updateImageDisplay();
                                                         updateLastAction(
                                                             regionalRadio->isChecked() ? "Regional Sharpen" : "Overall Sharpen",
                                                             QString::number(strength, 'f', 2)
                                                             );

                                                     } catch (const std::exception& e) {
                                                         QMessageBox::critical(this, "Error",
                                                                               QString("Failed to apply sharpen adjustment: %1").arg(e.what()));
                                                     }
                                                 }
                                             }}
                                        });
}

void ControlPanel::setupGraph() {
    createGroupBox("Graph Operations", {
                                           {"Histogram", [this]() {
                                                bool isCurrentlyVisible = m_histogram->isVisible();
                                                m_histogram->setVisible(!isCurrentlyVisible);

                                                if (!isCurrentlyVisible) {
                                                    const auto& finalImage = m_imageProcessor.getFinalImage();
                                                    int height = m_imageProcessor.getFinalImageHeight();
                                                    int width = m_imageProcessor.getFinalImageWidth();

                                                    if (finalImage) {  // Check if image exists
                                                        m_histogram->updateHistogram(finalImage, height, width);
                                                    }
                                                }

                                                m_mainLayout->invalidate();
                                                updateGeometry();
                                            }}
                                       });

    // Add histogram widget to the group box
    m_histogram = new Histogram(this);
    m_histogram->setMinimumWidth(250);
    m_histogram->setVisible(false);

    // Find the Graph Operations group box and add histogram to it
    for (int i = 0; i < m_scrollLayout->count(); ++i) {
        QGroupBox* box = qobject_cast<QGroupBox*>(m_scrollLayout->itemAt(i)->widget());
        if (box && box->title() == "Graph Operations") {
            box->layout()->addWidget(m_histogram);
            break;
        }
    }

    // Connect fileLoaded signal to enable histogram buttons
    connect(this, &ControlPanel::fileLoaded, this, [this](bool loaded) {
        for (QPushButton* button : m_allButtons) {
            if (button && (button->text() == "Histogram" || button->text() == "Toggle CLAHE View")) {
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

        QString paramString = QString("Mode: %1\nParameters: Y=%2, X=%3")
                                  .arg(actionDescription)
                                  .arg(newY)
                                  .arg(newX);

        updateLastAction("Calibration", paramString);

        InterlaceProcessor::setCalibrationParams(newY, newX);

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
        // 清理旧的检测结果
        resetDetectedLinesPointer();

        // 设置新的检测结果
        m_detectedLinesPointer = outLines;
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

void ControlPanel::validateImageData(const ImageData& imageData) {
    if (!imageData.data || imageData.rows <= 0 || imageData.cols <= 0) {
        throw std::runtime_error("Invalid image data");
    }

    // Additional validation for double** data
    for (int i = 0; i < imageData.rows; i++) {
        if (!imageData.data[i]) {
            throw std::runtime_error("Invalid row pointer in image data");
        }
    }
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


