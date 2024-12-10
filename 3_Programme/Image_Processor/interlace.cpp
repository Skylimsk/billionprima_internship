#include "interlace.h"
#include <algorithm>
#include <cstring>

InterlaceProcessor::InterlacedResult::InterlacedResult(int h, int w) : height(h), width(w) {
    lowEnergyImage = allocateImage(height, width);
    highEnergyImage = allocateImage(height, width);
    combinedImage = allocateImage(height, width);
}

InterlaceProcessor::InterlacedResult::~InterlacedResult() {
    if (lowEnergyImage) deallocateImage(lowEnergyImage, height);
    if (highEnergyImage) deallocateImage(highEnergyImage, height);
    if (combinedImage) deallocateImage(combinedImage, height);
}

double** InterlaceProcessor::allocateImage(int height, int width) {
    double** image = new double*[height];
    for (int i = 0; i < height; i++) {
        image[i] = new double[width]();  // Initialize to zero
    }
    return image;
}

void InterlaceProcessor::deallocateImage(double** image, int height) {
    if (!image) return;

    for (int i = 0; i < height; i++) {
        delete[] image[i];
    }
    delete[] image;
}

InterlaceProcessor::MergeParams InterlaceProcessor::determineBestWeightingMethod(
    double** lowEnergySection,
    double** highEnergySection,
    int height, int width) {

    // Calculate global statistics
    double lowMean = 0.0, highMean = 0.0;
    double lowVar = 0.0, highVar = 0.0;

    // First pass: calculate means
    int totalPixels = height * width;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            lowMean += lowEnergySection[y][x];
            highMean += highEnergySection[y][x];
        }
    }
    lowMean /= totalPixels;
    highMean /= totalPixels;

    // Second pass: calculate variances
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double lowDiff = lowEnergySection[y][x] - lowMean;
            double highDiff = highEnergySection[y][x] - highMean;
            lowVar += lowDiff * lowDiff;
            highVar += highDiff * highDiff;
        }
    }
    lowVar /= totalPixels;
    highVar /= totalPixels;

    // Calculate edge content
    double lowEdgeContent = 0.0, highEdgeContent = 0.0;
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            // Simplified Sobel operator
            double lowGx = lowEnergySection[y][x+1] - lowEnergySection[y][x-1];
            double lowGy = lowEnergySection[y+1][x] - lowEnergySection[y-1][x];
            double highGx = highEnergySection[y][x+1] - highEnergySection[y][x-1];
            double highGy = highEnergySection[y+1][x] - highEnergySection[y-1][x];

            lowEdgeContent += std::sqrt(lowGx*lowGx + lowGy*lowGy);
            highEdgeContent += std::sqrt(highGx*highGx + highGy*highGy);
        }
    }

    MergeParams params;
    params.method = MergeMethod::WEIGHTED_AVERAGE;

    // Decision logic based on image characteristics
    if (std::abs(lowVar - highVar) > 0.5 * std::max(lowVar, highVar)) {
        // Significant variance difference - use intensity-based weighting
        params.weightMethod = WeightingMethod::INTENSITY_BASED;
        params.baseWeight = 0.5f;
    } else if (std::abs(lowEdgeContent - highEdgeContent) > 0.3 * std::max(lowEdgeContent, highEdgeContent)) {
        // Significant edge content difference - use gradient-based weighting
        params.weightMethod = WeightingMethod::GRADIENT_BASED;
        params.baseWeight = 0.5f;
    } else {
        // Similar characteristics - use variance-based weighting
        params.weightMethod = WeightingMethod::VARIANCE_BASED;
        params.baseWeight = 0.5f;
        params.windowSize = 5;  // Use larger window for more stable variance estimation
    }

    return params;
}

