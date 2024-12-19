#ifdef _MSC_VER
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4244) // conversion from 'double' to 'float', possible loss of data
#pragma warning(disable: 4458) // declaration hides class member
#endif

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

CLAHEProcessor::CLAHEProcessor()
    : threadConfig(std::thread::hardware_concurrency(), 64) {
    clahe = cv::createCLAHE();
    metrics.reset();
}

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

void CLAHEProcessor::logThreadComplete(int threadId, const std::string& function) {
    std::lock_guard<std::mutex> lock(logMutex);
    qDebug() << QString("Thread ID %1 completed processing %2")
                    .arg(threadId)
                    .arg(function.c_str())
                    .toStdString().c_str();
}

void CLAHEProcessor::logMessage(const std::string& message, int threadId) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::stringstream ss;
    ss << "[Thread " << std::setw(2) << threadId << "] " << message;
    qDebug() << ss.str().c_str();
}

double** CLAHEProcessor::allocateImageBuffer(int height, int width) {
    double** buffer = new double*[height];
    for (int i = 0; i < height; i++) {
        buffer[i] = new double[width]();  // () initializes to zero
    }
    return buffer;
}

void CLAHEProcessor::deallocateImageBuffer(double** buffer, int height) {
    if (buffer) {
        for (int i = 0; i < height; i++) {
            delete[] buffer[i];
        }
        delete[] buffer;
    }
}

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

        // Convert normalized double input to Mat (scale to 8-bit for CLAHE)
        cv::Mat matInput(height, width, CV_64F);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                matInput.at<double>(y, x) = inputImage[y][x];
            }
        }

        // Convert to 8-bit for CLAHE processing
        cv::Mat input8bit;
        matInput.convertTo(input8bit, CV_8UC1, 255.0);

        // Apply CLAHE
        cv::cuda::GpuMat gpuInput;
        gpuInput.upload(input8bit);

        cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat gpuResult;
        gpuClahe->apply(gpuInput, gpuResult);

        // Download result
        cv::Mat result;
        gpuResult.download(result);

        // Convert back to double range (normalized 0-1)
        cv::Mat resultDouble;
        result.convertTo(resultDouble, CV_64F, 1.0/255.0);

        // Copy to output
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                outputImage[y][x] = resultDouble.at<double>(y, x);
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

void CLAHEProcessor::applyCLAHE_CPU(double** outputImage, double** inputImage,
                                    int height, int width, double clipLimit,
                                    const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert normalized double input to Mat
        cv::Mat matInput(height, width, CV_64F);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                matInput.at<double>(y, x) = inputImage[y][x];
            }
        }

        // Convert to 8-bit for CLAHE processing
        cv::Mat input8bit;
        matInput.convertTo(input8bit, CV_8UC1, 255.0);
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

        // Convert back to double range (normalized 0-1)
        cv::Mat resultDouble;
        result.convertTo(resultDouble, CV_64F, 1.0/255.0);

        // Copy to output
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                outputImage[y][x] = resultDouble.at<double>(y, x);
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

void CLAHEProcessor::applyCombinedCLAHE(double** outputImage, double** inputImage,
                                        int height, int width, double clipLimit,
                                        const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // 1. Convert input array to Mat format
        cv::Mat matImage(height, width, CV_64F);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                matImage.at<double>(y, x) = inputImage[y][x];
            }
        }

        // 2. Convert to 16-bit
        cv::Mat matImage16U;
        matImage.convertTo(matImage16U, CV_16UC1, 65535.0);

        // 3. Upload to GPU and apply CLAHE
        cv::cuda::GpuMat d_image16U(matImage16U);
        cv::cuda::GpuMat d_image8bit;
        d_image16U.convertTo(d_image8bit, CV_8UC1, 255.0/65535.0);

        // Create GPU CLAHE object
        cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat d_processed8bit;
        clahe->apply(d_image8bit, d_processed8bit);

        // 4. Convert CLAHE result back to 16-bit on GPU
        cv::cuda::GpuMat d_processed16U;
        d_processed8bit.convertTo(d_processed16U, CV_16UC1, 65535.0/255.0);

        // 5. Compare and select brighter pixels on GPU
        cv::cuda::GpuMat d_result;
        cv::cuda::max(d_image16U, d_processed16U, d_result);

        // Download result from GPU
        cv::Mat result;
        d_result.download(result);

        // 6. Convert back to double format
        cv::Mat resultDouble;
        result.convertTo(resultDouble, CV_64F, 1.0/65535.0);

        // 7. Copy back to original array
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                outputImage[y][x] = resultDouble.at<double>(y, x);
            }
        }

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;
    }
    catch (const cv::Exception& e) {
        qDebug() << "Error in GPU CLAHE processing:" << e.what();
        throw;
    }
}

