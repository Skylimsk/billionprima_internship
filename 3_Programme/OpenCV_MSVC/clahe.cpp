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

std::mutex CLAHEProcessor::logMutex;

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

// Vector/Matrix conversion functions
cv::Mat CLAHEProcessor::vectorToMat(const std::vector<std::vector<uint16_t>>& image) {
    int height = image.size();
    int width = image[0].size();
    cv::Mat mat(height, width, CV_16UC1);

    for (int y = 0; y < height; ++y) {
        memcpy(mat.ptr<uint16_t>(y), image[y].data(), width * sizeof(uint16_t));
    }

    return mat;
}

std::vector<std::vector<uint16_t>> CLAHEProcessor::matToVector(const cv::Mat& mat) {
    int height = mat.rows;
    int width = mat.cols;
    std::vector<std::vector<uint16_t>> image(height, std::vector<uint16_t>(width));

    for (int y = 0; y < height; ++y) {
        memcpy(image[y].data(), mat.ptr<uint16_t>(y), width * sizeof(uint16_t));
    }

    return image;
}

cv::Mat CLAHEProcessor::applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads; // GPU processing
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Log start of processing
        logThreadStart(0, "CLAHE_GPU", 0, inputImage.rows);

        // Convert and upload to GPU
        cv::Mat input8bit = convertTo8Bit(inputImage);
        cv::cuda::GpuMat gpuInput;
        gpuInput.upload(input8bit);

        // Apply CLAHE on GPU
        cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat gpuResult;
        gpuClahe->apply(gpuInput, gpuResult);

        // Download and convert result
        cv::Mat result;
        gpuResult.download(result);
        cv::Mat output16bit = convertTo16Bit(result);
        preserveValueRange(inputImage, output16bit);

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

        // Log completion
        logThreadComplete(0, "CLAHE_GPU");

        return output16bit;
    }
    catch (const cv::Exception& e) {
        throw;
    }
}

cv::Mat CLAHEProcessor::applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        cv::Mat input8bit = convertTo8Bit(inputImage);
        cv::Mat result = cv::Mat::zeros(input8bit.size(), input8bit.type());

        std::vector<cv::Ptr<cv::CLAHE>> claheInstances(threadConfig.numThreads);
        for (auto& instance : claheInstances) {
            instance = cv::createCLAHE(clipLimit, tileSize);
        }

        std::vector<std::future<void>> futures;
        int rowsPerThread = input8bit.rows / threadConfig.numThreads;
        std::atomic<int> completedRows{0};

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? input8bit.rows : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "CLAHE_CPU", startRow, endRow);

                cv::Mat strip = input8bit(cv::Range(startRow, endRow), cv::Range::all());
                cv::Mat stripResult;
                claheInstances[i]->apply(strip, stripResult);

                {
                    std::lock_guard<std::mutex> lock(processingMutex);
                    stripResult.copyTo(result(cv::Range(startRow, endRow), cv::Range::all()));
                }

                auto endTime = std::chrono::high_resolution_clock::now();
                metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                metrics.totalTime = metrics.processingTime;

                logThreadComplete(i, "CLAHE_CPU");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }

        cv::Mat output16bit = convertTo16Bit(result);
        preserveValueRange(inputImage, output16bit);

        return output16bit;
    }
    catch (const cv::Exception& e) {
        throw;
    }
}

