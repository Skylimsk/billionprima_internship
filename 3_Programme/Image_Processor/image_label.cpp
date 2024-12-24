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

void ImageLabel::paintEvent(QPaintEvent* event) {
    QLabel::paintEvent(event);

    if (isSelecting || regionSelected) {
        QPainter painter(this);
        painter.setPen(QPen(Qt::red, 2, Qt::SolidLine));

        // 使用规范化后的矩形来绘制
        QRect drawRect = normalizeRect(selectedRegion);
        painter.drawRect(drawRect);

        if (drawRect.isValid() && (drawRect.width() > 1 || drawRect.height() > 1)) {
            QString sizeText = QString("%1 x %2")
            .arg(drawRect.width())
                .arg(drawRect.height());

            QRect textRect = painter.fontMetrics().boundingRect(sizeText);
            textRect.moveTopLeft(selectedRegion.topLeft() + QPoint(5, -textRect.height() - 5));

            if (textRect.top() < 0) {
                textRect.moveTop(selectedRegion.top() + 5);
            }
            if (textRect.right() > width()) {
                textRect.moveRight(selectedRegion.right() - 5);
            }

            painter.fillRect(textRect, QColor(255, 255, 255, 200));
            painter.drawText(textRect, Qt::AlignCenter, sizeText);
        }
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
    QRect region = getSelectedRegion();
    return !region.isEmpty() && region.width() > 1 && region.height() > 1;
}

void ImageLabel::clearSelection() {
    qDebug() << "Clearing selection state...";
    try {
        // 保存当前变换状态
        QTransform currentTransform = transform;

        selectedRegion = QRect();
        startPoint = QPoint();
        isSelecting = false;
        regionSelected = false;
        interactionEnabled = true;

        // 恢复保存的变换状态而不是重置它
        transform = currentTransform;

        update();
        emit selectionCleared();

        qDebug() << "Selection cleared successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in clearSelection:" << e.what();
    }
}

void ImageLabel::updateImage(const QPixmap& pixmap) {
    qDebug() << "\n=== UpdateImage Start ===";
    qDebug() << "New pixmap size:" << pixmap.size();

    try {
        setPixmap(pixmap);
        m_imageSize = pixmap.size();
        setFixedSize(pixmap.size());
        resetInteractionState();
        qDebug() << "Image updated successfully";
    } catch (const std::exception& e) {
        qDebug() << "Exception in updateImage:" << e.what();
    }
    qDebug() << "=== UpdateImage End ===\n";
}

void ImageLabel::resetInteractionState() {
    qDebug() << "\n=== ResetInteractionState ===";
    isSelecting = false;
    regionSelected = false;
    selectedRegion = QRect();
    interactionEnabled = true;
    qDebug() << "Interaction state reset complete";
    update();
}

QRect ImageLabel::normalizeRect(const QRect& rect) const {
    int x1 = rect.left();
    int y1 = rect.top();
    int x2 = rect.right();
    int y2 = rect.bottom();

    QRect normalized;
    normalized.setLeft(std::min(x1, x2));
    normalized.setTop(std::min(y1, y2));
    normalized.setRight(std::max(x1, x2));
    normalized.setBottom(std::max(y1, y2));

    return normalized;
}

QRect ImageLabel::getNormalizedRegion() const {
    return normalizeRect(selectedRegion);
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
    if (event->button() == Qt::LeftButton && isSelecting) {
        try {
            updatePixelInfo(event->pos());
            QPointF scaledPoint = transform.inverted().map(event->pos());
            int x = qBound(0, static_cast<int>(scaledPoint.x()), width() - 1);
            int y = qBound(0, static_cast<int>(scaledPoint.y()), height() - 1);

            // 保存上一次的选区状态
            QRect prevRegion = selectedRegion;

            // 更新并规范化选区
            selectedRegion.setBottomRight(QPoint(x, y));
            selectedRegion = normalizeRect(selectedRegion);

            // 验证选区是否有效
            if (selectedRegion.isValid() &&
                selectedRegion.width() > 1 &&
                selectedRegion.height() > 1) {

                isSelecting = false;
                regionSelected = true;

                // 发送选区改变信号
                emit selectionChanged();

                // 强制更新显示
                update();
            } else {
                // 如果选区无效，还原到之前的状态
                selectedRegion = prevRegion;
                isSelecting = false;
                regionSelected = false;
                update();
            }
        } catch (const std::exception& e) {
            qDebug() << "Exception in mouseReleaseEvent:" << e.what();
            resetInteractionState();
        }
    }
}
