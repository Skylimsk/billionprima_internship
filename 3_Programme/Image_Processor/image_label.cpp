#include "image_label.h"
#include <QDebug>
#include <QPainter>

ImageLabel::ImageLabel(PixelInfoCallback callback, QWidget* parent)
    : QLabel(parent),
    pixelInfoCallback(callback),
    selectedRegion(),
    regionSelected(false),
    isSelecting(false)
{
    setMouseTracking(true);
}

void ImageLabel::setImageTransform(const QTransform& newTransform) {
    transform = newTransform;
}

void ImageLabel::updateImage(const QPixmap& pixmap)
{
    setPixmap(pixmap);
    setFixedSize(pixmap.size());
}

void ImageLabel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        updatePixelInfo(event->pos());
        QPointF scaledPoint = transform.inverted().map(event->pos());
        int x = qBound(0, static_cast<int>(scaledPoint.x()), width() - 1);
        int y = qBound(0, static_cast<int>(scaledPoint.y()), height() - 1);
        selectedRegion.setTopLeft(QPoint(x, y));
        selectedRegion.setBottomRight(QPoint(x, y));
        isSelecting = true;
        regionSelected = false;
        update();
    }
}

void ImageLabel::mouseMoveEvent(QMouseEvent* event) {
    QPointF scaledPoint = transform.inverted().map(event->pos());
    int x = qBound(0, static_cast<int>(scaledPoint.x()), width() - 1);
    int y = qBound(0, static_cast<int>(scaledPoint.y()), height() - 1);

    emit mouseMoved(QPoint(x, y));

    if (isSelecting) {
        selectedRegion.setBottomRight(QPoint(x, y));
        update();
    }
}

void ImageLabel::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        updatePixelInfo(event->pos());
        QPointF scaledPoint = transform.inverted().map(event->pos());
        int x = qBound(0, static_cast<int>(scaledPoint.x()), width() - 1);
        int y = qBound(0, static_cast<int>(scaledPoint.y()), height() - 1);
        selectedRegion.setBottomRight(QPoint(x, y));
        isSelecting = false;
        regionSelected = true;
        update();
    }
}

void ImageLabel::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);

    if (isSelecting || regionSelected) {
        QPainter painter(this);
        painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));
        painter.drawRect(selectedRegion);
    }
}

void ImageLabel::leaveEvent(QEvent* /*event*/) {
    if (pixelInfoCallback) {
        pixelInfoCallback("Pixel Info: ");
    }
}

void ImageLabel::updatePixelInfo(const QPoint& pos) {
    QPointF scaledPoint = transform.inverted().map(pos);
    int x = qBound(0, static_cast<int>(scaledPoint.x()), width() - 1);
    int y = qBound(0, static_cast<int>(scaledPoint.y()), height() - 1);
    if (pixelInfoCallback) {
        QString info = QString("Pixel Info: X: %1, Y: %2").arg(x).arg(y);
        pixelInfoCallback(info);
    }
}

QRect ImageLabel::getSelectedRegion() const {
    return selectedRegion;
}

bool ImageLabel::isRegionSelected() const {
    return regionSelected;
}

void ImageLabel::clearSelection() {
    selectedRegion = QRect();
    regionSelected = false;
    update();
}
