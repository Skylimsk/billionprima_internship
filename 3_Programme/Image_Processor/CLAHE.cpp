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

// Conversion helper functions
cv::Mat CLAHEProcessor::convertTo8Bit(const cv::Mat& input) {
    double minVal, maxVal;
    cv::minMaxLoc(input, &minVal, &maxVal);

    cv::Mat output;
    double scale = 255.0 / (maxVal - minVal);
    input.convertTo(output, CV_8UC1, scale, -minVal * scale);

    return output;
}

cv::Mat CLAHEProcessor::convertTo16Bit(const cv::Mat& input) {
    double minVal, maxVal;
    cv::minMaxLoc(input, &minVal, &maxVal);

    cv::Mat output;
    double scale = 65535.0 / (maxVal - minVal);
    input.convertTo(output, CV_16UC1, scale, -minVal * scale);

    return output;
}

void CLAHEProcessor::preserveValueRange(const cv::Mat& original, cv::Mat& processed) {
    double origMin, origMax, procMin, procMax;
    cv::minMaxLoc(original, &origMin, &origMax);
    cv::minMaxLoc(processed, &procMin, &procMax);

    double scale = (origMax - origMin) / (procMax - procMin);
    processed.convertTo(processed, processed.type(), scale, origMin - procMin * scale);
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

void CLAHEProcessor::applyThresholdCLAHE(double** image, int height, int width,
                                         uint16_t threshold, double clipLimit,
                                         const cv::Size& tileSize, bool afterNormalCLAHE) {
    try {
        // 1. Convert to Mat format
        cv::Mat matImage(height, width, CV_64F);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                matImage.at<double>(y, x) = image[y][x];
            }
        }

        // 2. Convert to 16-bit
        cv::Mat matImage16U;
        matImage.convertTo(matImage16U, CV_16UC1, 65535.0);

        // 3. Create threshold region mask - ensure conversion to 8-bit
        cv::Mat thresholdMask;
        cv::threshold(matImage16U, thresholdMask, threshold, 255, cv::THRESH_BINARY);
        thresholdMask.convertTo(thresholdMask, CV_8U);

        // 4. Get original image range
        double minVal, maxVal;
        cv::minMaxLoc(matImage16U, &minVal, &maxVal);

        // 5. Extract threshold and non-threshold regions
        cv::Mat thresholdRegion, nonThresholdRegion;
        matImage16U.copyTo(thresholdRegion, thresholdMask);
        matImage16U.copyTo(nonThresholdRegion, ~thresholdMask);

        // 6. Apply CLAHE to threshold region
        cv::Mat thresholdRegion8bit;
        thresholdRegion.convertTo(thresholdRegion8bit, CV_8UC1, 255.0/65535.0);
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
        cv::Mat processedThreshold8bit;
        clahe->apply(thresholdRegion8bit, processedThreshold8bit);

        // 7. Convert back to 16-bit and remap to wider range
        cv::Mat processedThreshold;
        processedThreshold8bit.convertTo(processedThreshold, CV_16UC1, 65535.0/255.0);

        // 8. Calculate new range: using original image range as reference
        double targetMin = minVal;  // Use original minimum value
        double targetMax = maxVal * 1.5;  // Allow maximum to expand to 1.5 times original maximum
        double thresholdMin, thresholdMax;
        cv::minMaxLoc(processedThreshold, &thresholdMin, &thresholdMax, nullptr, nullptr, thresholdMask);

        // Linear mapping to new range
        processedThreshold = (processedThreshold - thresholdMin) * (targetMax - targetMin) /
                                 (thresholdMax - thresholdMin) + targetMin;

        // 9. Collect dark region pixels and distribute more boldly
        std::vector<uint16_t> darkPixels;
        uint16_t blackThreshold = 10000;  // Raise dark region threshold to collect more dark pixels
        uint16_t whiteThreshold = 60000;  // Protect areas near pure white

        // Collect dark pixels only from threshold region
        for(int y = 0; y < height; y++) {
            for(int x = 0; x < width; x++) {
                if(thresholdMask.at<uint8_t>(y, x) > 0 &&  // Within threshold region
                    processedThreshold.at<uint16_t>(y, x) < blackThreshold) {
                    darkPixels.push_back(processedThreshold.at<uint16_t>(y, x));
                }
            }
        }

        // 10. More aggressively distribute dark values
        cv::Mat result = matImage16U.clone();
        if(!darkPixels.empty()) {
            double totalDarkness = std::accumulate(darkPixels.begin(), darkPixels.end(), 0.0);
            int totalPixels = height * width;
            // Increase darkness impact
            double darknessPerPixel = (totalDarkness / totalPixels) * 3.0;  // Double darkness impact

            for(int y = 0; y < height; y++) {
                for(int x = 0; x < width; x++) {
                    // Merge threshold region processing results
                    if(thresholdMask.at<uint8_t>(y, x) > 0) {
                        result.at<uint16_t>(y, x) = processedThreshold.at<uint16_t>(y, x);
                    }

                    // Add dark values to non-white regions
                    if(result.at<uint16_t>(y, x) < whiteThreshold) {
                        // Dynamically adjust dark value addition based on pixel value
                        double darknessFactor = 1.0 - (result.at<uint16_t>(y, x) / static_cast<double>(whiteThreshold));
                        result.at<uint16_t>(y, x) += static_cast<uint16_t>(darknessPerPixel * darknessFactor);
                    }
                }
            }
        }

        // 11. Convert back to double and output debug information
        cv::Mat resultDouble;
        result.convertTo(resultDouble, CV_64F, 1.0/65535.0);
        cv::minMaxLoc(resultDouble, &minVal, &maxVal);

        // 12. Copy back to original array
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                image[y][x] = resultDouble.at<double>(y, x);
            }
        }
    }
    catch (const cv::Exception& e) {
        qDebug() << "Error in threshold CLAHE processing:" << e.what();
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
        // 如果需要，先应用普通CLAHE
        if (afterNormalCLAHE) {
            double** tempOutput = allocateImageBuffer(height, width);
            applyCLAHE_CPU(tempOutput, finalImage, height, width, clipLimit, tileSize);

            // 复制结果回 finalImage
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    finalImage[y][x] = tempOutput[y][x];
                }
            }
            deallocateImageBuffer(tempOutput, height);
        }

        // 1. 创建输入Mat
        cv::Mat matImage(height, width, CV_64F);  // 修改变量名为 matImage
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                matImage.at<double>(y, x) = finalImage[y][x];  // 使用正确的变量名
            }
        }

        // 2. 转换为16位以便应用阈值
        cv::Mat matImage16U;
        matImage.convertTo(matImage16U, CV_16UC1, 65535.0);

        // 3. 创建掩码（标识需要处理的暗区域）
        cv::Mat darkMask;
        cv::threshold(matImage16U, darkMask, threshold, 255, cv::THRESH_BINARY_INV);
        darkMask.convertTo(darkMask, CV_8U);

        // 4. 只提取需要处理的暗区域
        cv::Mat darkRegion;
        matImage16U.copyTo(darkRegion, darkMask);

        if (cv::countNonZero(darkMask) > 0) {
            // 5. 创建CLAHE实例并处理暗区域
            cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
            cv::Mat processedDark;
            clahe->apply(darkRegion, processedDark);

            // 6. 合并结果：保持原图中超过阈值的部分不变，只更新处理过的暗区域
            cv::Mat result = matImage16U.clone();
            processedDark.copyTo(result, darkMask);

            // 7. 转换回double格式并复制到输出
            cv::Mat resultDouble;
            result.convertTo(resultDouble, CV_64F, 1.0/65535.0);

            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    finalImage[y][x] = resultDouble.at<double>(y, x);
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

DarkPixelInfo** CLAHEProcessor::createDarkPixelBuffer(int maxSize) {
    DarkPixelInfo** buffer = new DarkPixelInfo*[maxSize];
    for (int i = 0; i < maxSize; ++i) {
        buffer[i] = new DarkPixelInfo();
        buffer[i]->isUsed = false;
    }
    return buffer;
}

void CLAHEProcessor::releaseDarkPixelBuffer(DarkPixelInfo** buffer, int size) {
    if (buffer) {
        for (int i = 0; i < size; ++i) {
            delete buffer[i];
        }
        delete[] buffer;
    }
}

int CLAHEProcessor::collectDarkPixels(double** image, int height, int width,
                                      uint16_t threshold, DarkPixelInfo** darkPixels) {
    int count = 0;
    double thresholdNorm = threshold / 65535.0;  // 归一化阈值

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (image[y][x] < thresholdNorm) {
                darkPixels[count]->value = image[y][x];
                darkPixels[count]->x = x;
                darkPixels[count]->y = y;
                darkPixels[count]->isUsed = true;
                count++;
            }
        }
    }
    return count;
}

void CLAHEProcessor::applyDarkPixelsBack(double** image, DarkPixelInfo** darkPixels,
                                         int darkPixelCount) {
    for (int i = 0; i < darkPixelCount; ++i) {
        if (darkPixels[i]->isUsed) {
            int x = darkPixels[i]->x;
            int y = darkPixels[i]->y;
            image[y][x] = darkPixels[i]->value;
        }
    }
}


