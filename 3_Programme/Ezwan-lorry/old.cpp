#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <limits>

void savePNG(const std::string& path, const std::vector<std::vector<uint16_t>>& img, int width, int height) {
    std::vector<uint8_t> imgData(width * height * 3);

    // Convert 16-bit image to 8-bit RGB and store in imgData
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint8_t pixel = static_cast<uint8_t>(img[y][x] >> 8);  // Convert 16-bit to 8-bit
            std::fill(&imgData[3 * (y * width + x)], &imgData[3 * (y * width + x) + 3], pixel);  // Set RGB channels
        }
    }

    stbi_write_png(path.c_str(), width, height, 3, imgData.data(), width * 3);
}

void createPNGFromData(const std::string& txtFilePath, const std::string& pngFilePath) {
    std::ifstream infile(txtFilePath);
    std::vector<std::vector<uint16_t>> data;
    std::string line;

    // Read data from the text file
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::vector<uint16_t> row;
        uint16_t value;
        while (iss >> value) {
            row.push_back(value);
        }
        data.push_back(row);
    }

    // Assume all rows are of equal length
    int width = data.empty() ? 0 : data[0].size();
    int height = data.size();

    // Ensure image dimensions are valid
    if (width == 0 || height == 0) {
        std::cerr << "No valid data to create image." << std::endl;
        return;
    }

    std::vector<std::vector<uint16_t>> image(height, std::vector<uint16_t>(width));

    // Determine min and max values
    uint16_t minValue = std::numeric_limits<uint16_t>::max();
    uint16_t maxValue = std::numeric_limits<uint16_t>::min();

    for (const auto& row : data) {
        for (const auto& val : row) {
            if (val < minValue) minValue = val;
            if (val > maxValue) maxValue = val;
        }
    }

    // Fill the image data
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            image[y][x] = static_cast<uint16_t>(255.0 * (data[y][x] - minValue) / (maxValue - minValue));
        }
    }

    savePNG(pngFilePath, image, width, height);
}

int main() {
    std::string txtFilePath = "ezwan-lorry-4800-1.txt";
    std::string pngFilePath = "output_new.png";

    createPNGFromData(txtFilePath, pngFilePath);

    return 0;
}
