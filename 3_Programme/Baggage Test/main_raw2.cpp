#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>

#pragma pack(push, 1)
struct BMPFileHeader {
    uint16_t fileType{0x4D42}; 
    uint32_t fileSize{0};
    uint16_t reserved1{0};
    uint16_t reserved2{0};
    uint32_t offsetData{0};
};

struct BMPInfoHeader {
    uint32_t size{0};
    int32_t width{0};
    int32_t height{0};
    uint16_t planes{1};
    uint16_t bitCount{0};
    uint32_t compression{0};
    uint32_t sizeImage{0};
    int32_t xPixelsPerMeter{0};
    int32_t yPixelsPerMeter{0};
    uint32_t colorsUsed{0};
    uint32_t colorsImportant{0};
};
#pragma pack(pop)

void applySharpeningFilter(std::vector<std::vector<uint8_t>>& image, int iterations = 1) {
    int height = image.size();
    int width = image[0].size();
    std::vector<std::vector<uint8_t>> temp;

    int kernel[3][3] = { {-1, -1, -1},
                         {-1,  9, -1},
                         {-1, -1, -1} };

    for (int i = 0; i < iterations; ++i) {
        temp = image; 

        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                int newValue = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        newValue += temp[y + ky][x + kx] * kernel[ky + 1][kx + 1];
                    }
                }

                image[y][x] = std::min(std::max(newValue, 0), 255);
            }
        }
    }
}

void applyContrast(std::vector<std::vector<uint8_t>>& image, double contrastFactor) {
    int height = image.size();
    int width = image[0].size();

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int newValue = 128 + contrastFactor * (image[y][x] - 128);
            image[y][x] = std::min(std::max(newValue, 0), 255);
        }
    }
}

std::vector<std::vector<uint8_t>> zoomImage(const std::vector<std::vector<uint8_t>>& image, int zoomFactor) {
    int originalHeight = image.size();
    int originalWidth = image[0].size();
    int newHeight = originalHeight * zoomFactor;
    int newWidth = originalWidth * zoomFactor;

    std::vector<std::vector<uint8_t>> zoomedImage(newHeight, std::vector<uint8_t>(newWidth));

    for (int y = 0; y < newHeight; ++y) {
        for (int x = 0; x < newWidth; ++x) {

            int origY = y / zoomFactor;
            int origX = x / zoomFactor;
            zoomedImage[y][x] = image[origY][origX];
        }
    }

    return zoomedImage;
}

void createBMPFromData(const std::string& txtFilePath, const std::string& bmpFilePath, int zoomFactor, double contrastFactor, int sharpenIterations) {

    std::ifstream inFile(txtFilePath);
    if (!inFile) {
        std::cerr << "Error opening text file: " << txtFilePath << std::endl;
        return;
    }

    std::vector<std::vector<uint16_t>> data;
    std::string line;
    uint16_t minValue = 65535;
    uint16_t maxValue = 0;

    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<uint16_t> row;
        uint16_t value;
        while (ss >> value) {
            row.push_back(value);
            if (value < minValue && value > 0) minValue = value;
            if (value > maxValue) maxValue = value;
        }
        data.push_back(row);
    }
    inFile.close();

    if (data.empty()) {
        std::cerr << "Error: Text file is empty or not properly formatted." << std::endl;
        return;
    }

    int left = data[0].size(), right = 0, top = data.size(), bottom = 0;
    for (int y = 0; y < data.size(); ++y) {
        for (int x = 0; x < data[y].size(); ++x) {
            if (data[y][x] != 0) {
                if (x < left) left = x;
                if (x > right) right = x;
                if (y < top) top = y;
                if (y > bottom) bottom = y;
            }
        }
    }

    int width = right - left + 1;
    int height = bottom - top + 1;

    if (width <= 0 || height <= 0) {
        std::cerr << "Error: No non-zero data found in the file." << std::endl;
        return;
    }

    std::vector<std::vector<uint8_t>> image(height, std::vector<uint8_t>(width));
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            image[y][x] = static_cast<uint8_t>(255.0 * (data[top + y][left + x] - minValue) / (maxValue - minValue));
        }
    }

    applySharpeningFilter(image, sharpenIterations);

    applyContrast(image, contrastFactor);

    std::vector<std::vector<uint8_t>> zoomedImage = zoomImage(image, zoomFactor);

    int paddedRowSize = (zoomedImage[0].size() * 3 + 3) & (~3);
    int dataSize = paddedRowSize * zoomedImage.size();

    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    infoHeader.size = sizeof(BMPInfoHeader);
    infoHeader.width = zoomedImage[0].size();
    infoHeader.height = zoomedImage.size();
    infoHeader.bitCount = 24;  
    infoHeader.compression = 0;
    infoHeader.sizeImage = dataSize;
    fileHeader.fileSize = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + dataSize;
    fileHeader.offsetData = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader);

    std::ofstream outFile(bmpFilePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error creating BMP file: " << bmpFilePath << std::endl;
        return;
    }

    outFile.write(reinterpret_cast<char*>(&fileHeader), sizeof(fileHeader));
    outFile.write(reinterpret_cast<char*>(&infoHeader), sizeof(infoHeader));

    for (int y = zoomedImage.size() - 1; y >= 0; --y) { 
        for (int x = 0; x < zoomedImage[0].size(); ++x) {
            uint8_t color[] = { zoomedImage[y][x], zoomedImage[y][x], zoomedImage[y][x] }; 
            outFile.write(reinterpret_cast<char*>(color), 3);
        }
        uint8_t padding[3] = { 0, 0, 0 }; 
        outFile.write(reinterpret_cast<char*>(padding), paddedRowSize - zoomedImage[0].size() * 3);
    }

    outFile.close();
    std::cout << "BMP file created successfully." << std::endl;
}

int main() {
    std::string txtFilePath = "Raw 2.txt"; 
    std::string bmpFilePath = "output_new.bmp";  
    int zoomFactor = 2; 
    double contrastFactor = 1.15; 
    int sharpenIterations = 1.80; 

    createBMPFromData(txtFilePath, bmpFilePath, zoomFactor, contrastFactor, sharpenIterations);

    return 0;
}
