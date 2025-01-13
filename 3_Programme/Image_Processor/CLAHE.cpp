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

// ========== D2D CLAHE FROM HERE ==========

// Standard CLAHE: Enhances local contrast by applying histogram equalization within
// small tile regions with a clip limit to prevent noise amplification.

// GPU-accelerated CLAHE implementation with timing metrics
void CLAHEProcessor::applyCLAHE(double** outputImage, double** inputImage,
                                int height, int width, double clipLimit,
                                const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        logThreadStart(0, "CLAHE_GPU", 0, height);

        // Convert 16-bit input to 8-bit with single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputImage[i / width][i % width] * 255.0) / 65535.0);
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

        // Convert back to 16-bit range with single loop
        if (result.isContinuous()) {
            const uchar* data = result.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                outputImage[i / width][i % width] = (static_cast<double>(data[i]) * 65535.0) / 255.0;
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
        // Convert to 8-bit with single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputImage[i / width][i % width] * 255.0) / 65535.0);
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

        // Convert back to 16-bit range with single loop
        if (result.isContinuous()) {
            const uchar* data = result.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                outputImage[i / width][i % width] =
                    (static_cast<double>(data[i]) * 65535.0) / 255.0;
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
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputImage[i / width][i % width] * 255.0) / 65535.0);
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
        if (processed.isContinuous()) {
            const uchar* data = processed.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                int y = i / width;
                int x = i % width;
                double original = inputImage[y][x];
                double processed = (static_cast<double>(data[i]) * 65535.0) / 255.0;
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

//Combined CPU-accelerated CLAHE with enhanced brightness preservation
void CLAHEProcessor::applyCombinedCLAHE_CPU(double** outputImage, double** inputImage,
                                            int height, int width, double clipLimit,
                                            const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert to 8-bit directly with single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputImage[i / width][i % width] * 255.0) / 65535.0);
            }
        }

        // Create CPU CLAHE object and process
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
        cv::Mat processed;
        clahe->apply(input8bit, processed);

        // Compare and take maximum in 16-bit space with single loop
        if (processed.isContinuous()) {
            const uchar* data = processed.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                int y = i / width;
                int x = i % width;
                double original = inputImage[y][x];
                double processed = (static_cast<double>(data[i]) * 65535.0) / 255.0;
                outputImage[y][x] = std::max(original, processed);
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

    } catch (const cv::Exception& e) {
        logMessage("Error in CPU Combined CLAHE: " + std::string(e.what()), -1);
        throw;
    }
}

//Threshold CLAHE: Selectively applies CLAHE only to dark regions below a threshold value while leaving brighter areas untouched.

//GPU-accelerated CLAHE with threshold-based dark region enhancement
void CLAHEProcessor::applyThresholdCLAHE(double** finalImage, int height, int width,
                                         uint16_t threshold, double clipLimit,
                                         const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = 1;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert to 8-bit with single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((finalImage[i / width][i % width] * 255.0) / 65535.0);
            }
        }

        // Create dark region mask
        cv::Mat darkMask;
        cv::threshold(input8bit, darkMask, (threshold * 255.0) / 65535.0, 255, cv::THRESH_BINARY_INV);

        if (cv::countNonZero(darkMask) > 0) {
            // Process with CLAHE
            cv::cuda::GpuMat d_input8bit, d_darkMask;
            d_input8bit.upload(input8bit);
            d_darkMask.upload(darkMask);

            cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(clipLimit, tileSize);
            cv::cuda::GpuMat d_processed;
            clahe->apply(d_input8bit, d_processed);

            cv::Mat processed;
            d_processed.download(processed);

            // Apply results with single loop
            if (processed.isContinuous() && darkMask.isContinuous()) {
                const uchar* procData = processed.ptr<uchar>(0);
                const uchar* maskData = darkMask.ptr<uchar>(0);
                int totalPixels = height * width;
                for (int i = 0; i < totalPixels; ++i) {
                    if (maskData[i] > 0) {
                        finalImage[i / width][i % width] =
                            (static_cast<double>(procData[i]) * 65535.0) / 255.0;
                    }
                }
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

    } catch (const cv::Exception& e) {
        logMessage("Error in GPU Threshold CLAHE: " + std::string(e.what()), -1);
        throw;
    }
}

