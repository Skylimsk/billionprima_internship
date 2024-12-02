#ifndef DISPLAY_WINDOW_H
#define DISPLAY_WINDOW_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QScrollArea>
#include <memory>
#include <vector>
#include <QString>

class DisplayWindow : public QWidget {
    Q_OBJECT

public:
    explicit DisplayWindow(const QString& title, QWidget* parent = nullptr,
                           const QPoint& position = QPoint(0, 0));
    void updateImage(const std::vector<std::vector<uint16_t>>& image);
    void clear();

private slots:
    void saveImage();
    void zoomChanged(int value);

private:
    void setupUI();
    void updateDisplayedImage();

    QVBoxLayout* mainLayout;
    QHBoxLayout* controlsLayout;
    QPushButton* saveButton;
    QSlider* zoomSlider;
    QLabel* zoomLabel;
    QScrollArea* scrollArea;
    QLabel* imageLabel;
    double currentZoom;
    std::unique_ptr<std::vector<std::vector<uint16_t>>> originalImage;

    void setWindowPosition(const QPoint& position);
};

#endif // DISPLAY_WINDOW_H
