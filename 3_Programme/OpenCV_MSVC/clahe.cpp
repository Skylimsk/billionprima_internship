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
    metrics.threadsUsed = threadConfig.numThreads;
    auto startTime = std::chrono::high_resolution_clock::now();

    // Convert vector to Mat
    cv::Mat matImage = vectorToMat(finalImage);
    int height = matImage.rows;
    int width = matImage.cols;

    // 1. 改进的暗区检测
    cv::Mat cpuDarkMask = cv::Mat::zeros(matImage.size(), CV_32F);
    cv::Mat darkPixels = matImage.clone();
    float transitionRange = threshold * 0.2f;

    // 并行创建暗区掩码
    {
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "Enhanced Dark Mask Creation", startRow, endRow);
                createEnhancedDarkMask(matImage, cpuDarkMask, threshold, transitionRange,
                                       startRow, endRow, i);
                logThreadComplete(i, "Enhanced Dark Mask Creation");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }
    }

    // 2. 改进的模糊处理
    cv::cuda::GpuMat darkMask;
    darkMask.upload(cpuDarkMask);

    cv::cuda::GpuMat tempMask;
    cv::Ptr<cv::cuda::Filter> gaussianFilter1 = cv::cuda::createGaussianFilter(
        darkMask.type(), darkMask.type(), cv::Size(5, 5), 1.0);
    cv::Ptr<cv::cuda::Filter> gaussianFilter2 = cv::cuda::createGaussianFilter(
        darkMask.type(), darkMask.type(), cv::Size(3, 3), 0.8);

    gaussianFilter1->apply(darkMask, tempMask);
    gaussianFilter2->apply(tempMask, darkMask);

    // 3. 动态调整CLAHE参数
    double adaptiveClipLimit = clipLimit;
    cv::Size adaptiveTileSize = tileSize;

    // 基于图像特征动态调整参数
    {
        cv::Mat hist;
        int histSize = 256;
        float range[] = {0, 65536};
        const float* histRange = {range};
        cv::calcHist(&matImage, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

        float darkPixelRatio = cv::sum(hist.rowRange(0, threshold/256))[0] / cv::sum(hist)[0];

        if (darkPixelRatio > 0.3f) {
            adaptiveClipLimit *= 1.5;
            adaptiveTileSize.width = std::max(8, tileSize.width / 2);
            adaptiveTileSize.height = std::max(8, tileSize.height / 2);
        }
    }

    // 4. 改进的CLAHE处理
    cv::cuda::GpuMat gpuClaheResult;
    cv::Mat darkMaskVis;
    cv::Mat cpuDarkMask2;
    darkMask.download(cpuDarkMask2);
    cpuDarkMask2.convertTo(darkMaskVis, CV_8U, 255.0);

    cv::cuda::GpuMat gpuImage;
    gpuImage.upload(matImage);

    if (cv::countNonZero(darkMaskVis) > 0) {
        // 在GPU上创建和应用CLAHE
        cv::cuda::GpuMat validPixels;
        gpuImage.copyTo(validPixels, darkMask);

        cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(adaptiveClipLimit, adaptiveTileSize);
        gpuClahe->apply(validPixels, gpuClaheResult);

        // 5. 细节增强
        cv::cuda::GpuMat detailMask;
        cv::Ptr<cv::cuda::Filter> laplacian = cv::cuda::createLaplacianFilter(
            validPixels.type(), validPixels.type(), 1);
        laplacian->apply(validPixels, detailMask);

        // 保持原始细节
        cv::cuda::addWeighted(gpuClaheResult, 0.85, detailMask, 0.15, 0, gpuClaheResult);

        // 6. 锐化处理
        cv::Mat sharpenKernel = (cv::Mat_<float>(3,3) <<
                                     -0.1, -0.1, -0.1,
                                 -0.1,  2.0, -0.1,
                                 -0.1, -0.1, -0.1);

        cv::cuda::GpuMat gpuSharpenKernel;
        gpuSharpenKernel.upload(sharpenKernel);

        cv::cuda::GpuMat sharpened;
        cv::Ptr<cv::cuda::Filter> sharpenFilter = cv::cuda::createLinearFilter(
            gpuClaheResult.type(), gpuClaheResult.type(), sharpenKernel);
        sharpenFilter->apply(gpuClaheResult, sharpened);

        // 自适应混合
        cv::cuda::addWeighted(gpuClaheResult, 0.7, sharpened, 0.3, 0, gpuClaheResult);
    } else {
        gpuImage.copyTo(gpuClaheResult);
    }

    // 7. 改进的结果混合
    cv::cuda::GpuMat gpuResult;
    gpuImage.copyTo(gpuResult);

    cv::Mat resultImage;
    gpuResult.download(resultImage);

    // 使用CPU进行最终的图像块处理（保持与CPU版本一致）
    {
        std::vector<std::future<void>> futures;
        int rowsPerThread = height / threadConfig.numThreads;

        cv::Mat claheResult;
        gpuClaheResult.download(claheResult);

        for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
            int startRow = i * rowsPerThread;
            int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

            futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
                logThreadStart(i, "Enhanced Progressive Blending", startRow, endRow);
                processEnhancedImageChunk(resultImage, matImage, claheResult, cpuDarkMask2,
                                          threshold, startRow, endRow, i);
                logThreadComplete(i, "Enhanced Progressive Blending");
            }));
        }

        for (auto& future : futures) {
            future.wait();
        }
    }

    // 8. 最终优化
    cv::cuda::GpuMat gpuFinal;
    gpuFinal.upload(resultImage);

    cv::Ptr<cv::cuda::Filter> denoise = cv::cuda::createGaussianFilter(
        gpuFinal.type(), gpuFinal.type(), cv::Size(3, 3), 0.5);
    denoise->apply(gpuFinal, gpuFinal);

    auto endTime = std::chrono::high_resolution_clock::now();
    metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    metrics.totalTime = metrics.processingTime;

    gpuFinal.download(resultImage);
    finalImage = matToVector(resultImage);
}