InterlaceProcessor::InterlacedResult* InterlaceProcessor::processEnhancedInterlacedSections(
    double** inputImage,
    int height, int width,
    StartPoint lowEnergyStart,
    StartPoint highEnergyStart,
    const MergeParams& userParams) {

    int quarterWidth = width / 4;

    // Allocate four parts
    double** leftLeft = allocateImage(height, quarterWidth);
    double** leftRight = allocateImage(height, quarterWidth);
    double** rightLeft = allocateImage(height, quarterWidth);
    double** rightRight = allocateImage(height, quarterWidth);

    // Copy parts from input image
    for (int y = 0; y < height; ++y) {
        memcpy(leftLeft[y], &inputImage[y][0], quarterWidth * sizeof(double));
        memcpy(leftRight[y], &inputImage[y][quarterWidth], quarterWidth * sizeof(double));
        memcpy(rightLeft[y], &inputImage[y][2 * quarterWidth], quarterWidth * sizeof(double));
        memcpy(rightRight[y], &inputImage[y][3 * quarterWidth], quarterWidth * sizeof(double));
    }

    // Determine sections to use
    double** lowEnergyFirst = nullptr;
    double** lowEnergySecond = nullptr;
    double** highEnergyFirst = nullptr;
    double** highEnergySecond = nullptr;

    // Map start points to corresponding image parts
    if (lowEnergyStart == StartPoint::LEFT_LEFT) {
        lowEnergyFirst = leftLeft;
        lowEnergySecond = leftRight;
    } else {
        lowEnergyFirst = leftRight;
        lowEnergySecond = leftLeft;
    }

    if (highEnergyStart == StartPoint::RIGHT_LEFT) {
        highEnergyFirst = rightLeft;
        highEnergySecond = rightRight;
    } else {
        highEnergyFirst = rightRight;
        highEnergySecond = rightLeft;
    }

    // Create result object with doubled height
    InterlacedResult* result = new InterlacedResult(height * 2, quarterWidth);

    // Perform interlacing
    performInterlacing(lowEnergyFirst, lowEnergySecond, height, quarterWidth, result->lowEnergyImage);
    performInterlacing(highEnergyFirst, highEnergySecond, height, quarterWidth, result->highEnergyImage);

    // Automatically determine best weighting method
    MergeParams bestParams = determineBestWeightingMethod(
        result->lowEnergyImage,
        result->highEnergyImage,
        height * 2, quarterWidth);

    // Use automatically determined parameters
    mergeInterlacedSections(
        result->lowEnergyImage,
        result->highEnergyImage,
        height * 2, quarterWidth,
        bestParams,
        result->combinedImage);

    applyCalibration(result->combinedImage, height * 2, quarterWidth);

    // Clean up temporary arrays
    deallocateImage(leftLeft, height);
    deallocateImage(leftRight, height);
    deallocateImage(rightLeft, height);
    deallocateImage(rightRight, height);

    return result;
}
void InterlaceProcessor::performInterlacing(
    double** firstSection,
    double** secondSection,
    int height, int width,
    double** result) {

    // Perform interlacing by alternating rows
    for (int y = 0; y < height; ++y) {
        // Copy row from first section to even rows
        memcpy(result[y * 2], firstSection[y], width * sizeof(double));
        // Copy row from second section to odd rows
        memcpy(result[y * 2 + 1], secondSection[y], width * sizeof(double));
    }
}

float InterlaceProcessor::calculateLocalVariance(
    double** section,
    int height, int width,
    int centerY, int centerX,
    int windowSize) {

    int halfWindow = windowSize / 2;

    // Calculate mean
    double mean = 0.0;
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
    double variance = 0.0;
    for (int y = std::max(0, centerY - halfWindow);
         y <= std::min(height - 1, centerY + halfWindow); ++y) {
        for (int x = std::max(0, centerX - halfWindow);
             x <= std::min(width - 1, centerX + halfWindow); ++x) {
            double diff = section[y][x] - mean;
            variance += diff * diff;
        }
    }
    variance /= count;

    return static_cast<float>(variance);
}
float InterlaceProcessor::calculateDynamicWeight(
    double** lowSection,
    double** highSection,
    int height, int width,
    int y, int x,
    const MergeParams& params) {

    const float epsilon = 1e-6f;  // Prevent division by zero
    const float conservativeRange = 0.3f;  // Restrict weight range to 0.3-0.7
    float weight = params.baseWeight;

    switch (params.weightMethod) {
    case WeightingMethod::STATIC:
        // Apply conservative limits to static weight
        return std::clamp(params.baseWeight, 0.3f, 0.7f);

    case WeightingMethod::INTENSITY_BASED: {
        double lowIntensity = lowSection[y][x];
        double highIntensity = highSection[y][x];
        double totalIntensity = lowIntensity + highIntensity + epsilon;

        // Calculate base weight
        float rawWeight = static_cast<float>(lowIntensity / totalIntensity);

        // Apply conservative range mapping
        float mappedWeight = 0.5f + (rawWeight - 0.5f) * (1.0f - 2 * conservativeRange);
        weight = std::clamp(mappedWeight, conservativeRange, 1.0f - conservativeRange);
        break;
    }

    case WeightingMethod::GRADIENT_BASED: {
        // Calculate gradients with safe boundary checks
        double lowGradX = 0.0, lowGradY = 0.0;
        double highGradX = 0.0, highGradY = 0.0;

        if (x > 0 && x < width - 1) {
            lowGradX = std::abs(lowSection[y][x+1] - lowSection[y][x-1]) / 2.0;
            highGradX = std::abs(highSection[y][x+1] - highSection[y][x-1]) / 2.0;
        }

        if (y > 0 && y < height - 1) {
            lowGradY = std::abs(lowSection[y+1][x] - lowSection[y-1][x]) / 2.0;
            highGradY = std::abs(highSection[y+1][x] - highSection[y-1][x]) / 2.0;
        }

        double lowGradMag = std::sqrt(lowGradX * lowGradX + lowGradY * lowGradY);
        double highGradMag = std::sqrt(highGradX * highGradX + highGradY * highGradY);
        double totalGradMag = lowGradMag + highGradMag + epsilon;

        // Calculate base weight
        float rawWeight = static_cast<float>(lowGradMag / totalGradMag);

        // Apply conservative range mapping
        float mappedWeight = 0.5f + (rawWeight - 0.5f) * (1.0f - 2 * conservativeRange);
        weight = std::clamp(mappedWeight, conservativeRange, 1.0f - conservativeRange);
        break;
    }

    case WeightingMethod::VARIANCE_BASED: {
        float lowVar = calculateLocalVariance(lowSection, height, width, y, x, params.windowSize);
        float highVar = calculateLocalVariance(highSection, height, width, y, x, params.windowSize);
        float totalVar = lowVar + highVar + epsilon;

        // Calculate base weight
        float rawWeight = lowVar / totalVar;

        // Apply conservative range mapping
        float mappedWeight = 0.5f + (rawWeight - 0.5f) * (1.0f - 2 * conservativeRange);
        weight = std::clamp(mappedWeight, conservativeRange, 1.0f - conservativeRange);
        break;
    }
    }

    // Final weight protection
    return std::clamp(weight, 0.3f, 0.7f);
}

