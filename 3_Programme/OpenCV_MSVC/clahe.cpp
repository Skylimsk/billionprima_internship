#ifdef _MSC_VER
#pragma warning(disable: 4267) // conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable: 4244) // conversion from 'double' to 'float', possible loss of data
#pragma warning(disable: 4458) // declaration hides class member
#endif

#include "CLAHE.h"

CLAHEProcessor::CLAHEProcessor() {
    clahe = cv::createCLAHE();
}

cv::Mat CLAHEProcessor::applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        cv::Mat input8bit = convertTo8Bit(inputImage);
        cv::cuda::GpuMat gpuInput;
        gpuInput.upload(input8bit);

        cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit, tileSize);
        cv::cuda::GpuMat gpuResult;
        gpuClahe->apply(gpuInput, gpuResult);

        cv::Mat result;
        gpuResult.download(result);

        cv::Mat output16bit = convertTo16Bit(result);
        preserveValueRange(inputImage, output16bit);

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

        return output16bit;
    }
    catch (const cv::Exception& e) {
        qDebug() << "GPU CLAHE processing failed:" << e.what();
        throw;
    }
}

cv::Mat CLAHEProcessor::applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        cv::Mat input8bit = convertTo8Bit(inputImage);

        cv::Ptr<cv::CLAHE> cpuClahe = cv::createCLAHE(clipLimit, tileSize);
        cv::Mat result;
        cpuClahe->apply(input8bit, result);

        cv::Mat output16bit = convertTo16Bit(result);
        preserveValueRange(inputImage, output16bit);

        auto endTime = std::chrono::high_resolution_clock::now();
        metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
        metrics.totalTime = metrics.processingTime;

        return output16bit;
    }
    catch (const cv::Exception& e) {
        qDebug() << "CPU CLAHE processing failed:" << e.what();
        throw;
    }
}

// Helper functions
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

cv::Mat CLAHEProcessor::vectorToMat(const std::vector<std::vector<uint16_t>>& image) {
    int height = image.size();
    int width = image[0].size();
    cv::Mat mat(height, width, CV_16UC1);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            mat.at<uint16_t>(y, x) = image[y][x];
        }
    }

    return mat;
}

std::vector<std::vector<uint16_t>> CLAHEProcessor::matToVector(const cv::Mat& mat) {
    int height = mat.rows;
    int width = mat.cols;
    std::vector<std::vector<uint16_t>> image(height, std::vector<uint16_t>(width));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            image[y][x] = mat.at<uint16_t>(y, x);
        }
    }

    return image;
}

