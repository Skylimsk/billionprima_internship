# ifndef IMAGEPROCESSOR_H
# define IMAGEPROCESSOR_H

#include <QMainWindow>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

class ImageProcessor : public QMainWindow {
    Q_OBJECT

public:
    ImageProcessor(QWidget *parent = nullptr);

private slots:
    void loadFile();
    void saveFile();

private:
    void setupUI();
    void connectSignalsSlots();
    void loadTxtImage(const std::string& txtFilePath);
    void updateDisplayImage();
    void saveCurrentState();

    QWidget *centralWidget;
    QLabel *imageLabel;
    QPushButton *loadButton;
    QPushButton *saveButton;

    std::vector<std::vector<uint16_t>> imgData;
    std::vector<std::vector<uint16_t>> originalImg;
    std::vector<std::vector<uint16_t>> finalImage;

    QRect selectedRegion;
    bool regionSelected;
    QImage displayImage;
};

#endif // IMAGEPROCESSOR_H
