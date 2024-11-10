#include "interlace.h"
#include <algorithm>

InterlaceProcessor::InterlacedResult InterlaceProcessor::processEnhancedInterlacedSections(
    const std::vector<std::vector<uint16_t>>& inputImage,
    StartPoint lowEnergyStart,
    StartPoint highEnergyStart,
    MergeMethod mergeMethod) {

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

    // Create result structure
    InterlacedResult result;

    // Perform interlacing for low energy and high energy sections
    result.lowEnergyImage = performInterlacing(*lowEnergyFirst, *lowEnergySecond);
    result.highEnergyImage = performInterlacing(*highEnergyFirst, *highEnergySecond);

    // Merge the interlaced sections
    result.combinedImage = mergeInterlacedSections(
        result.lowEnergyImage,
        result.highEnergyImage,
        mergeMethod
        );

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

std::vector<std::vector<uint16_t>> InterlaceProcessor::mergeInterlacedSections(
    const std::vector<std::vector<uint16_t>>& lowEnergySection,
    const std::vector<std::vector<uint16_t>>& highEnergySection,
    MergeMethod method) {

    int height = lowEnergySection.size();
    int width = lowEnergySection[0].size();
    std::vector<std::vector<uint16_t>> mergedResult(height, std::vector<uint16_t>(width));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (method == MergeMethod::MINIMUM_VALUE) {
                // Use minimum value
                mergedResult[y][x] = std::min(
                    lowEnergySection[y][x],
                    highEnergySection[y][x]
                    );
            } else {
                // Use weighted average (default 0.5 weight for each)
                mergedResult[y][x] = static_cast<uint16_t>(
                    (static_cast<float>(lowEnergySection[y][x]) +
                     static_cast<float>(highEnergySection[y][x])) / 2.0f
                    );
            }
        }
    }

    return mergedResult;
}
