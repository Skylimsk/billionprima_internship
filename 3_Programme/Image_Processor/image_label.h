#ifndef IMAGE_LABEL_H
#define IMAGE_LABEL_H

#include <QtWidgets/QLabel>
#include <QMouseEvent>
#include <functional>

class ImageLabel : public QLabel {
    Q_OBJECT

public:
    using PixelInfoCallback = std::function<void(const QString&)>;

    ImageLabel(PixelInfoCallback callback, QWidget* parent = nullptr);

    void setImageTransform(const QTransform& newTransform);
    QRect getSelectedRegion() const;
    bool isRegionSelected() const;
    void updateImage(const QPixmap& pixmap);
    void clearSelection();
    void setImageSize(const QSize& size) { m_imageSize = size; }
    void resetInteractionState();
    QRect getNormalizedRegion() const;

signals:
    void mouseMoved(const QPoint& pos);
    void selectionChanged();
    void selectionCleared();

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void updatePixelInfo(const QPoint& pos);
    QSize m_imageSize;
    QTransform transform;
    PixelInfoCallback pixelInfoCallback;
    QRect selectedRegion;
    QPoint startPoint;
    bool regionSelected;
    bool isSelecting;
    bool interactionEnabled;
    QRect normalizeRect(const QRect& rect) const;
};

#endif // IMAGE_LABEL_H