// 改进的暗区掩码创建函数
void CLAHEProcessor::createEnhancedDarkMask(const cv::Mat& input, cv::Mat& darkMask,
                                            uint16_t threshold, float transitionRange,
                                            int startRow, int endRow, int threadId) {
    const int width = input.cols;
    const int kernelSize = 3;
    const int offset = kernelSize / 2;

    for (int y = startRow; y < endRow; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t pixelValue = input.at<uint16_t>(y, x);

            // 计算局部方差作为纹理特征
            float localVariance = 0.0f;
            if (y >= offset && y < input.rows - offset && x >= offset && x < width - offset) {
                float mean = 0.0f;
                float sqSum = 0.0f;
                int count = 0;

                for (int ky = -offset; ky <= offset; ++ky) {
                    for (int kx = -offset; kx <= offset; ++kx) {
                        float val = input.at<uint16_t>(y + ky, x + kx);
                        mean += val;
                        sqSum += val * val;
                        count++;
                    }
                }

                mean /= count;
                localVariance = (sqSum / count) - (mean * mean);
            }

            if (pixelValue < threshold) {
                if (pixelValue > threshold - transitionRange) {
                    // 改进的过渡函数，考虑局部方差
                    float factor = (threshold - pixelValue) / transitionRange;
                    factor = std::pow(factor, 2.0f) * (3.0f - 2.0f * factor);

                    // 根据局部方差调整因子
                    float varianceWeight = std::min(1.0f, localVariance / 10000.0f);
                    factor = factor * (0.8f + 0.2f * varianceWeight);

                    darkMask.at<float>(y, x) = factor;
                } else {
                    // 为深暗区域提供更强的增强
                    float intensity = pixelValue / static_cast<float>(threshold - transitionRange);
                    float enhancementFactor = 0.6f + 0.4f * intensity;

                    // 考虑局部方差
                    float varianceWeight = std::min(1.0f, localVariance / 10000.0f);
                    enhancementFactor *= (0.85f + 0.15f * varianceWeight);

                    darkMask.at<float>(y, x) = enhancementFactor;
                }
            }
        }
    }
}

// 改进的图像块处理函数
void CLAHEProcessor::processEnhancedImageChunk(cv::Mat& result, const cv::Mat& original,
                                               const cv::Mat& claheResult, const cv::Mat& darkMask,
                                               uint16_t threshold, int startRow, int endRow,
                                               int threadId) {
    const int width = result.cols;
    const float gamma = 1.1f;  // 轻微的gamma校正

    for (int y = startRow; y < endRow; ++y) {
        for (int x = 0; x < width; ++x) {
            float blendFactor = darkMask.at<float>(y, x);
            if (blendFactor > 0) {
                uint16_t originalValue = original.at<uint16_t>(y, x);
                uint16_t claheValue = claheResult.at<uint16_t>(y, x);

                // 改进的自适应混合
                float adaptiveBlend = blendFactor;
                if (originalValue < threshold * 0.5) {
                    float darknessFactor = originalValue / (threshold * 0.5f);
                    // 为较暗区域提供更平滑的过渡
                    adaptiveBlend *= 0.8f + 0.2f * std::pow(darknessFactor, gamma);
                }

                // 保持更多原始细节
                float detailPreservation = 1.0f - std::min(1.0f, std::abs(claheValue - originalValue) / 5000.0f);
                adaptiveBlend *= (0.85f + 0.15f * detailPreservation);

                result.at<uint16_t>(y, x) = static_cast<uint16_t>(
                    originalValue * (1.0f - adaptiveBlend) +
                    claheValue * adaptiveBlend
                    );
            }
        }
    }
}