//CPU implementation of threshold-based CLAHE for dark regions
void CLAHEProcessor::applyThresholdCLAHE_CPU(double** finalImage, int height, int width,
                                             uint16_t threshold, double clipLimit,
                                             const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert to 8-bit with single loop
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((finalImage[i / width][i % width] * 255.0) / 65535.0);
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

            // Apply results with single loop
            if (processed.isContinuous() && darkMask.isContinuous()) {
                const uchar* procData = processed.ptr<uchar>(0);
                const uchar* maskData = darkMask.ptr<uchar>(0);
                int totalPixels = height * width;
                for (int i = 0; i < totalPixels; ++i) {
                    if (maskData[i] > 0) {
                        finalImage[i / width][i % width] =
                            (static_cast<double>(procData[i]) * 65535.0) / 255.0;
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

// ========== uint32t CLAHE FROM HERE ==========

void CLAHEProcessor::applyCLAHE(double** outputImage, const uint32_t* inputBuffer,
                                int height, int width, double clipLimit, const cv::Size& tileSize) {
    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        logThreadStart(0, "CLAHE_GPU", 0, height);

        // Step 1: Convert 32-bit input (0-4294967295) to 8-bit (0-255) for CLAHE processing
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                // Scale from 32-bit to 8-bit range
                data[i] = static_cast<uchar>((inputBuffer[i] * 255.0) / 4294967295.0);
            }
        }

        // Step 2: Process with GPU CLAHE (operates on 8-bit data)
        cv::cuda::GpuMat gpuInput;
        gpuInput.upload(input8bit);
        cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat gpuResult;
        gpuClahe->apply(gpuInput, gpuResult);

        // Download result
        cv::Mat result;
        gpuResult.download(result);

        // Step 3: Convert 8-bit CLAHE result (0-255) back to 16-bit range (0-65535)
        const uchar* resultData = result.ptr<uchar>(0);
        int totalPixels = height * width;
        #pragma omp parallel for
        for (int i = 0; i < totalPixels; ++i) {
            outputImage[i / width][i % width] = (static_cast<double>(resultData[i]) * 65535.0) / 255.0;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        logMessage(QString("GPU CLAHE completed in %1 ms").arg(processingTime).toStdString());

    } catch (const cv::Exception& e) {
        logMessage("Error in GPU CLAHE: " + std::string(e.what()));
        throw;
    }
}

void CLAHEProcessor::applyCLAHE_CPU(double** outputImage, const uint32_t* inputBuffer,
                                    int height, int width, double clipLimit, const cv::Size& tileSize) {
    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        unsigned int numThreads = threadConfig.numThreads;

        // Step 1: Convert 32-bit input (0-4294967295) to 8-bit (0-255) for CLAHE processing
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                // Scale from 32-bit to 8-bit range
                data[i] = static_cast<uchar>((inputBuffer[i] * 255.0) / 4294967295.0);
            }
        }

        cv::Mat result = cv::Mat::zeros(input8bit.size(), input8bit.type());

        // Create CLAHE instances for each thread
        std::vector<cv::Ptr<cv::CLAHE>> claheInstances(numThreads);
        for (auto& instance : claheInstances) {
            instance = cv::createCLAHE(clipLimit, tileSize);
        }

        // Step 2: Process with CPU CLAHE in parallel strips
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / numThreads;

        for (unsigned int i = 0; i < numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == numThreads - 1) ? height : (i + 1) * rowsPerThread;

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

        // Step 3: Convert 8-bit CLAHE result (0-255) back to 16-bit range (0-65535)
        const uchar* resultData = result.ptr<uchar>(0);
        int totalPixels = height * width;
        #pragma omp parallel for
        for (int i = 0; i < totalPixels; ++i) {
            outputImage[i / width][i % width] = (static_cast<double>(resultData[i]) * 65535.0) / 255.0;
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        logMessage(QString("CPU CLAHE completed in %1 ms").arg(processingTime).toStdString());

    } catch (const cv::Exception& e) {
        logMessage("Error in CPU CLAHE: " + std::string(e.what()));
        throw;
    }
}