void CLAHEProcessor::applyThresholdCLAHE_GPU(std::vector<std::vector<uint16_t>>& finalImage, uint16_t threshold, double clipLimit, const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    auto startTime = std::chrono::high_resolution_clock::now();

    // Convert vector to Mat
    cv::Mat matImage = vectorToMat(finalImage);

    // Debug window for original image
    cv::Mat debugOriginal;
    matImage.convertTo(debugOriginal, CV_8U, 1.0/256.0);
    //cv::namedWindow("Original Image", cv::WINDOW_AUTOSIZE);
    //cv::imshow("Original Image", debugOriginal);

    int height = matImage.rows;
    int width = matImage.cols;

    // Step 1: Create mask with transition zone on CPU
    cv::Mat cpuDarkMask = cv::Mat::zeros(matImage.size(), CV_32F);
    float transitionRange = threshold * 0.35f;

    // Process mask on CPU (complex logic is harder to parallelize)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t pixelValue = matImage.at<uint16_t>(y, x);
            if (pixelValue < threshold) {
                if (pixelValue > threshold - transitionRange) {
                    float factor = (threshold - pixelValue) / transitionRange;
                    factor = factor * factor * factor * (factor * (6 * factor - 15) + 10);
                    cpuDarkMask.at<float>(y, x) = factor;
                } else {
                    cpuDarkMask.at<float>(y, x) = 0.8f + 0.2f * (pixelValue / (threshold - transitionRange));
                }
            }
        }
    }

    // Upload mask to GPU
    cv::cuda::GpuMat darkMask;
    darkMask.upload(cpuDarkMask);

    // Step 2: Multi-pass Gaussian blur using CUDA
    cv::cuda::GpuMat tempMask;

    // Create Gaussian filters
    cv::Ptr<cv::cuda::Filter> gaussianFilter1 = cv::cuda::createGaussianFilter(
        darkMask.type(), darkMask.type(), cv::Size(7, 7), 1.5);
    cv::Ptr<cv::cuda::Filter> gaussianFilter2 = cv::cuda::createGaussianFilter(
        darkMask.type(), darkMask.type(), cv::Size(5, 5), 1.0);

    // Apply filters
    gaussianFilter1->apply(darkMask, tempMask);
    gaussianFilter2->apply(tempMask, darkMask);

    // Visualize mask
    cv::Mat darkMaskVis;
    cv::Mat cpuDarkMask2;
    darkMask.download(cpuDarkMask2);
    cpuDarkMask2.convertTo(darkMaskVis, CV_8U, 255.0);
    //cv::namedWindow("Dark Mask", cv::WINDOW_AUTOSIZE);
    //cv::imshow("Dark Mask", darkMaskVis);

    // Step 3: Apply gentler CLAHE on GPU
    cv::Ptr<cv::cuda::CLAHE> gpuClahe = cv::cuda::createCLAHE(clipLimit * 0.6, tileSize);
    cv::cuda::GpuMat gpuClaheResult;
    cv::cuda::GpuMat darkPixels;
    darkPixels.upload(matImage);

    if (cv::countNonZero(darkMaskVis) > 0) {
        // Create valid pixels mask on GPU
        cv::cuda::GpuMat validPixels;
        darkPixels.copyTo(validPixels, darkMask);

        // Apply CLAHE
        gpuClahe->apply(validPixels, gpuClaheResult);

        // Download for CPU operation (sharpening)
        cv::Mat cpuResult;
        gpuClaheResult.download(cpuResult);

        // Sharpen on CPU
        cv::Mat sharpenKernel = (cv::Mat_<float>(3,3) <<
                                     0, -0.3, 0,
                                 -0.3, 2.2, -0.3,
                                 0, -0.3, 0);

        cv::Mat sharpened;
        cv::filter2D(cpuResult, sharpened, CV_16U, sharpenKernel);

        // Upload sharpened result back to GPU
        cv::cuda::GpuMat gpuSharpened;
        gpuSharpened.upload(sharpened);

        // Blend operation
        cv::cuda::addWeighted(gpuClaheResult, 0.5, gpuSharpened, 0.5, 0, gpuClaheResult);

        // Debug visualization
        cv::Mat debugCLAHE;
        cv::Mat cpuClaheResult;
        gpuClaheResult.download(cpuClaheResult);
        cpuClaheResult.convertTo(debugCLAHE, CV_8U, 1.0/256.0);
        //cv::namedWindow("CLAHE Result", cv::WINDOW_AUTOSIZE);
        //cv::imshow("CLAHE Result", debugCLAHE);
    } else {
        darkPixels.copyTo(gpuClaheResult);
    }

    // Step 4: Progressive blending (on CPU due to complex pixel-wise operations)
    cv::Mat resultImage = matImage.clone();
    cv::Mat cpuClaheResult;
    gpuClaheResult.download(cpuClaheResult);
    darkMask.download(cpuDarkMask);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float blendFactor = cpuDarkMask.at<float>(y, x);
            if (blendFactor > 0) {
                uint16_t originalValue = matImage.at<uint16_t>(y, x);
                uint16_t claheValue = cpuClaheResult.at<uint16_t>(y, x);

                float adaptiveBlend = blendFactor;
                if (originalValue < threshold * 0.6) {
                    float darknessFactor = originalValue / (threshold * 0.6f);
                    adaptiveBlend *= 0.7f + 0.3f * darknessFactor;
                }

                resultImage.at<uint16_t>(y, x) = static_cast<uint16_t>(
                    originalValue * (1.0f - adaptiveBlend) + claheValue * adaptiveBlend
                    );
            }
        }
    }

    // Final processing steps on GPU
    cv::cuda::GpuMat gpuResult;
    gpuResult.upload(resultImage);

    // Normalize using OpenCV CPU operations then upload result
    cv::Mat normalizedResult;
    cv::normalize(resultImage, normalizedResult, 0, 65535, cv::NORM_MINMAX, CV_16U);
    cv::cuda::GpuMat gpuNormalized;
    gpuNormalized.upload(normalizedResult);

    // Final blending
    cv::cuda::addWeighted(gpuResult, 0.8, gpuNormalized, 0.2, 0, gpuResult);

    // Final Gaussian blur
    cv::Ptr<cv::cuda::Filter> finalBlur = cv::cuda::createGaussianFilter(
        gpuResult.type(), gpuResult.type(), cv::Size(3, 3), 0.5);
    finalBlur->apply(gpuResult, gpuResult);

    // Create debug window for final result
    cv::Mat debugFinal;
    gpuResult.download(resultImage);
    resultImage.convertTo(debugFinal, CV_8U, 1.0/256.0);
    //cv::namedWindow("Final Result", cv::WINDOW_AUTOSIZE);
    //cv::imshow("Final Result", debugFinal);

    cv::waitKey(1);

    // Update timing metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    metrics.totalTime = metrics.processingTime;

    // Update the final image
    finalImage = matToVector(resultImage);
}

