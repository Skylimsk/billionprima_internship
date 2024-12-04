#include "interlace.h"
#include <algorithm>

InterlaceProcessor::InterlacedResult InterlaceProcessor::processEnhancedInterlacedSections(
    const std::vector<std::vector<uint16_t>>& inputImage,
    StartPoint lowEnergyStart,
    StartPoint highEnergyStart,
    const MergeParams& userParams)  {

    int height = inputImage.size();
    int width = inputImage[0].size();
    int quarterWidth = width / 4;

    // Split image into four parts
    std::vector<std::vector<uint16_t>> leftLeft(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> leftRight(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightLeft(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightRight(height, std::vector<uint16_t>(quarterWidth));

    // Copy parts from input image
    for (int y = 0; y < height; ++y) {
        std::copy(inputImage[y].begin(),
                  inputImage[y].begin() + quarterWidth,
                  leftLeft[y].begin());
        std::copy(inputImage[y].begin() + quarterWidth,
                  inputImage[y].begin() + 2 * quarterWidth,
                  leftRight[y].begin());
        std::copy(inputImage[y].begin() + 2 * quarterWidth,
                  inputImage[y].begin() + 3 * quarterWidth,
                  rightLeft[y].begin());
        std::copy(inputImage[y].begin() + 3 * quarterWidth,
                  inputImage[y].end(),
                  rightRight[y].begin());
    }

    // Determine sections to use
    const std::vector<std::vector<uint16_t>>* lowEnergyFirst = nullptr;
    const std::vector<std::vector<uint16_t>>* lowEnergySecond = nullptr;
    const std::vector<std::vector<uint16_t>>* highEnergyFirst = nullptr;
    const std::vector<std::vector<uint16_t>>* highEnergySecond = nullptr;

    // Map start points to corresponding image parts
    if (lowEnergyStart == StartPoint::LEFT_LEFT) {
        lowEnergyFirst = &leftLeft;
        lowEnergySecond = &leftRight;
    } else {
        lowEnergyFirst = &leftRight;
        lowEnergySecond = &leftLeft;
    }

    if (highEnergyStart == StartPoint::RIGHT_LEFT) {
        highEnergyFirst = &rightLeft;
        highEnergySecond = &rightRight;
    } else {
        highEnergyFirst = &rightRight;
        highEnergySecond = &rightLeft;
    }

    InterlacedResult result;

    result.lowEnergyImage = performInterlacing(*lowEnergyFirst, *lowEnergySecond);
    result.highEnergyImage = performInterlacing(*highEnergyFirst, *highEnergySecond);

    // Automatically determine best weighting method
    MergeParams bestParams = determineBestWeightingMethod(
        result.lowEnergyImage,
        result.highEnergyImage
        );

    // Use automatically determined parameters
    result.combinedImage = mergeInterlacedSections(
        result.lowEnergyImage,
        result.highEnergyImage,
        bestParams
        );

    applyCalibration(result.combinedImage);
    return result;
}

std::vector<std::vector<uint16_t>> InterlaceProcessor::performInterlacing(
    const std::vector<std::vector<uint16_t>>& firstSection,
    const std::vector<std::vector<uint16_t>>& secondSection) {

    int height = firstSection.size();
    int width = firstSection[0].size();
    std::vector<std::vector<uint16_t>> interlacedResult(height * 2, std::vector<uint16_t>(width));

    // Perform interlacing by alternating rows
    for (int y = 0; y < height; ++y) {
        // Copy row from first section to even rows
        std::copy(firstSection[y].begin(), firstSection[y].end(), interlacedResult[y * 2].begin());

        // Copy row from second section to odd rows
        std::copy(secondSection[y].begin(), secondSection[y].end(), interlacedResult[y * 2 + 1].begin());
    }

    return interlacedResult;
}

float InterlaceProcessor::calculateLocalVariance(
    const std::vector<std::vector<uint16_t>>& section,
    int centerY, int centerX,
    int windowSize
    ) {
    int height = section.size();
    int width = section[0].size();
    int halfWindow = windowSize / 2;

    // Calculate mean
    float mean = 0.0f;
    int count = 0;

    for (int y = std::max(0, centerY - halfWindow);
         y <= std::min(height - 1, centerY + halfWindow); ++y) {
        for (int x = std::max(0, centerX - halfWindow);
             x <= std::min(width - 1, centerX + halfWindow); ++x) {
            mean += section[y][x];
            count++;
        }
    }
    mean /= count;

    // Calculate variance
    float variance = 0.0f;
    for (int y = std::max(0, centerY - halfWindow);
         y <= std::min(height - 1, centerY + halfWindow); ++y) {
        for (int x = std::max(0, centerX - halfWindow);
             x <= std::min(width - 1, centerX + halfWindow); ++x) {
            float diff = section[y][x] - mean;
            variance += diff * diff;
        }
    }
    variance /= count;

    return variance;
}

float InterlaceProcessor::calculateDynamicWeight(
    const std::vector<std::vector<uint16_t>>& lowSection,
    const std::vector<std::vector<uint16_t>>& highSection,
    int y, int x,
    const MergeParams& params) {

    const float epsilon = 1e-6f;  // 防止除零
    const float conservativeRange = 0.3f;  // 限制权重范围在 0.3-0.7
    float weight = params.baseWeight;

    switch (params.weightMethod) {
    case WeightingMethod::STATIC:
        // 对静态权重进行保守限制
        return std::clamp(params.baseWeight, 0.3f, 0.7f);

    case WeightingMethod::INTENSITY_BASED: {
        float lowIntensity = static_cast<float>(lowSection[y][x]);
        float highIntensity = static_cast<float>(highSection[y][x]);
        float totalIntensity = lowIntensity + highIntensity + epsilon;

        // 计算基础权重
        float rawWeight = lowIntensity / totalIntensity;

        // 应用保守范围映射
        float mappedWeight = 0.5f + (rawWeight - 0.5f) * (1.0f - 2 * conservativeRange);
        weight = std::clamp(mappedWeight, conservativeRange, 1.0f - conservativeRange);
        break;
    }

    case WeightingMethod::GRADIENT_BASED: {
        int height = lowSection.size();
        int width = lowSection[0].size();

        // 计算梯度，使用安全的边界检查
        float lowGradX = 0.0f, lowGradY = 0.0f;
        float highGradX = 0.0f, highGradY = 0.0f;

        if (x > 0 && x < width - 1) {
            lowGradX = std::abs(static_cast<float>(lowSection[y][x+1]) -
                                static_cast<float>(lowSection[y][x-1])) / 2.0f;
            highGradX = std::abs(static_cast<float>(highSection[y][x+1]) -
                                 static_cast<float>(highSection[y][x-1])) / 2.0f;
        }

        if (y > 0 && y < height - 1) {
            lowGradY = std::abs(static_cast<float>(lowSection[y+1][x]) -
                                static_cast<float>(lowSection[y-1][x])) / 2.0f;
            highGradY = std::abs(static_cast<float>(highSection[y+1][x]) -
                                 static_cast<float>(highSection[y-1][x])) / 2.0f;
        }

        float lowGradMag = std::sqrt(lowGradX * lowGradX + lowGradY * lowGradY);
        float highGradMag = std::sqrt(highGradX * highGradX + highGradY * highGradY);
        float totalGradMag = lowGradMag + highGradMag + epsilon;

        // 计算基础权重
        float rawWeight = lowGradMag / totalGradMag;

        // 应用保守范围映射
        float mappedWeight = 0.5f + (rawWeight - 0.5f) * (1.0f - 2 * conservativeRange);
        weight = std::clamp(mappedWeight, conservativeRange, 1.0f - conservativeRange);
        break;
    }

    case WeightingMethod::VARIANCE_BASED: {
        float lowVar = calculateLocalVariance(lowSection, y, x, params.windowSize);
        float highVar = calculateLocalVariance(highSection, y, x, params.windowSize);
        float totalVar = lowVar + highVar + epsilon;

        // 计算基础权重
        float rawWeight = lowVar / totalVar;

        // 应用保守范围映射
        float mappedWeight = 0.5f + (rawWeight - 0.5f) * (1.0f - 2 * conservativeRange);
        weight = std::clamp(mappedWeight, conservativeRange, 1.0f - conservativeRange);
        break;
    }
    }

    // 最终的权重保护
    return std::clamp(weight, 0.3f, 0.7f);
}

std::vector<std::vector<uint16_t>> InterlaceProcessor::mergeInterlacedSections(
    const std::vector<std::vector<uint16_t>>& lowEnergySection,
    const std::vector<std::vector<uint16_t>>& highEnergySection,
    const MergeParams& mergeParams) {

    int height = lowEnergySection.size();
    int width = lowEnergySection[0].size();
    std::vector<std::vector<uint16_t>> mergedResult(height, std::vector<uint16_t>(width));

    // 计算全局统计信息
    float avgLow = 0.0f, avgHigh = 0.0f;
    float varLow = 0.0f, varHigh = 0.0f;
    int count = 0;

    // 第一遍：计算平均值
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            avgLow += lowEnergySection[y][x];
            avgHigh += highEnergySection[y][x];
            count++;
        }
    }
    avgLow /= count;
    avgHigh /= count;

    // 第二遍：计算方差
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float diffLow = lowEnergySection[y][x] - avgLow;
            float diffHigh = highEnergySection[y][x] - avgHigh;
            varLow += diffLow * diffLow;
            varHigh += diffHigh * diffHigh;
        }
    }
    varLow = std::sqrt(varLow / count);
    varHigh = std::sqrt(varHigh / count);

    // 合并处理
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float lowValue = static_cast<float>(lowEnergySection[y][x]);
            float highValue = static_cast<float>(highEnergySection[y][x]);

            // 计算每个像素相对于其平均值的偏差程度
            float devLow = std::abs(lowValue - avgLow) / varLow;
            float devHigh = std::abs(highValue - avgHigh) / varHigh;

            // 根据偏差选择权重
            float lowWeight;
            if (devLow > devHigh) {
                // 如果低能量图像的偏差更大，说明这里可能有重要特征
                lowWeight = 0.8f;
            } else if (devHigh > devLow) {
                // 如果高能量图像的偏差更大
                lowWeight = 0.2f;
            } else {
                // 偏差相近时稍微偏向低能量图像
                lowWeight = 0.6f;
            }

            float mergedValue = lowValue * lowWeight + highValue * (1.0f - lowWeight);

            // 如果合并结果导致对比度显著降低，则选择对比度更强的图像
            float contrastLow = std::abs(lowValue - avgLow);
            float contrastHigh = std::abs(highValue - avgHigh);
            float contrastMerged = std::abs(mergedValue - (avgLow + avgHigh) * 0.5f);

            if (contrastMerged < std::max(contrastLow, contrastHigh) * 0.8f) {
                mergedValue = (contrastLow > contrastHigh) ? lowValue : highValue;
            }

            mergedResult[y][x] = static_cast<uint16_t>(std::clamp(mergedValue, 0.0f, 65535.0f));
        }
    }

    return mergedResult;
}

