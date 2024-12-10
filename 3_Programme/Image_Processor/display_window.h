#ifndef DISPLAY_WINDOW_H
#define DISPLAY_WINDOW_H

#include <QtWidgets/QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QScrollArea>
#include <memory>
#include <QString>

class DisplayWindow : public QWidget {
    Q_OBJECT

public:
    explicit DisplayWindow(const QString& title, QWidget* parent = nullptr,
                           const QPoint& position = QPoint(0, 0));
    ~DisplayWindow();
    void updateImage(double** image, int height, int width);
    void clear();

private slots:
    void saveImage();
    void zoomChanged(int value);

private:
    void setupUI();
    void updateDisplayedImage();
    void cleanupImage();

    QVBoxLayout* mainLayout;
    QHBoxLayout* controlsLayout;
    QPushButton* saveButton;
    QSlider* zoomSlider;
    QLabel* zoomLabel;
    QScrollArea* scrollArea;
    QLabel* imageLabel;
    double currentZoom;

    // Image data
    double** originalImage;
    int imageHeight;
    int imageWidth;

    void setWindowPosition(const QPoint& position);
};

#endif // DISPLAY_WINDOW_H