void CLAHEProcessor::applyThresholdCLAHE_CPU(std::vector<std::vector<uint16_t>>& finalImage,
                                             uint16_t threshold, double clipLimit,
                                             const cv::Size& tileSize) {
    cv::Mat matImage = vectorToMat(finalImage);
    std::vector<PixelInfo> darkPixels;

    // 1. 收集低于阈值的像素
    for (int y = 0; y < matImage.rows; ++y) {
        for (int x = 0; x < matImage.cols; ++x) {
            uint16_t value = matImage.at<uint16_t>(y, x);
            if (value < threshold) {
                darkPixels.push_back({x, y, value});
            }
        }
    }

    if (darkPixels.empty()) {
        return;
    }

    // 2. 创建暗区图像
    cv::Mat darkImage(matImage.rows, matImage.cols, matImage.type(), cv::Scalar(0));
    for (const auto& pixel : darkPixels) {
        darkImage.at<uint16_t>(pixel.y, pixel.x) = pixel.value;
    }

    // 3. 对暗区应用CLAHE
    cv::Mat processed;
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit, tileSize);
    clahe->apply(darkImage, processed);

    // 4. 将处理后的像素放回原位置
    for (const auto& pixel : darkPixels) {
        uint16_t processedValue = processed.at<uint16_t>(pixel.y, pixel.x);
        matImage.at<uint16_t>(pixel.y, pixel.x) = processedValue;
    }

    finalImage = matToVector(matImage);
}

// void CLAHEProcessor::applyThresholdCLAHE_CPU(std::vector<std::vector<uint16_t>>& finalImage,
//                                              uint16_t threshold, double clipLimit,
//                                              const cv::Size& tileSize) {
//     metrics.reset();
//     metrics.isCLAHE = true;
//     metrics.threadsUsed = threadConfig.numThreads;
//     auto startTime = std::chrono::high_resolution_clock::now();

//     // Convert vector to Mat
//     cv::Mat matImage = vectorToMat(finalImage);
//     int height = matImage.rows;
//     int width = matImage.cols;

//     // 1. 改进的暗区检测
//     cv::Mat darkMask = cv::Mat::zeros(matImage.size(), CV_32F);
//     cv::Mat darkPixels = matImage.clone();
//     float transitionRange = threshold * 0.2f;

//     // 并行创建暗区掩码
//     {
//         std::vector<std::future<void>> futures;
//         int rowsPerThread = height / threadConfig.numThreads;

//         for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
//             int startRow = i * rowsPerThread;
//             int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

//             futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
//                 logThreadStart(i, "Enhanced Dark Mask Creation", startRow, endRow);
//                 createEnhancedDarkMask(matImage, darkMask, threshold, transitionRange,
//                                        startRow, endRow, i);
//                 logThreadComplete(i, "Enhanced Dark Mask Creation");
//             }));
//         }

//         for (auto& future : futures) {
//             future.wait();
//         }
//     }

//     // 2. 改进的模糊处理 - 转换为CPU版本
//     cv::Mat tempMask;
//     cv::GaussianBlur(darkMask, tempMask, cv::Size(5, 5), 1.0);
//     cv::GaussianBlur(tempMask, darkMask, cv::Size(3, 3), 0.8);

//     // 3. 动态调整CLAHE参数
//     double adaptiveClipLimit = clipLimit;
//     cv::Size adaptiveTileSize = tileSize;

//     // 基于图像特征动态调整参数
//     {
//         cv::Mat hist;
//         int histSize = 256;
//         float range[] = {0, 65536};
//         const float* histRange = {range};
//         cv::calcHist(&matImage, 1, 0, cv::Mat(), hist, 1, &histSize, &histRange);

//         float darkPixelRatio = cv::sum(hist.rowRange(0, threshold/256))[0] / cv::sum(hist)[0];

//         if (darkPixelRatio > 0.3f) {
//             adaptiveClipLimit *= 1.5;
//             adaptiveTileSize.width = std::max(8, tileSize.width / 2);
//             adaptiveTileSize.height = std::max(8, tileSize.height / 2);
//         }
//     }

