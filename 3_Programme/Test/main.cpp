// main.cpp
#include <QApplication>
#include "imageprocessor.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    ImageProcessor processor;
    processor.show();
    return app.exec();
}