void CLAHEProcessor::applyThresholdCLAHE(double** finalImage, const uint32_t* inputBuffer,
                                         int height, int width, uint16_t threshold,
                                         double clipLimit, const cv::Size& tileSize) {
    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        logThreadStart(0, "ThresholdCLAHE_GPU", 0, height);

        // Step 1: Convert all input data to finalImage (keeping original values)
        int totalPixels = height * width;
        #pragma omp parallel for
        for (int i = 0; i < totalPixels; ++i) {
            // Convert 32-bit (0-4294967295) to double (0-65535)
            finalImage[i / width][i % width] = (static_cast<double>(inputBuffer[i]) * 65535.0) / 4294967295.0;
        }

        // Step 2: Convert input to 8-bit for CLAHE processing
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputBuffer[i] * 255.0) / 4294967295.0);
            }
        }

        // Step 3: Create dark region mask using scaled threshold
        cv::Mat darkMask;
        uint8_t threshold8bit = static_cast<uint8_t>((threshold * 255.0) / 65535.0);
        cv::threshold(input8bit, darkMask, threshold8bit, 255, cv::THRESH_BINARY_INV);

        // Only process if there are dark regions
        if (cv::countNonZero(darkMask) > 0) {
            // Step 4: Process with GPU CLAHE
            cv::cuda::GpuMat d_input8bit, d_darkMask;
            d_input8bit.upload(input8bit);
            d_darkMask.upload(darkMask);

            cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(clipLimit, tileSize);
            cv::cuda::GpuMat d_processed;
            clahe->apply(d_input8bit, d_processed);

            cv::Mat processed;
            d_processed.download(processed);

            // Step 5: Apply CLAHE results only to dark regions
            if (processed.isContinuous() && darkMask.isContinuous()) {
                const uchar* procData = processed.ptr<uchar>(0);
                const uchar* maskData = darkMask.ptr<uchar>(0);
                int totalPixels = height * width;
                #pragma omp parallel for
                for (int i = 0; i < totalPixels; ++i) {
                    if (maskData[i] > 0) {
                        // Update only dark regions, keeping bright regions unchanged
                        finalImage[i / width][i % width] =
                            (static_cast<double>(procData[i]) * 65535.0) / 255.0;
                    }
                    // Bright regions retain their original values from Step 1
                }
            }
        }
        // If no dark regions, the original values from Step 1 are preserved

        auto endTime = std::chrono::high_resolution_clock::now();
        double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        logMessage(QString("GPU Threshold CLAHE completed in %1 ms").arg(processingTime).toStdString());

    } catch (const cv::Exception& e) {
        logMessage("Error in GPU Threshold CLAHE: " + std::string(e.what()));
        throw;
    }
}

void CLAHEProcessor::applyThresholdCLAHE_CPU(double** finalImage, const uint32_t* inputBuffer,
                                             int height, int width, uint16_t threshold,
                                             double clipLimit, const cv::Size& tileSize) {
    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        unsigned int numThreads = threadConfig.numThreads;
        logThreadStart(0, "ThresholdCLAHE_CPU", 0, height);

        // Step 1: Convert all input data to finalImage (keeping original values)
        int totalPixels = height * width;
        #pragma omp parallel for
        for (int i = 0; i < totalPixels; ++i) {
            // Convert 32-bit (0-4294967295) to double (0-65535)
            finalImage[i / width][i % width] = (static_cast<double>(inputBuffer[i]) * 65535.0) / 4294967295.0;
        }

        // Step 2: Convert input to 8-bit for CLAHE processing
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputBuffer[i] * 255.0) / 4294967295.0);
            }
        }

        // Step 3: Create dark region mask using scaled threshold
        cv::Mat darkMask;
        uint8_t threshold8bit = static_cast<uint8_t>((threshold * 255.0) / 65535.0);
        cv::threshold(input8bit, darkMask, threshold8bit, 255, cv::THRESH_BINARY_INV);

        // Only process if there are dark regions
        if (cv::countNonZero(darkMask) > 0) {
            // Step 4: Process with CPU CLAHE
            cv::Mat processed;
            cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
            clahe->apply(input8bit, processed);

            // Step 5: Apply CLAHE results only to dark regions
            if (processed.isContinuous() && darkMask.isContinuous()) {
                const uchar* procData = processed.ptr<uchar>(0);
                const uchar* maskData = darkMask.ptr<uchar>(0);
                int totalPixels = height * width;
                #pragma omp parallel for
                for (int i = 0; i < totalPixels; ++i) {
                    if (maskData[i] > 0) {
                        // Update only dark regions, keeping bright regions unchanged
                        finalImage[i / width][i % width] =
                            (static_cast<double>(procData[i]) * 65535.0) / 255.0;
                    }
                    // Bright regions retain their original values from Step 1
                }
            }
        }
        // If no dark regions, the original values from Step 1 are preserved

        auto endTime = std::chrono::high_resolution_clock::now();
        double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        logMessage(QString("CPU Threshold CLAHE completed in %1 ms").arg(processingTime).toStdString());

    } catch (const cv::Exception& e) {
        logMessage("Error in CPU Threshold CLAHE: " + std::string(e.what()));
        throw;
    }
}

