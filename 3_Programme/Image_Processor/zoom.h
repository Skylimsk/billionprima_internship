#ifndef ZOOM_H
#define ZOOM_H

#include <QSize>
#include <QRect>
#include <QtWidgets/QMessageBox>

class ZoomManager {
public:
    ZoomManager();

    // Core zoom operations
    void setZoomLevel(float level);
    float getZoomLevel() const { return currentZoomLevel; }
    void zoomIn() { setZoomLevel(currentZoomLevel * ZOOM_STEP); }
    void zoomOut() { setZoomLevel(currentZoomLevel / ZOOM_STEP); }
    void resetZoom() {
        setZoomLevel(1.0f);
        storedZoomLevel = 1.0f;
    }

    // Zoom mode management
    void toggleZoomMode(bool active);
    bool isZoomModeActive() const { return zoomModeActive; }

    // Fixed zoom management
    void toggleFixedZoom(bool fixed);
    bool isZoomFixed() const { return zoomFixed; }
    void applyFixedZoom();

    // Size calculations
    QSize getZoomedSize(const QSize& originalSize) const;
    QRect getZoomedRect(const QRect& originalRect) const;
    QRect getUnzoomedRect(const QRect& zoomedRect) const;

    // Warning dialog
    bool showZoomWarningIfNeeded();

    static constexpr float MIN_ZOOM_LEVEL = 0.1f;
    static constexpr float MAX_ZOOM_LEVEL = 10.0f;
    static constexpr float ZOOM_STEP = 1.2f;

private:
    float currentZoomLevel;
    float storedZoomLevel;  // Changed from preZoomLevel
    float fixedZoomLevel;
    bool zoomModeActive;
    bool zoomFixed;
    QMessageBox* zoomWarningBox;
    QMessageBox* m_zoomResetDialog;

    void initializeWarningBox();
    void initializeZoomResetDialog();
};

#endif // ZOOM_H
