#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Test OpenCV CUDA
    try {
        qDebug() << "CUDA Device Count:" << cv::cuda::getCudaEnabledDeviceCount();
        cv::cuda::printCudaDeviceInfo(0);
    } catch (const cv::Exception& e) {
        qDebug() << "OpenCV CUDA Error:" << e.what();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
