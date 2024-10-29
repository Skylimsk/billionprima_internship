#include <QApplication>
#include <QWidget>
#include <QHBoxLayout>
#include <QScrollArea>
#include "image_processor.h"
#include "image_label.h"
#include "control_panel.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    QWidget window;
    window.setWindowTitle("Image Processor");
    window.resize(1600, 800);

    QHBoxLayout* mainLayout = new QHBoxLayout(&window);

    ImageLabel* imageLabel = new ImageLabel([](const QString&){}, &window);
    ImageProcessor imageProcessor(imageLabel);

    ControlPanel* controlPanel = new ControlPanel(imageProcessor, imageLabel);
    QScrollArea* controlScrollArea = new QScrollArea();
    controlScrollArea->setWidget(controlPanel);
    controlScrollArea->setWidgetResizable(true);
    controlScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QScrollArea* imageScrollArea = new QScrollArea();
    imageScrollArea->setWidget(imageLabel);
    imageScrollArea->setWidgetResizable(true);
    imageScrollArea->setAlignment(Qt::AlignCenter);

    mainLayout->addWidget(controlScrollArea, 3);
    mainLayout->addWidget(imageScrollArea, 9);

    QObject::connect(imageLabel, &ImageLabel::mouseMoved, controlPanel, &ControlPanel::updatePixelInfo);

    window.setLayout(mainLayout);
    window.show();
    return app.exec();
}
