#pragma once
#include "pch.h"
#include "CLAHE.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <atomic>
#include <future>
#include <vector>

std::mutex CLAHEProcessor::logMutex;

struct PixelInfo {
    int x, y;
    uint16_t value;
};

// Constructor: Sets up CLAHE processor with hardware threads and OpenCV CLAHE object
CLAHEProcessor::CLAHEProcessor()
    : threadConfig(std::thread::hardware_concurrency(), 64) {
    clahe = cv::createCLAHE();
    metrics.reset();
}

// Helper function: Memory Management Functions
// Allocates a 2D double array for image processing
double** CLAHEProcessor::allocateImageBuffer(int height, int width) {
    double** buffer;
    malloc2D(buffer, height, width);
    return buffer;
}

// Safely deallocates a 2D image buffer to prevent memory leaks
void CLAHEProcessor::deallocateImageBuffer(double** buffer, int height) {
    if (buffer) {
        for (int i = 0; i < height; i++) {
            delete[] buffer[i];
        }
        delete[] buffer;
    }
}

//Helper function:Conversion of Format
// Converts double array image data to OpenCV Mat format for processing
cv::Mat CLAHEProcessor::doubleToMat(double** image, int height, int width) {
    cv::Mat mat(height, width, CV_64F);
    for (int y = 0; y < height; ++y) {
        double* matRow = mat.ptr<double>(y);
        for (int x = 0; x < width; ++x) {
            matRow[x] = image[y][x];
        }
    }
    return mat;
}

// Converts OpenCV Mat back to double array format after processing
void CLAHEProcessor::matToDouble(const cv::Mat& mat, double** image) {
    int height = mat.rows;
    int width = mat.cols;

    for (int y = 0; y < height; ++y) {
        const double* matRow = mat.ptr<double>(y);
        for (int x = 0; x < width; ++x) {
            image[y][x] = matRow[x];
        }
    }
}

//Helper Function: Logging and Monitoring
// Thread-safe logging of processing start with row range information
void CLAHEProcessor::logThreadStart(int threadId, const std::string& function,
                                    int startRow, int endRow) {
    std::lock_guard<std::mutex> lock(logMutex);
    qDebug() << QString("Thread ID %1 processed from row %2 to row %3 for processing %4")
                    .arg(threadId)
                    .arg(startRow)
                    .arg(endRow)
                    .arg(function.c_str())
                    .toStdString().c_str();
}

// Thread-safe logging of thread completion
void CLAHEProcessor::logThreadComplete(int threadId, const std::string& function) {
    std::lock_guard<std::mutex> lock(logMutex);
    qDebug() << QString("Thread ID %1 completed processing %2")
                    .arg(threadId)
                    .arg(function.c_str())
                    .toStdString().c_str();
}

// Thread-safe general message logging with thread identification
void CLAHEProcessor::logMessage(const std::string& message, int threadId) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::stringstream ss;
    ss << "[Thread " << std::setw(2) << threadId << "] " << message;
    qDebug() << ss.str().c_str();
}

// Standard CLAHE: Enhances local contrast by applying histogram equalization within
// small tile regions with a clip limit to prevent noise amplification.

// GPU-accelerated CLAHE implementation with timing metrics
void CLAHEProcessor::applyCLAHE(double** outputImage, double** inputImage,
                                int height, int width, double clipLimit,
                                const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads; // GPU processing
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Log start of processing
        logThreadStart(0, "CLAHE_GPU", 0, height);

        // Convert 16-bit input directly to 8-bit for CLAHE in a single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        for (int y = 0; y < height; ++y) {
            uchar* row = input8bit.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                row[x] = static_cast<uchar>((inputImage[y][x] * 255.0) / 65535.0);
            }
        }

        // Apply CLAHE
        cv::cuda::GpuMat gpuInput;
        gpuInput.upload(input8bit);

        cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat gpuResult;
        gpuClahe->apply(gpuInput, gpuResult);

        // Download result
        cv::Mat result;
        gpuResult.download(result);

        // Convert back to 16-bit range in a single loop
        for (int y = 0; y < height; ++y) {
            const uchar* resultRow = result.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                outputImage[y][x] = (static_cast<double>(resultRow[x]) * 65535.0) / 255.0;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

        logThreadComplete(0, "CLAHE_GPU");
    }
    catch (const cv::Exception& e) {
        throw;
    }
}

