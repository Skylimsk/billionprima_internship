#ifndef IMAGE_LABEL_H
#define IMAGE_LABEL_H

#include <QLabel>
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

signals:
    void mouseMoved(const QPoint& pos);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void updatePixelInfo(const QPoint& pos);

    QTransform transform;
    PixelInfoCallback pixelInfoCallback;
    QRect selectedRegion;
    bool regionSelected;
    bool isSelecting;
};

#endif // IMAGE_LABEL_H