void CLAHEProcessor::applyThresholdCLAHE_GPU(std::vector<std::vector<uint16_t>>& finalImage,
                                             uint16_t threshold, double clipLimit,
                                             const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads; // GPU processing
    auto startTime = std::chrono::high_resolution_clock::now();

    // Convert vector to Mat
    cv::Mat matImage = vectorToMat(finalImage);
    int height = matImage.rows;
    int width = matImage.cols;

    // Create dark mask with transition zone using multiple threads
    cv::Mat cpuDarkMask = cv::Mat::zeros(matImage.size(), CV_32F);
    float transitionRange = threshold * 0.35f;

    {
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "Dark Mask Creation", startRow, endRow);
                createDarkMask(matImage, cpuDarkMask, threshold, transitionRange,
                               startRow, endRow, i);
                logThreadComplete(i, "Dark Mask Creation");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }
    }

    // Upload mask to GPU and apply Gaussian blur
    cv::cuda::GpuMat darkMask;
    darkMask.upload(cpuDarkMask);

    cv::cuda::GpuMat tempMask;
    cv::Ptr<cv::cuda::Filter> gaussianFilter1 = cv::cuda::createGaussianFilter(
        darkMask.type(), darkMask.type(), cv::Size(7, 7), 1.5);
    cv::Ptr<cv::cuda::Filter> gaussianFilter2 = cv::cuda::createGaussianFilter(
        darkMask.type(), darkMask.type(), cv::Size(5, 5), 1.0);

    gaussianFilter1->apply(darkMask, tempMask);
    gaussianFilter2->apply(tempMask, darkMask);

    // Setup CLAHE processing
    cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit * 0.6, tileSize);
    cv::cuda::GpuMat gpuClaheResult;
    cv::cuda::GpuMat darkPixels;
    darkPixels.upload(matImage);

    cv::Mat darkMaskVis;
    cv::Mat cpuDarkMask2;
    darkMask.download(cpuDarkMask2);
    cpuDarkMask2.convertTo(darkMaskVis, CV_8U, 255.0);

    // Apply CLAHE if dark regions are found
    if (cv::countNonZero(darkMaskVis) > 0) {
        logThreadStart(0, "CLAHE GPU Processing", 0, height);
        cv::cuda::GpuMat validPixels;
        darkPixels.copyTo(validPixels, darkMask);
        gpuClahe->apply(validPixels, gpuClaheResult);
        logThreadComplete(0, "CLAHE GPU Processing");

        // Download for CPU sharpening
        cv::Mat cpuResult;
        gpuClaheResult.download(cpuResult);

        logThreadStart(0, "Sharpening Filter", 0, height);
        cv::Mat sharpenKernel = (cv::Mat_<float>(3,3) <<
                                     0, -0.3, 0,
                                 -0.3, 2.2, -0.3,
                                 0, -0.3, 0);

        cv::Mat sharpened;
        cv::filter2D(cpuResult, sharpened, CV_16U, sharpenKernel);

        cv::cuda::GpuMat gpuSharpened;
        gpuSharpened.upload(sharpened);
        cv::cuda::addWeighted(gpuClaheResult, 0.5, gpuSharpened, 0.5, 0, gpuClaheResult);
        logThreadComplete(0, "Sharpening Filter");
    } else {
        darkPixels.copyTo(gpuClaheResult);
    }

    // Process final image blending
    cv::Mat resultImage = matImage.clone();
    cv::Mat cpuClaheResult;
    gpuClaheResult.download(cpuClaheResult);

    {
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "Progressive Blending", startRow, endRow);
                processImageChunk(resultImage, matImage, cpuClaheResult, cpuDarkMask2,
                                  threshold, startRow, endRow, i);
                logThreadComplete(i, "Progressive Blending");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }
    }

    // Final GPU processing steps
    logThreadStart(0, "Final GPU Processing", 0, height);
    cv::cuda::GpuMat gpuResult;
    gpuResult.upload(resultImage);

    cv::Mat normalizedResult;
    cv::normalize(resultImage, normalizedResult, 0, 65535, cv::NORM_MINMAX, CV_16U);
    cv::cuda::GpuMat gpuNormalized;
    gpuNormalized.upload(normalizedResult);

    cv::cuda::addWeighted(gpuResult, 0.8, gpuNormalized, 0.2, 0, gpuResult);

    cv::Ptr<cv::cuda::Filter> finalBlur = cv::cuda::createGaussianFilter(
        gpuResult.type(), gpuResult.type(), cv::Size(3, 3), 0.5);
    finalBlur->apply(gpuResult, gpuResult);

    auto endTime = std::chrono::high_resolution_clock::now();
    metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    metrics.totalTime = metrics.processingTime;

    gpuResult.download(resultImage);
    logThreadComplete(0, "Final GPU Processing");

    // Update the final image
    finalImage = matToVector(resultImage);
}