//Multi-threaded CPU CLAHE implementation with parallel processing
void CLAHEProcessor::applyCLAHE_CPU(double** outputImage, double** inputImage,
                                    int height, int width, double clipLimit,
                                    const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert 16-bit input directly to 8-bit for CLAHE
        cv::Mat input8bit(height, width, CV_8UC1);
        for (int y = 0; y < height; ++y) {
            uchar* row = input8bit.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                row[x] = static_cast<uchar>((inputImage[y][x] * 255.0) / 65535.0);
            }
        }

        cv::Mat result = cv::Mat::zeros(input8bit.size(), input8bit.type());

        // Create CLAHE instances for each thread
        std::vector<cv::Ptr<cv::CLAHE>> claheInstances(threadConfig.numThreads);
        for (auto& instance : claheInstances) {
            instance = cv::createCLAHE(clipLimit, tileSize);
        }

        // Process image in parallel strips
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "CLAHE_CPU", startRow, endRow);

                cv::Mat strip = input8bit(cv::Range(startRow, endRow), cv::Range::all());
                cv::Mat stripResult;
                claheInstances[i]->apply(strip, stripResult);

                {
                    std::lock_guard<std::mutex> lock(processingMutex);
                    stripResult.copyTo(result(cv::Range(startRow, endRow), cv::Range::all()));
                }

                logThreadComplete(i, "CLAHE_CPU");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }

        // Convert back to 16-bit range in a single loop
        for (int y = 0; y < height; ++y) {
            const uchar* resultRow = result.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                outputImage[y][x] = (static_cast<double>(resultRow[x]) * 65535.0) / 255.0;
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;
    }
    catch (const cv::Exception& e) {
        throw;
    }
}

//Combined CLAHE: Preserves the brightest information from the original image
//while enhancing contrast by taking the maximum value (Brighter) between original and CLAHE-processed pixels.

//Combined CPU-accelerated CLAHE with enhanced brightness preservation
void CLAHEProcessor::applyCombinedCLAHE_CPU(double** outputImage, double** inputImage,
                                            int height, int width, double clipLimit,
                                            const cv::Size& tileSize) {

    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;

    // Start timing the processing
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // 1. Convert input 2D array to OpenCV Mat format for processing
        cv::Mat matImage(height, width, CV_64F);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                matImage.at<double>(y, x) = inputImage[y][x];
            }
        }

        // 2. Convert image to 16-bit unsigned integer format
        cv::Mat matImage16U;
        matImage.convertTo(matImage16U, CV_16UC1, 65535.0);

        // 3. Convert to 8-bit for OpenCV's CLAHE implementation
        cv::Mat image8bit;
        matImage16U.convertTo(image8bit, CV_8UC1, 255.0/65535.0);

        // Create CLAHE object with specified clip limit and tile size
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);

        // Apply CLAHE to enhance local contrast
        cv::Mat processed8bit;
        clahe->apply(image8bit, processed8bit);

        // 4. Convert CLAHE result back to 16-bit format
        cv::Mat processed16U;
        processed8bit.convertTo(processed16U, CV_16UC1, 65535.0/255.0);

        // 5. Compare original and CLAHE-processed images
        cv::Mat result = cv::Mat::zeros(height, width, CV_16UC1);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                uint16_t originalVal = matImage16U.at<uint16_t>(y, x);
                uint16_t claheVal = processed16U.at<uint16_t>(y, x);

                // Choose the maximum value to retain the brightest information
                result.at<uint16_t>(y, x) = std::max(originalVal, claheVal);
            }
        }

        // 6. Convert result back to double format
        cv::Mat resultDouble;
        result.convertTo(resultDouble, CV_64F, 1.0/65535.0);

        // 7. Copy processed image back to output 2D array
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                outputImage[y][x] = resultDouble.at<double>(y, x);
            }
        }

        // Calculate and store processing time
        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;
    }
    catch (const cv::Exception& e) {

        qDebug() << "Error in threshold CLAHE processing:" << e.what();
        throw;
    }
}

