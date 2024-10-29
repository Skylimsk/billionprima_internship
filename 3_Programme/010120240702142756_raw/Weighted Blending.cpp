#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <numeric>  // For std::iota
#include <cmath>    // For std::fmin and std::fmax

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Function to save PNG
void savePNG(const std::string& pngFilePath, const std::vector<std::vector<uint16_t>>& image, int width, int height) {
    std::vector<uint8_t> imageData(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t pixel = static_cast<uint8_t>(image[y][x] >> 8);  // Convert 16-bit to 8-bit
            imageData[3 * (y * width + x) + 0] = pixel; // R
            imageData[3 * (y * width + x) + 1] = pixel; // G
            imageData[3 * (y * width + x) + 2] = pixel; // B
        }
    }
    stbi_write_png(pngFilePath.c_str(), width, height, 3, imageData.data(), width * 3);
}

// Function to apply Y-axis enhancement
void processYAxis(std::vector<std::vector<uint16_t>>& image, int linesToProcess, float darkPixelThreshold, uint16_t darkThreshold, float yScalingFactor) {
    int height = image.size();
    int width = image[0].size();

    std::vector<int> avgLine(width, 0);

    for (int y = 0; y < std::min(linesToProcess, height); ++y) {
        for (int x = 0; x < width; ++x) {
            avgLine[x] += image[y][x];
        }
    }

    for (int x = 0; x < width; ++x) {
        avgLine[x] /= linesToProcess;
    }

    for (int y = 0; y < height; ++y) {
        int darkPixelCount = std::count_if(image[y].begin(), image[y].end(), [&](uint16_t pixel) { return pixel < darkThreshold; });

        if (static_cast<float>(darkPixelCount) / width > darkPixelThreshold) {
            for (int x = 0; x < width; ++x) {
                image[y][x] = static_cast<uint16_t>(std::fmin(avgLine[x] * yScalingFactor, 65535.0));
            }
        } else {
            for (int x = 0; x < width; ++x) {
                image[y][x] = static_cast<uint16_t>(std::fmin(image[y][x] * yScalingFactor, 65535.0));
            }
        }
    }
}

// Function to apply X-axis flattening
void processXAxis(std::vector<std::vector<uint16_t>>& image, float xScalingFactor) {
    int height = image.size();
    int width = image[0].size();

    for (int x = 0; x < width; ++x) {
        float columnSum = 0;
        for (int y = 0; y < height; ++y) {
            columnSum += image[y][x];
        }
        float columnAverage = columnSum / height;

        for (int y = 0; y < height; ++y) {
            image[y][x] = static_cast<uint16_t>(std::fmin(image[y][x] * (xScalingFactor / columnAverage), 65535.0f));
        }
    }
}

// Function to blend two images pixel-wise using weighted blending
std::vector<std::vector<uint16_t>> blendImages(const std::vector<std::vector<uint16_t>>& image1, const std::vector<std::vector<uint16_t>>& image2, float weight1, float weight2) {
    int height = image1.size();
    int width = image1[0].size();
    std::vector<std::vector<uint16_t>> result(height, std::vector<uint16_t>(width));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            result[y][x] = static_cast<uint16_t>(std::fmin(weight1 * image1[y][x] + weight2 * image2[y][x], 65535.0));
        }
    }
    return result;
}

// Function to process and merge image parts into Low (Left Left + Left Right) and High (Right Left + Right Right)
void processAndMergeImageParts(const std::string& txtFilePath, int linesToProcess, float darkPixelThreshold, uint16_t darkThreshold, float yScalingFactor, float xScalingFactor) {
    std::ifstream inFile(txtFilePath);
    std::vector<std::vector<uint16_t>> data;
    std::string line;

    // Read data from the text file
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<uint16_t> row;
        uint16_t value;
        while (ss >> value)
            row.push_back(value);
        data.push_back(row);
    }

    int dataHeight = data.size();
    int dataWidth = data[0].size();
    int halfWidth = dataWidth / 2;
    int quarterWidth = halfWidth / 2;

    // Split the image into four parts: Left Left, Left Right, Right Left, Right Right
    std::vector<std::vector<uint16_t>> leftLeft(dataHeight, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> leftRight(dataHeight, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightLeft(dataHeight, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightRight(dataHeight, std::vector<uint16_t>(quarterWidth));

    for (int y = 0; y < dataHeight; ++y) {
        for (int x = 0; x < quarterWidth; ++x) {
            leftLeft[y][x] = data[y][x];
            leftRight[y][x] = data[y][x + quarterWidth];
            rightLeft[y][x] = data[y][x + halfWidth];
            rightRight[y][x] = data[y][x + halfWidth + quarterWidth];
        }
    }

    // Apply Y-axis and X-axis processing to each part with their respective scaling factors
    processYAxis(leftLeft, linesToProcess, darkPixelThreshold, darkThreshold, yScalingFactor);
    processXAxis(leftLeft, xScalingFactor);

    processYAxis(leftRight, linesToProcess, darkPixelThreshold, darkThreshold, yScalingFactor);
    processXAxis(leftRight, xScalingFactor);

    processYAxis(rightLeft, linesToProcess, darkPixelThreshold, darkThreshold, yScalingFactor);
    processXAxis(rightLeft, xScalingFactor);

    processYAxis(rightRight, linesToProcess, darkPixelThreshold, darkThreshold, yScalingFactor);
    processXAxis(rightRight, xScalingFactor);

    // Weighted blending: blend Left Left and Left Right into Low
    float weightLeft = 0.6f;  // Weight for the left images
    float weightRight = 0.4f; // Weight for the right images
    auto lowImage = blendImages(leftLeft, leftRight, weightLeft, weightRight);

    // Weighted blending: blend Right Left and Right Right into High
    auto highImage = blendImages(rightLeft, rightRight, weightLeft, weightRight);

    // Now blend Low and High using weighted blending
    auto finalMergedImage = blendImages(lowImage, highImage, weightLeft, weightRight);

    // Save the final merged image
    savePNG("output_weighted_blend.png", finalMergedImage, quarterWidth, dataHeight);
}

int main() {
    // Parameters for image processing
    int linesToProcess = 20;  // Number of lines to process for Y-axis
    float darkPixelThreshold = 0.25;  // Percentage threshold for dark pixels in a row
    uint16_t darkThreshold = 50;   // Threshold to determine whether a pixel is dark

    // Independent scaling factors for X and Y axis
    float yScalingFactor = 1.0;  // Scaling factor for Y-axis enhancement
    float xScalingFactor = 78000.0;  // Scaling factor for X-axis flattening

    // Process and merge image parts into Low and High using weighted blending, then merge them together
    processAndMergeImageParts("010120240702142756_raw.txt", linesToProcess, darkPixelThreshold, darkThreshold, yScalingFactor, xScalingFactor);

    return 0;
}
