// #include <QApplication>
// #include <QMessageBox>
// #include <opencv2/opencv.hpp>
// #include <opencv2/cudaimgproc.hpp>

// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);

//     try {
//         int deviceCount = cv::cuda::getCudaEnabledDeviceCount();
//         QMessageBox::information(nullptr, "CUDA Test",
//                                  QString("CUDA Devices Found: %1").arg(deviceCount));
//     }
//     catch (const cv::Exception& e) {
//         QMessageBox::critical(nullptr, "Error",
//                               QString("CUDA Test Failed: %1").arg(e.what()));
//     }

//     return a.exec();
// }

// #include <QApplication>
// #include <QMessageBox>
// #include <opencv2/opencv.hpp>
// #include <opencv2/cudaimgproc.hpp>
// #include <opencv2/cudafilters.hpp>

// int main(int argc, char *argv[])
// {
//     QApplication a(argc, argv);

//     try {
//         // Check CUDA device
//         int deviceCount = cv::cuda::getCudaEnabledDeviceCount();
//         if (deviceCount == 0) throw cv::Exception(0, "No CUDA devices found", "", __FILE__, __LINE__);

//         // Create test image
//         cv::Mat cpuSrc(1000, 1000, CV_8UC3, cv::Scalar(255, 0, 0));
//         cv::cuda::GpuMat gpuSrc;
//         gpuSrc.upload(cpuSrc);

//         // Apply Gaussian blur using CUDA
//         cv::Ptr<cv::cuda::Filter> gaussian = cv::cuda::createGaussianFilter(
//             CV_8UC3, CV_8UC3, cv::Size(5, 5), 1.0);

//         cv::cuda::GpuMat gpuDst;
//         gaussian->apply(gpuSrc, gpuDst);

//         // Download result
//         cv::Mat cpuDst;
//         gpuDst.download(cpuDst);

//         QMessageBox::information(nullptr, "Success",
//                                  QString("CUDA Test Passed!\nDevices: %1\nImage Size: %2x%3")
//                                      .arg(deviceCount)
//                                      .arg(cpuDst.cols)
//                                      .arg(cpuDst.rows));
//     }
//     catch (const cv::Exception& e) {
//         QMessageBox::critical(nullptr, "Error",
//                               QString("CUDA Test Failed: %1").arg(e.what()));
//     }

//     return a.exec();
// }

#include <QApplication>
#include <QMessageBox>
#include <QFileInfo>
#include <opencv2/opencv.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    try {
        // Check CUDA device
        int deviceCount = cv::cuda::getCudaEnabledDeviceCount();
        if (deviceCount == 0)
            throw cv::Exception(0, "No CUDA devices found", "", __FILE__, __LINE__);

        // Read input image using full path
        QString imagePath = "D:/billionprima_internship/3_Programme/Test_Debug/input.jpg";
        if (!QFileInfo::exists(imagePath)) {
            throw cv::Exception(0, "Input image not found: " + imagePath.toStdString(), "", __FILE__, __LINE__);
        }

        cv::Mat cpuSrc = cv::imread(imagePath.toStdString());
        if (cpuSrc.empty()) {
            throw cv::Exception(0, "Failed to load image: " + imagePath.toStdString(), "", __FILE__, __LINE__);
        }

        // Upload to GPU
        cv::cuda::GpuMat gpuSrc;
        gpuSrc.upload(cpuSrc);

        // Process on GPU
        cv::cuda::GpuMat gpuGray;
        cv::cuda::cvtColor(gpuSrc, gpuGray, cv::COLOR_BGR2GRAY);

        // Gaussian blur
        cv::Ptr<cv::cuda::Filter> gaussian = cv::cuda::createGaussianFilter(
            CV_8UC1, CV_8UC1, cv::Size(5, 5), 1.0);
        cv::cuda::GpuMat gpuBlurred;
        gaussian->apply(gpuGray, gpuBlurred);

        // Save results to the same directory as input
        cv::Mat cpuGray;
        gpuGray.download(cpuGray);
        cv::imwrite("D:/billionprima_internship/3_Programme/Test_Debug/output_gray.jpg", cpuGray);

        cv::Mat cpuBlurred;
        gpuBlurred.download(cpuBlurred);
        cv::imwrite("D:/billionprima_internship/3_Programme/Test_Debug/output_blur.jpg", cpuBlurred);

        QMessageBox::information(nullptr, "Success",
                                 QString("CUDA Image Processing Complete!\n"
                                         "Input Size: %1x%2\n"
                                         "Files saved in: D:/billionprima_internship/3_Programme/Test_Debug/")
                                     .arg(cpuSrc.cols)
                                     .arg(cpuSrc.rows));
    }
    catch (const cv::Exception& e) {
        QMessageBox::critical(nullptr, "Error",
                              QString("Processing Failed: %1").arg(e.what()));
    }

    return a.exec();
}