void CLAHEProcessor::applyThresholdCLAHE_CPU(std::vector<std::vector<uint16_t>>& finalImage, uint16_t threshold, double clipLimit, const cv::Size& tileSize) {
    metrics.reset();
    metrics.isCLAHE = true;
    auto startTime = std::chrono::high_resolution_clock::now();

    cv::Mat matImage = vectorToMat(finalImage);

    // Debug window for original image
    cv::Mat debugOriginal;
    matImage.convertTo(debugOriginal, CV_8U, 1.0/256.0);
    cv::namedWindow("Original Image", cv::WINDOW_AUTOSIZE);
    cv::imshow("Original Image", debugOriginal);

    int height = matImage.rows;
    int width = matImage.cols;

    // Step 1: Create mask with even wider transition zone
    cv::Mat darkMask = cv::Mat::zeros(matImage.size(), CV_32F);
    cv::Mat darkPixels = matImage.clone();

    float transitionRange = threshold * 0.35f;

    // Extract dark pixels with extended smooth transition
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t pixelValue = matImage.at<uint16_t>(y, x);
            if (pixelValue < threshold) {
                if (pixelValue > threshold - transitionRange) {
                    float factor = (threshold - pixelValue) / transitionRange;
                    factor = factor * factor * factor * (factor * (6 * factor - 15) + 10);
                    darkMask.at<float>(y, x) = factor;
                } else {
                    darkMask.at<float>(y, x) = 0.8f + 0.2f * (pixelValue / (threshold - transitionRange));
                }
            }
        }
    }

    // Step 2: Multi-pass Gaussian smoothing
    cv::GaussianBlur(darkMask, darkMask, cv::Size(7, 7), 1.5);
    cv::GaussianBlur(darkMask, darkMask, cv::Size(5, 5), 1.0);

    // Visualize mask
    cv::Mat darkMaskVis;
    darkMask.convertTo(darkMaskVis, CV_8U, 255.0);
    //cv::namedWindow("Dark Mask", cv::WINDOW_AUTOSIZE);
    //cv::imshow("Dark Mask", darkMaskVis);

    // Step 3: Apply gentler CLAHE
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(clipLimit * 0.6, tileSize);
    cv::Mat claheResult;

    if (cv::countNonZero(darkMaskVis) > 0) {
        cv::Mat validPixels;
        darkPixels.copyTo(validPixels, darkMaskVis);

        clahe->apply(validPixels, claheResult);

        cv::Mat sharpenKernel = (cv::Mat_<float>(3,3) <<
                                     0, -0.3, 0,
                                 -0.3, 2.2, -0.3,
                                 0, -0.3, 0);

        cv::Mat sharpened;
        cv::filter2D(claheResult, sharpened, CV_16U, sharpenKernel);

        float sharpenBlend = 0.5f;
        cv::addWeighted(claheResult, 1.0 - sharpenBlend, sharpened, sharpenBlend, 0, claheResult);

        cv::Mat debugCLAHE;
        claheResult.convertTo(debugCLAHE, CV_8U, 1.0/256.0);
        //cv::namedWindow("CLAHE Result", cv::WINDOW_AUTOSIZE);
        //cv::imshow("CLAHE Result", debugCLAHE);
    } else {
        claheResult = darkPixels.clone();
    }

    // Step 4: Progressive blending
    cv::Mat resultImage = matImage.clone();
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float blendFactor = darkMask.at<float>(y, x);
            if (blendFactor > 0) {
                uint16_t originalValue = matImage.at<uint16_t>(y, x);
                uint16_t claheValue = claheResult.at<uint16_t>(y, x);

                float adaptiveBlend = blendFactor;
                if (originalValue < threshold * 0.6) {
                    float darknessFactor = originalValue / (threshold * 0.6f);
                    adaptiveBlend *= 0.7f + 0.3f * darknessFactor;
                }

                resultImage.at<uint16_t>(y, x) = static_cast<uint16_t>(
                    originalValue * (1.0f - adaptiveBlend) + claheValue * adaptiveBlend
                    );
            }
        }
    }

    // Final processing steps
    cv::Mat normalizedResult;
    cv::normalize(resultImage, normalizedResult, 0, 65535, cv::NORM_MINMAX, CV_16U);

    float normalizationStrength = 0.2f;
    cv::addWeighted(resultImage, 1.0 - normalizationStrength,
                    normalizedResult, normalizationStrength, 0, resultImage);

    cv::GaussianBlur(resultImage, resultImage, cv::Size(3, 3), 0.5);

    // Create debug window for final result
    cv::Mat debugFinal;
    resultImage.convertTo(debugFinal, CV_8U, 1.0/256.0);
    //cv::namedWindow("Final Result", cv::WINDOW_AUTOSIZE);
    //cv::imshow("Final Result", debugFinal);

    cv::waitKey(1);

    // Update timing metrics
    auto endTime = std::chrono::high_resolution_clock::now();
    metrics.processingTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    metrics.totalTime = metrics.processingTime;

    // Update the final image
    finalImage = matToVector(resultImage);
}

