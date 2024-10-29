#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cmath>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Function to save a PNG image
void savePNG(const std::string& path, const std::vector<std::vector<uint16_t>>& img, int width, int height) {
    std::vector<uint8_t> imgData(width * height * 3);

    // Convert 16-bit image to 8-bit RGB and store in imgData
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            uint8_t pixel = static_cast<uint8_t>(img[y][x] >> 8);  // Convert 16-bit to 8-bit
            std::fill(&imgData[3 * (y * width + x)], &imgData[3 * (y * width + x) + 3], pixel);  // Set RGB channels
        }
    
    stbi_write_png(path.c_str(), width, height, 3, imgData.data(), width * 3);
}

// Process Y-axis of the image
void processYAxis(std::vector<std::vector<uint16_t>>& img, int linesToAvg, float darkPct, uint16_t darkThreshold) {
    const float Y_SCALING_FACTOR = 1.25f;
    const uint16_t MAX_PIXEL_VALUE = 65535;
    int height = img.size();
    int width = img[0].size();

    // Calculate average pixel value for the first few lines
    std::vector<int> avgLine(width, 0);
    for (int y = 0; y < std::min(linesToAvg, height); ++y)
        for (int x = 0; x < width; ++x)
            avgLine[x] += img[y][x];
    
    for (int& val : avgLine)
        val /= linesToAvg;

    // Process each pixel based on dark pixel percentage
    for (int y = 0; y < height; ++y) {
        int darkPixels = std::count_if(img[y].begin(), img[y].end(), [&](uint16_t px) { return px < darkThreshold; });
        bool hasManyDarkPixels = static_cast<float>(darkPixels) / width > darkPct;

        for (int x = 0; x < width; ++x) {
            uint16_t newVal = hasManyDarkPixels ? avgLine[x] : img[y][x];
            img[y][x] = static_cast<uint16_t>(std::fmin(newVal * Y_SCALING_FACTOR, MAX_PIXEL_VALUE));
        }
    }
}

// Process X-axis of the image
void processXAxis(std::vector<std::vector<uint16_t>>& img) {
    const float X_SCALING_FACTOR = 57900.0f;
    const uint16_t MAX_PIXEL_VALUE = 65535;
    int height = img.size();
    int width = img[0].size();

    // Adjust each column based on its average value
    for (int x = 0; x < width; ++x) {
        float colSum = 0;

        for (int y = 0; y < height; ++y)
            colSum += img[y][x];

        float colAvg = std::max(colSum / height, 1.0f);

        for (int y = 0; y < height; ++y)
            img[y][x] = static_cast<uint16_t>(std::fmin(img[y][x] * (X_SCALING_FACTOR / colAvg), MAX_PIXEL_VALUE));
    }
}

// Refined Y-axis stretching using interpolation for smoother stretching
void stretchImageY(std::vector<std::vector<uint16_t>>& img, float yStretchFactor) {
    int origHeight = img.size();
    int width = img[0].size();
    int newHeight = static_cast<int>(origHeight * yStretchFactor);
    
    std::vector<std::vector<uint16_t>> stretchedImg(newHeight, std::vector<uint16_t>(width));

    // Linear interpolation for smoother Y-stretching
    for (int y = 0; y < newHeight; ++y) {
        float srcY = y / yStretchFactor;
        int yLow = static_cast<int>(srcY);
        int yHigh = std::min(yLow + 1, origHeight - 1);
        float weight = srcY - yLow;

        for (int x = 0; x < width; ++x) {
            stretchedImg[y][x] = static_cast<uint16_t>(
                (1 - weight) * img[yLow][x] + weight * img[yHigh][x]
            );
        }
    }

    img = std::move(stretchedImg);
}

// Average two images (pixel by pixel)
std::vector<std::vector<uint16_t>> averageImages(const std::vector<std::vector<uint16_t>>& img1, const std::vector<std::vector<uint16_t>>& img2) {
    int height = img1.size();
    int width = img1[0].size();
    std::vector<std::vector<uint16_t>> result(height, std::vector<uint16_t>(width));

    // Calculate the average of each pixel
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            result[y][x] = (img1[y][x] + img2[y][x]) / 2;

    return result;
}

// Apply gamma correction to the image
void gammaCorrection(std::vector<std::vector<uint16_t>>& img, float gammaValue) {
    const float invGamma = 1.0f / gammaValue;
    const float MAX_PIXEL_VALUE = 65535.0f;
    int height = img.size();
    int width = img[0].size();

    // Apply gamma correction
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            float normalizedPixel = static_cast<float>(img[y][x]) / MAX_PIXEL_VALUE;
            img[y][x] = static_cast<uint16_t>(std::pow(normalizedPixel, invGamma) * MAX_PIXEL_VALUE);
        }
}

// Adjust image contrast
void adjustContrast(std::vector<std::vector<uint16_t>>& img, float contrastFactor) {
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;
    int height = img.size();
    int width = img[0].size();

    // Adjust contrast of each pixel
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            float pixel = static_cast<float>(img[y][x]);
            img[y][x] = static_cast<uint16_t>(std::fmin(std::fmax((pixel - midGray) * contrastFactor + midGray, 0.0f), MAX_PIXEL_VALUE));
        }
}