InterlaceProcessor::MergeParams::MergeParams(
    MergeMethod m,
    WeightingMethod w,
    float weight,
    int window)
    : method(m)
    , weightMethod(w)
    , baseWeight(weight)
    , windowSize(window) {
}

void InterlaceProcessor::mergeInterlacedSections(
    double** lowEnergySection,
    double** highEnergySection,
    int height, int width,
    const MergeParams& mergeParams,
    double** result) {

    // Calculate global statistics
    double avgLow = 0.0, avgHigh = 0.0;
    double varLow = 0.0, varHigh = 0.0;
    int count = 0;

    // First pass: calculate means
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            avgLow += lowEnergySection[y][x];
            avgHigh += highEnergySection[y][x];
            count++;
        }
    }
    avgLow /= count;
    avgHigh /= count;

    // Second pass: calculate variances
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double diffLow = lowEnergySection[y][x] - avgLow;
            double diffHigh = highEnergySection[y][x] - avgHigh;
            varLow += diffLow * diffLow;
            varHigh += diffHigh * diffHigh;
        }
    }
    varLow = std::sqrt(varLow / count);
    varHigh = std::sqrt(varHigh / count);

    // Merge processing
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double lowValue = lowEnergySection[y][x];
            double highValue = highEnergySection[y][x];

            // Calculate deviation from mean for each pixel
            double devLow = std::abs(lowValue - avgLow) / varLow;
            double devHigh = std::abs(highValue - avgHigh) / varHigh;

            // Choose weight based on deviation
            double lowWeight;
            if (devLow > devHigh) {
                // Low energy image has more significant features
                lowWeight = 0.8;
            } else if (devHigh > devLow) {
                // High energy image has more significant features
                lowWeight = 0.2;
            } else {
                // Similar deviations, slightly favor low energy image
                lowWeight = 0.6;
            }

            double mergedValue = lowValue * lowWeight + highValue * (1.0 - lowWeight);

            // Check if merge reduces contrast significantly
            double contrastLow = std::abs(lowValue - avgLow);
            double contrastHigh = std::abs(highValue - avgHigh);
            double contrastMerged = std::abs(mergedValue - (avgLow + avgHigh) * 0.5);

            if (contrastMerged < std::max(contrastLow, contrastHigh) * 0.8) {
                mergedValue = (contrastLow > contrastHigh) ? lowValue : highValue;
            }

            result[y][x] = mergedValue;
        }
    }
}

InterlaceProcessor::CalibrationParams InterlaceProcessor::calibrationParams;

void InterlaceProcessor::setCalibrationParams(int linesToProcessY, int linesToProcessX) {
    calibrationParams.linesToProcessY = linesToProcessY;
    calibrationParams.linesToProcessX = linesToProcessX;
    calibrationParams.isInitialized = true;
}

void InterlaceProcessor::applyCalibration(double** image, int height, int width) {
    if (!calibrationParams.isInitialized) return;

    // Y-axis processing (only if Y lines are specified)
    if (calibrationParams.linesToProcessY > 0) {
        double* referenceYMean = new double[width]();

        // Calculate mean for each column using reference lines
        for (int y = 0; y < std::min(calibrationParams.linesToProcessY, height); ++y) {
            for (int x = 0; x < width; ++x) {
                referenceYMean[x] += image[y][x];
            }
        }
        for (int x = 0; x < width; ++x) {
            referenceYMean[x] /= std::min(calibrationParams.linesToProcessY, height);
        }

        // Apply normalization
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                image[y][x] = image[y][x] / referenceYMean[x];
            }
        }

        delete[] referenceYMean;
    }

    // X-axis processing (only if X lines are specified)
    if (calibrationParams.linesToProcessX > 0) {
        double* referenceXMean = new double[height]();

        // Calculate mean for each row using reference lines
        for (int y = 0; y < height; ++y) {
            for (int x = width - calibrationParams.linesToProcessX; x < width; ++x) {
                referenceXMean[y] += image[y][x];
            }
            referenceXMean[y] /= calibrationParams.linesToProcessX;
        }

        // Apply normalization
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                image[y][x] = image[y][x] / referenceXMean[y];
            }
        }

        delete[] referenceXMean;
    }
}