void CLAHEProcessor::applyThresholdCLAHE(double** finalImage, int height, int width,
                                             uint16_t threshold, double clipLimit,
                                             const cv::Size& tileSize, bool afterNormalCLAHE) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = 1; // GPU version uses 1 host thread
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Allocate GPU memory and copy input image
        cv::cuda::GpuMat d_matImage(height, width, CV_16UC1);
        cv::Mat h_matImage(height, width, CV_16UC1);

        // Convert input to Mat format on CPU first
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                h_matImage.at<uint16_t>(y, x) = static_cast<uint16_t>(finalImage[y][x] * 65535.0);
            }
        }
        d_matImage.upload(h_matImage);

        // Create dark region mask on GPU
        cv::cuda::GpuMat d_darkMask;
        cv::cuda::threshold(d_matImage, d_darkMask, threshold, 255, cv::THRESH_BINARY_INV);
        d_darkMask.convertTo(d_darkMask, CV_8UC1);

        // Check if there are dark regions to process
        cv::Mat h_darkMask;
        d_darkMask.download(h_darkMask);
        if (cv::countNonZero(h_darkMask) > 0) {
            // Extract dark regions
            cv::cuda::GpuMat d_darkRegion;
            d_matImage.copyTo(d_darkRegion, d_darkMask);

            // Convert to 8-bit for CLAHE
            cv::cuda::GpuMat d_darkRegion8bit;
            d_darkRegion.convertTo(d_darkRegion8bit, CV_8UC1, 255.0/65535.0);

            // Adjust CLAHE parameters
            double adjustedClipLimit = clipLimit * 0.8;
            cv::Size adjustedTileSize(tileSize.width * 2, tileSize.height * 2);

            // Apply CLAHE to dark regions
            cv::cuda::GpuMat d_processedDark8bit;
            cv::Ptr<cv::cuda::CLAHE> clahe = cv::cuda::createCLAHE(adjustedClipLimit, adjustedTileSize);
            clahe->apply(d_darkRegion8bit, d_processedDark8bit);

            // Calculate value range
            double minVal, maxVal;
            cv::Mat h_processedDark8bit;
            d_processedDark8bit.download(h_processedDark8bit);
            cv::minMaxLoc(h_processedDark8bit, &minVal, &maxVal);

            // Adjust value range stretching on GPU
            cv::cuda::GpuMat d_processedDark16;
            double stretchFactor = 0.7;
            double scale = (65535.0 / (maxVal - minVal)) * stretchFactor;
            double offset = -minVal * scale;
            d_processedDark8bit.convertTo(d_processedDark16, CV_16UC1, scale, offset);

            // Merge results on GPU
            cv::cuda::GpuMat d_result;
            d_matImage.copyTo(d_result);
            d_processedDark16.copyTo(d_result, d_darkMask);

            // Download result and convert back to normalized double range
            cv::Mat h_result;
            d_result.download(h_result);
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    finalImage[y][x] = h_result.at<uint16_t>(y, x) / 65535.0;
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

void CLAHEProcessor::applyThresholdCLAHE_CPU(double** finalImage, int height, int width,
                                             uint16_t threshold, double clipLimit,
                                             const cv::Size& tileSize, bool afterNormalCLAHE) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Convert to Mat format
        cv::Mat matImage(height, width, CV_16UC1);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                matImage.at<uint16_t>(y, x) = static_cast<uint16_t>(finalImage[y][x] * 65535.0);
            }
        }

        // Create dark region mask
        cv::Mat darkMask;
        cv::threshold(matImage, darkMask, threshold, 255, cv::THRESH_BINARY_INV);
        darkMask.convertTo(darkMask, CV_8UC1);

        if (cv::countNonZero(darkMask) > 0) {
            // Extract dark regions
            cv::Mat darkRegion;
            matImage.copyTo(darkRegion, darkMask);

            // Convert to 8-bit for CLAHE
            cv::Mat darkRegion8bit;
            darkRegion.convertTo(darkRegion8bit, CV_8UC1, 255.0/65535.0);

            // Adjust CLAHE parameters
            double adjustedClipLimit = clipLimit * 0.8;
            cv::Size adjustedTileSize(tileSize.width * 2, tileSize.height * 2);

            // Apply CLAHE to dark regions
            cv::Mat processedDark8bit;
            cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(adjustedClipLimit, adjustedTileSize);
            clahe->apply(darkRegion8bit, processedDark8bit);

            // Calculate value range
            double minVal, maxVal;
            cv::minMaxLoc(processedDark8bit, &minVal, &maxVal);

            // Adjust value range stretching
            cv::Mat processedDark16;
            double stretchFactor = 0.7;
            double scale = (65535.0 / (maxVal - minVal)) * stretchFactor;
            double offset = -minVal * scale;
            processedDark8bit.convertTo(processedDark16, CV_16UC1, scale, offset);

            // Merge results
            cv::Mat result = matImage.clone();
            processedDark16.copyTo(result, darkMask);

            // Convert back to normalized double range
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    finalImage[y][x] = result.at<uint16_t>(y, x) / 65535.0;
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