// Refined sharpening to improve clarity without introducing artifacts
void sharpenImage(std::vector<std::vector<uint16_t>>& img, float sharpenStrength) {
    int height = img.size();
    int width = img[0].size();
    std::vector<std::vector<uint16_t>> tempImg = img;

    // Sharpening kernel with refined strength
    float kernel[3][3] = {
        { 0, -1 * sharpenStrength,  0 },
        { -1 * sharpenStrength, 1 + 4 * sharpenStrength, -1 * sharpenStrength },
        { 0, -1 * sharpenStrength,  0 }
    };

    // Apply sharpening filter
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            float newPixelValue = 0;

            for (int ky = -1; ky <= 1; ++ky) {
                for (int kx = -1; kx <= 1; ++kx) {
                    newPixelValue += kernel[ky + 1][kx + 1] * img[y + ky][x + kx];
                }
            }

            tempImg[y][x] = static_cast<uint16_t>(std::fmin(std::fmax(newPixelValue, 0), 65535));
        }
    }

    img = std::move(tempImg);
}

// Process based on image region
void processBasedOnRegion(std::vector<std::vector<uint16_t>>& img) {
    const float HIGH_CONTRAST_FACTOR = 2.75f;
    const float LOW_CONTRAST_FACTOR = 1.15f;
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;
    int height = img.size();
    int width = img[0].size();

    // Process each pixel based on its brightness
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x) {
            float pixel = static_cast<float>(img[y][x]);

            // High contrast adjustment for dark pixels
            if (pixel < 5000) {
                img[y][x] = static_cast<uint16_t>(std::fmin(std::fmax((pixel - midGray) * HIGH_CONTRAST_FACTOR + midGray, 0.0f), MAX_PIXEL_VALUE));
                img[y][x] = static_cast<uint16_t>(std::fmin(img[y][x] * HIGH_CONTRAST_FACTOR, MAX_PIXEL_VALUE));
            }
            // Low contrast adjustment for brighter pixels
            else {
                img[y][x] = static_cast<uint16_t>(std::fmin(std::fmax((pixel - midGray) * LOW_CONTRAST_FACTOR + midGray, 0.0f), MAX_PIXEL_VALUE));
                img[y][x] = static_cast<uint16_t>(std::fmin(img[y][x] * LOW_CONTRAST_FACTOR, MAX_PIXEL_VALUE));
            }
        }
}

// Process and merge image parts from a text file
void processAndMergeImageParts(const std::string& txtFilePath, int linesToProcess, float darkPct, uint16_t darkThres, float yStretch, float contrastFactor, float sharpenStrength, float gammaValue) {
    std::ifstream inFile(txtFilePath);
    std::vector<std::vector<uint16_t>> data;
    std::string line;

    // Read image data from text file
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<uint16_t> row;
        uint16_t value;

        while (ss >> value)
            row.push_back(value);

        data.push_back(row);
    }

    int height = data.size();
    int width = data[0].size();
    int quarterWidth = width / 4;

    // Split the image into 4 parts
    std::vector<std::vector<uint16_t>> leftLeft(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> leftRight(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightLeft(height, std::vector<uint16_t>(quarterWidth));
    std::vector<std::vector<uint16_t>> rightRight(height, std::vector<uint16_t>(quarterWidth));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < quarterWidth; ++x) {
            leftLeft[y][x] = data[y][x];
            leftRight[y][x] = data[y][x + quarterWidth];
            rightLeft[y][x] = data[y][x + 2 * quarterWidth];
            rightRight[y][x] = data[y][x + 3 * quarterWidth];
        }
    }

    auto processPart = [&](auto& part) {
        processYAxis(part, linesToProcess, darkPct, darkThres);
        processXAxis(part);
        processBasedOnRegion(part);
    };

    // Process each image part
    processPart(leftLeft);
    processPart(leftRight);
    processPart(rightLeft);
    processPart(rightRight);

    // Average the left and right parts of the image
    auto lowImage = averageImages(leftLeft, leftRight);
    auto highImage = averageImages(rightLeft, rightRight);

    // Merge the low and high contrast images
    auto finalImage = averageImages(lowImage, highImage);

    // Apply further adjustments
    stretchImageY(finalImage, yStretch);
    adjustContrast(finalImage, contrastFactor);
    gammaCorrection(finalImage, gammaValue);
    sharpenImage(finalImage, sharpenStrength);

    // Save the final image as PNG
    savePNG("output_adjusted.png", finalImage, quarterWidth, static_cast<int>(height * yStretch));
}

int main() {
    int linesToProcess = 20;
    float darkPixelThreshold = 5.0f;
    uint16_t darkThreshold = 4096;
    float yStretchFactor = 3.5f;
    float contrastFactor = 1.09f;
    float sharpenStrength = 1.09f;
    float gammaValue = 0.6f;

    processAndMergeImageParts("010120240702142756_raw.txt", linesToProcess, darkPixelThreshold, darkThreshold, yStretchFactor, contrastFactor, sharpenStrength, gammaValue);
    return 0;
}