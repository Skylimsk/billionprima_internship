#include "zoom.h"
#include <algorithm>

ZoomManager::ZoomManager()
    : currentZoomLevel(1.0f)
    , storedZoomLevel(1.0f)
    , fixedZoomLevel(1.0f)
    , zoomModeActive(false)
    , zoomFixed(false)
    , zoomWarningBox(nullptr)
    , m_zoomResetDialog(nullptr) {
    initializeWarningBox();
    initializeZoomResetDialog();
}

void ZoomManager::setZoomLevel(float level) {
    currentZoomLevel = std::clamp(level, MIN_ZOOM_LEVEL, MAX_ZOOM_LEVEL);
    if (!zoomModeActive) {
        storedZoomLevel = currentZoomLevel;
    }
}

void ZoomManager::toggleZoomMode(bool active) {
    if (zoomModeActive == active) return;

    if (active) {
        // Activating zoom mode - restore stored zoom level
        zoomModeActive = true;
        if (!zoomFixed) {
            currentZoomLevel = storedZoomLevel;
        }
    } else {
        // Deactivating zoom mode - store current zoom level
        zoomModeActive = false;
        storedZoomLevel = currentZoomLevel;
        // Don't reset zoom level when deactivating
    }
}

void ZoomManager::toggleFixedZoom(bool fixed) {
    if (zoomFixed == fixed) return;

    zoomFixed = fixed;
    if (fixed) {
        fixedZoomLevel = currentZoomLevel;
    }
}

void ZoomManager::applyFixedZoom() {
    if (zoomFixed) {
        currentZoomLevel = fixedZoomLevel;
    }
}

QSize ZoomManager::getZoomedSize(const QSize& originalSize) const {
    return QSize(
        static_cast<int>(originalSize.width() * currentZoomLevel),
        static_cast<int>(originalSize.height() * currentZoomLevel)
        );
}

QRect ZoomManager::getZoomedRect(const QRect& originalRect) const {
    return QRect(
        static_cast<int>(originalRect.x() * currentZoomLevel),
        static_cast<int>(originalRect.y() * currentZoomLevel),
        static_cast<int>(originalRect.width() * currentZoomLevel),
        static_cast<int>(originalRect.height() * currentZoomLevel)
        );
}

QRect ZoomManager::getUnzoomedRect(const QRect& zoomedRect) const {
    return QRect(
        static_cast<int>(zoomedRect.x() / currentZoomLevel),
        static_cast<int>(zoomedRect.y() / currentZoomLevel),
        static_cast<int>(zoomedRect.width() / currentZoomLevel),
        static_cast<int>(zoomedRect.height() / currentZoomLevel)
        );
}

bool ZoomManager::showZoomWarningIfNeeded() {
    if (zoomModeActive && !zoomFixed) {
        zoomWarningBox->exec();
        return true;
    }
    return false;
}

void ZoomManager::initializeWarningBox() {
    zoomWarningBox = new QMessageBox();
    zoomWarningBox->setIcon(QMessageBox::Warning);
    zoomWarningBox->setWindowTitle("Zoom Mode Active");
    zoomWarningBox->setText("No functions can be applied while in Zoom Mode.\nPlease exit Zoom Mode first.");
    zoomWarningBox->setStandardButtons(QMessageBox::Ok);
}

void ZoomManager::initializeZoomResetDialog() {
    m_zoomResetDialog = new QMessageBox();
    m_zoomResetDialog->setIcon(QMessageBox::Question);
    m_zoomResetDialog->setWindowTitle("Keep Zoom Level");
    m_zoomResetDialog->setText("Do you want to maintain the current zoom level?");
    m_zoomResetDialog->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m_zoomResetDialog->setDefaultButton(QMessageBox::No);
}