void CLAHEProcessor::applyThresholdCLAHE_CPU(std::vector<std::vector<uint16_t>>& finalImage,
                                             uint16_t threshold, double clipLimit,
                                             const cv::Size& tileSize,
                                             const ThreadConfig& threadConfig) {
    metrics.reset();
    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();    metrics.isCLAHE = true;
    metrics.threadsUsed = threadConfig.numThreads;

    // Convert vector to Mat
    cv::Mat matImage = vectorToMat(finalImage);
    int height = matImage.rows;
    int width = matImage.cols;

    // Create mask with transition zone using multiple threads
    cv::Mat darkMask = cv::Mat::zeros(matImage.size(), CV_32F);
    cv::Mat darkPixels = matImage.clone();
    float transitionRange = threshold * 0.35f;

    {
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "Dark Mask Creation", startRow, endRow);
                createDarkMask(matImage, darkMask, threshold, transitionRange,
                               startRow, endRow, i);
                logThreadComplete(i, "Dark Mask Creation");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }
    }

    // Multi-pass Gaussian smoothing
    cv::GaussianBlur(darkMask, darkMask, cv::Size(7, 7), 1.5);
    cv::GaussianBlur(darkMask, darkMask, cv::Size(5, 5), 1.0);

    // Apply CLAHE with parallel processing
    cv::Mat claheResult;
    cv::Mat darkMaskVis;
    darkMask.convertTo(darkMaskVis, CV_8U, 255.0);

    if (cv::countNonZero(darkMaskVis) > 0) {
        std::vector<cv::Ptr<cv::CLAHE>> claheInstances(threadConfig.numThreads);
        for (auto& instance : claheInstances) {
            instance = cv::createCLAHE(clipLimit * 0.6, tileSize);
        }

        cv::Mat validPixels;
        darkPixels.copyTo(validPixels, darkMaskVis);
        claheResult = cv::Mat::zeros(validPixels.size(), validPixels.type());

        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "CLAHE Processing", startRow, endRow);
                cv::Mat strip = validPixels(cv::Range(startRow, endRow), cv::Range::all());
                cv::Mat stripResult;

                claheInstances[i]->apply(strip, stripResult);

                {
                    std::lock_guard<std::mutex> lock(processingMutex);
                    stripResult.copyTo(claheResult(cv::Range(startRow, endRow), cv::Range::all()));
                }
                logThreadComplete(i, "CLAHE Processing");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }

        // Apply sharpening
        cv::Mat sharpenKernel = (cv::Mat_<float>(3,3) <<
                                     0, -0.3, 0,
                                 -0.3, 2.2, -0.3,
                                 0, -0.3, 0);

        cv::Mat sharpened;
        cv::filter2D(claheResult, sharpened, CV_16U, sharpenKernel);

        float sharpenBlend = 0.5f;
        cv::addWeighted(claheResult, 1.0 - sharpenBlend, sharpened, sharpenBlend, 0, claheResult);
    } else {
        claheResult = darkPixels.clone();
    }

    // Progressive blending with parallel processing
    cv::Mat resultImage = matImage.clone();

    {
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "Progressive Blending", startRow, endRow);
                processImageChunk(resultImage, matImage, claheResult, darkMask,
                                  threshold, startRow, endRow, i);
                logThreadComplete(i, "Progressive Blending");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }
    }

    // Final processing steps
    cv::Mat normalizedResult;
    cv::normalize(resultImage, normalizedResult, 0, 65535, cv::NORM_MINMAX, CV_16U);

    float normalizationStrength = 0.2f;
    cv::addWeighted(resultImage, 1.0 - normalizationStrength,
                    normalizedResult, normalizationStrength, 0, resultImage);

    cv::GaussianBlur(resultImage, resultImage, cv::Size(3, 3), 0.5);

    auto endTime = std::chrono::high_resolution_clock::now();
    metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    metrics.totalTime = metrics.processingTime;

    // Update the final image
    finalImage = matToVector(resultImage);
}

void CLAHEProcessor::processImageChunk(cv::Mat& result, const cv::Mat& original,
                                       const cv::Mat& claheResult, const cv::Mat& darkMask,
                                       uint16_t threshold, int startRow, int endRow,
                                       int threadId) {
    const int width = result.cols;

    for (int y = startRow; y < endRow; ++y) {
        for (int x = 0; x < width; ++x) {
            float blendFactor = darkMask.at<float>(y, x);
            if (blendFactor > 0) {
                uint16_t originalValue = original.at<uint16_t>(y, x);
                uint16_t claheValue = claheResult.at<uint16_t>(y, x);

                float adaptiveBlend = blendFactor;
                if (originalValue < threshold * 0.6) {
                    float darknessFactor = originalValue / (threshold * 0.6f);
                    adaptiveBlend *= 0.7f + 0.3f * darknessFactor;
                }

                result.at<uint16_t>(y, x) = static_cast<uint16_t>(
                    originalValue * (1.0f - adaptiveBlend) +
                    claheValue * adaptiveBlend
                    );
            }
        }
    }
}

void CLAHEProcessor::createDarkMask(const cv::Mat& input, cv::Mat& darkMask,
                                    uint16_t threshold, float transitionRange,
                                    int startRow, int endRow,
                                    int threadId) {
    const int width = input.cols;

    for (int y = startRow; y < endRow; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t pixelValue = input.at<uint16_t>(y, x);
            if (pixelValue < threshold) {
                if (pixelValue > threshold - transitionRange) {
                    float factor = (threshold - pixelValue) / transitionRange;
                    factor = factor * factor * factor * (factor * (6 * factor - 15) + 10);
                    darkMask.at<float>(y, x) = factor;
                } else {
                    darkMask.at<float>(y, x) = 0.8f +
                                               0.2f * (pixelValue / (threshold - transitionRange));
                }
            }
        }
    }
}