//     // 4. 改进的CLAHE处理 - 转换为CPU版本
//     cv::Mat claheResult;
//     cv::Mat darkMaskVis;
//     darkMask.convertTo(darkMaskVis, CV_8U, 255.0);

//     if (cv::countNonZero(darkMaskVis) > 0) {
//         // 创建和应用CLAHE
//         cv::Mat validPixels;
//         matImage.copyTo(validPixels, darkMaskVis);

//         // 创建CLAHE实例并应用
//         cv::Ptr<cv::CLAHE> clahePtr = cv::createCLAHE(adaptiveClipLimit, adaptiveTileSize);
//         clahePtr->apply(validPixels, claheResult);

//         // 5. 细节增强
//         cv::Mat detailMask;
//         cv::Laplacian(validPixels, detailMask, validPixels.type(), 1);

//         // 保持原始细节
//         cv::addWeighted(claheResult, 0.85, detailMask, 0.15, 0, claheResult);

//         // 6. 锐化处理
//         cv::Mat sharpenKernel = (cv::Mat_<float>(3,3) <<
//                                      -0.1, -0.1, -0.1,
//                                  -0.1,  2.0, -0.1,
//                                  -0.1, -0.1, -0.1);

//         cv::Mat sharpened;
//         cv::filter2D(claheResult, sharpened, claheResult.depth(), sharpenKernel);

//         // 自适应混合
//         cv::addWeighted(claheResult, 0.7, sharpened, 0.3, 0, claheResult);
//     } else {
//         claheResult = matImage.clone();
//     }

//     // 7. 改进的结果混合
//     cv::Mat resultImage = matImage.clone();

//     // 使用CPU进行最终的图像块处理
//     {
//         std::vector<std::future<void>> futures;
//         int rowsPerThread = height / threadConfig.numThreads;

//         for (unsigned int i = 0; i < threadConfig.numThreads; ++i) {
//             int startRow = i * rowsPerThread;
//             int endRow = (i == threadConfig.numThreads - 1) ? height : (i + 1) * rowsPerThread;

//             futures.push_back(std::async(std::launch::async, [&, i, startRow, endRow]() {
//                 logThreadStart(i, "Enhanced Progressive Blending", startRow, endRow);
//                 processEnhancedImageChunk(resultImage, matImage, claheResult, darkMask,
//                                           threshold, startRow, endRow, i);
//                 logThreadComplete(i, "Enhanced Progressive Blending");
//             }));
//         }

//         for (auto& future : futures) {
//             future.wait();
//         }
//     }

//     // 8. 最终优化 - 转换为CPU版本
//     cv::GaussianBlur(resultImage, resultImage, cv::Size(3, 3), 0.5);

//     auto endTime = std::chrono::high_resolution_clock::now();
//     metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
//     metrics.totalTime = metrics.processingTime;

//     finalImage = matToVector(resultImage);
// }

// 新增：增强的条带处理函数
void CLAHEProcessor::processEnhancedStrip(cv::Mat& strip, cv::Ptr<cv::CLAHE>& clahe) {
    cv::Mat stripResult;
    clahe->apply(strip, stripResult);

    // 局部对比度增强
    cv::Mat localContrast;
    cv::Laplacian(strip, localContrast, CV_16U, 1);

    // 自适应混合
    float contrastWeight = 0.15f;
    cv::addWeighted(stripResult, 1.0 - contrastWeight, localContrast, contrastWeight, 0, stripResult);

    // 边缘保持
    cv::Mat edges;
    cv::Sobel(strip, edges, CV_16U, 1, 1);
    float edgeWeight = 0.1f;
    cv::addWeighted(stripResult, 1.0 - edgeWeight, edges, edgeWeight, 0, strip);
}

// 新增：增强的结果优化函数
void CLAHEProcessor::optimizeResult(cv::Mat& result, const cv::Mat& original, float strength = 1.0f) {
    // 计算局部方差作为细节图
    cv::Mat variance;
    cv::Mat mean, meanSq;
    cv::boxFilter(result, mean, -1, cv::Size(3, 3));
    cv::boxFilter(result.mul(result), meanSq, -1, cv::Size(3, 3));
    variance = meanSq - mean.mul(mean);

    // 使用方差图来调整细节保持
    cv::Mat mask = variance > 1000;
    result.copyTo(result, mask);

    // 轻微的去噪
    cv::GaussianBlur(result, result, cv::Size(3, 3), 0.5 * strength);
}