void CLAHEProcessor::applyCombinedCLAHE(double** outputImage, const uint32_t* inputBuffer,
                                        int height, int width, double clipLimit,
                                        const cv::Size& tileSize) {
    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        logThreadStart(0, "CombinedCLAHE_GPU", 0, height);

        // Step 1: Convert 32-bit input (0-4294967295) to 8-bit (0-255) for CLAHE processing
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputBuffer[i] * 255.0) / 4294967295.0);
            }
        }

        // Step 2: Process with GPU CLAHE
        cv::cuda::GpuMat gpuInput;
        gpuInput.upload(input8bit);
        cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat gpuResult;
        gpuClahe->apply(gpuInput, gpuResult);

        // Download result
        cv::Mat processed;
        gpuResult.download(processed);

        // Step 3: Compare and take maximum between original and CLAHE-processed values
        if (processed.isContinuous()) {
            const uchar* procData = processed.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                // Convert original 32-bit value to 16-bit range
                double originalValue = (static_cast<double>(inputBuffer[i]) * 65535.0) / 4294967295.0;

                // Convert processed 8-bit value to 16-bit range
                double processedValue = (static_cast<double>(procData[i]) * 65535.0) / 255.0;

                // Take maximum value
                outputImage[i / width][i % width] = std::max(originalValue, processedValue);
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        logMessage(QString("GPU Combined CLAHE completed in %1 ms").arg(processingTime).toStdString());

    } catch (const cv::Exception& e) {
        logMessage("Error in GPU Combined CLAHE: " + std::string(e.what()));
        throw;
    }
}

void CLAHEProcessor::applyCombinedCLAHE_CPU(double** outputImage, const uint32_t* inputBuffer,
                                            int height, int width, double clipLimit,
                                            const cv::Size& tileSize) {
    try {
        auto startTime = std::chrono::high_resolution_clock::now();
        unsigned int numThreads = threadConfig.numThreads;
        logThreadStart(0, "CombinedCLAHE_CPU", 0, height);

        // Step 1: Convert 32-bit input (0-4294967295) to 8-bit (0-255) for CLAHE processing
        cv::Mat input8bit(height, width, CV_8UC1);
        if (input8bit.isContinuous()) {
            uchar* data = input8bit.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                data[i] = static_cast<uchar>((inputBuffer[i] * 255.0) / 4294967295.0);
            }
        }

        // Step 2: Process with CPU CLAHE
        cv::Mat processed;
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
        clahe->apply(input8bit, processed);

        // Step 3: Compare and take maximum between original and CLAHE-processed values
        if (processed.isContinuous()) {
            const uchar* procData = processed.ptr<uchar>(0);
            int totalPixels = height * width;
            #pragma omp parallel for
            for (int i = 0; i < totalPixels; ++i) {
                // Convert original 32-bit value to 16-bit range
                double originalValue = (static_cast<double>(inputBuffer[i]) * 65535.0) / 4294967295.0;

                // Convert processed 8-bit value to 16-bit range
                double processedValue = (static_cast<double>(procData[i]) * 65535.0) / 255.0;

                // Take maximum value
                outputImage[i / width][i % width] = std::max(originalValue, processedValue);
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        double processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        logMessage(QString("CPU Combined CLAHE completed in %1 ms").arg(processingTime).toStdString());

    } catch (const cv::Exception& e) {
        logMessage("Error in CPU Combined CLAHE: " + std::string(e.what()));
        throw;
    }
}