//GPU version of combined CLAHE with 16-bit precision
void CLAHEProcessor::applyCombinedCLAHE(double** outputImage, double** inputImage,
                                        int height, int width, double clipLimit,
                                        const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert to 8-bit directly with single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        for (int y = 0; y < height; ++y) {
            uchar* row = input8bit.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                row[x] = static_cast<uchar>((inputImage[y][x] * 255.0) / 65535.0);
            }
        }

        // Upload to GPU and apply CLAHE
        cv::cuda::GpuMat d_input8bit;
        d_input8bit.upload(input8bit);

        // Create GPU CLAHE object and process
        cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat d_processed;
        clahe->apply(d_input8bit, d_processed);

        // Download result
        cv::Mat processed;
        d_processed.download(processed);

        // Compare and take maximum in 16-bit space with single loop
        for (int y = 0; y < height; ++y) {
            const uchar* processedRow = processed.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                double original = inputImage[y][x];
                double processed = (static_cast<double>(processedRow[x]) * 65535.0) / 255.0;
                outputImage[y][x] = std::max(original, processed);
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

    } catch (const cv::Exception& e) {
        logMessage("Error in GPU Combined CLAHE: " + std::string(e.what()), -1);
        throw;
    }
}

//Threshold CLAHE: Selectively applies CLAHE only to dark regions below a threshold value while leaving brighter areas untouched.

//GPU-accelerated CLAHE with threshold-based dark region enhancement
void CLAHEProcessor::applyThresholdCLAHE(double** finalImage, int height, int width,
                                         uint16_t threshold, double clipLimit,
                                         const cv::Size& tileSize, bool afterNormalCLAHE) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = 1; // GPU version uses 1 host thread
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Create 8-bit input directly
        cv::Mat input8bit(height, width, CV_8UC1);
        for (int y = 0; y < height; ++y) {
            uchar* row = input8bit.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                row[x] = static_cast<uchar>((finalImage[y][x] * 255.0) / 65535.0);
            }
        }

        // Create dark region mask
        cv::cuda::GpuMat d_darkMask;
        cv::Mat darkMask;
        cv::threshold(input8bit, darkMask, (threshold * 255.0) / 65535.0, 255, cv::THRESH_BINARY_INV);
        d_darkMask.upload(darkMask);

        if (cv::countNonZero(darkMask) > 0) {
            // Process dark regions with CLAHE
            cv::cuda::GpuMat d_input8bit;
            d_input8bit.upload(input8bit);

            cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(clipLimit, tileSize);
            cv::cuda::GpuMat d_processed;
            clahe->apply(d_input8bit, d_processed);

            // Download result
            cv::Mat processed;
            d_processed.download(processed);

            // Convert back to 16-bit range directly
            for (int y = 0; y < height; ++y) {
                const uchar* processedRow = processed.ptr<uchar>(y);
                const uchar* maskRow = darkMask.ptr<uchar>(y);
                for (int x = 0; x < width; ++x) {
                    if (maskRow[x] > 0) { // Apply only to dark regions
                        finalImage[y][x] = (static_cast<double>(processedRow[x]) * 65535.0) / 255.0;
                    }
                }
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

    } catch (const cv::Exception& e) {
        logMessage("Error in GPU Threshold CLAHE: " + std::string(e.what()), -1);
        throw;
    }
}

//CPU implementation of threshold-based CLAHE for dark regions
void CLAHEProcessor::applyThresholdCLAHE_CPU(double** finalImage, int height, int width,
                                             uint16_t threshold, double clipLimit,
                                             const cv::Size& tileSize, bool afterNormalCLAHE) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert to 8-bit with single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        for (int y = 0; y < height; ++y) {
            uchar* row = input8bit.ptr<uchar>(y);
            for (int x = 0; x < width; ++x) {
                row[x] = static_cast<uchar>((finalImage[y][x] * 255.0) / 65535.0);
            }
        }

        // Create dark region mask
        cv::Mat darkMask;
        cv::threshold(input8bit, darkMask, (threshold * 255.0) / 65535.0, 255, cv::THRESH_BINARY_INV);

        if (cv::countNonZero(darkMask) > 0) {
            // Process dark regions with CLAHE
            cv::Mat processed;
            cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
            clahe->apply(input8bit, processed);

            // Apply results only to dark regions, convert directly back to 16-bit
            for (int y = 0; y < height; ++y) {
                const uchar* processedRow = processed.ptr<uchar>(y);
                const uchar* maskRow = darkMask.ptr<uchar>(y);
                for (int x = 0; x < width; ++x) {
                    if (maskRow[x] > 0) { // Only modify dark regions
                        finalImage[y][x] = (static_cast<double>(processedRow[x]) * 65535.0) / 255.0;
                    }
                }
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

    } catch (const cv::Exception& e) {
        logMessage("Error in CPU Threshold CLAHE: " + std::string(e.what()), -1);
        throw;
    }
}