InterlaceProcessor::CalibrationParams InterlaceProcessor::calibrationParams;

// Add calibration parameter setter implementation
void InterlaceProcessor::setCalibrationParams(int linesToProcessY, int linesToProcessX) {
    calibrationParams.linesToProcessY = linesToProcessY;
    calibrationParams.linesToProcessX = linesToProcessX;
    calibrationParams.isInitialized = true;
}

// Add calibration implementation
void InterlaceProcessor::applyCalibration(std::vector<std::vector<uint16_t>>& image) {
    if (!calibrationParams.isInitialized) return;

    int height = image.size();
    int width = image[0].size();

    // Y-axis processing (only if Y lines are specified)
    if (calibrationParams.linesToProcessY > 0) {
        std::vector<float> referenceYMean(width, 0.0f);
        for (int y = 0; y < std::min(calibrationParams.linesToProcessY, height); ++y) {
            for (int x = 0; x < width; ++x) {
                referenceYMean[x] += image[y][x];
            }
        }
        for (int x = 0; x < width; ++x) {
            referenceYMean[x] /= calibrationParams.linesToProcessY;
        }

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float normalizedValue = static_cast<float>(image[y][x]) / referenceYMean[x];
                image[y][x] = static_cast<uint16_t>(std::min(normalizedValue * 65535.0f, 65535.0f));
            }
        }
    }

    // X-axis processing (only if X lines are specified)
    if (calibrationParams.linesToProcessX > 0) {
        std::vector<float> referenceXMean(height, 0.0f);
        for (int y = 0; y < height; ++y) {
            for (int x = width - calibrationParams.linesToProcessX; x < width; ++x) {
                referenceXMean[y] += image[y][x];
            }
            referenceXMean[y] /= calibrationParams.linesToProcessX;
        }

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < height; ++y) {
                float normalizedValue = static_cast<float>(image[y][x]) / referenceXMean[y];
                image[y][x] = static_cast<uint16_t>(std::min(normalizedValue * 65535.0f, 65535.0f));
            }
        }
    }
}
