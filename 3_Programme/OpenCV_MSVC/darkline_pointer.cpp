#include "darkline_pointer.h"
#include <QDebug>
#include <algorithm>
#include <iostream>
#include <stdexcept>

// ImageData implementation of special member functions
ImageData::ImageData(const ImageData& other) : data(nullptr), rows(0), cols(0) {
    if (other.data && other.rows > 0 && other.cols > 0) {
        rows = other.rows;
        cols = other.cols;
        data = new double*[rows];
        for (int i = 0; i < rows; i++) {
            data[i] = new double[cols];
            std::copy(other.data[i], other.data[i] + cols, data[i]);
        }
    }
}

ImageData& ImageData::operator=(const ImageData& other) {
    if (this != &other) {
        // Clean up existing data
        if (data) {
            for (int i = 0; i < rows; i++) {
                delete[] data[i];
            }
            delete[] data;
        }

        if (other.data && other.rows > 0 && other.cols > 0) {
            rows = other.rows;
            cols = other.cols;
            data = new double*[rows];
            for (int i = 0; i < rows; i++) {
                data[i] = new double[cols];
                std::copy(other.data[i], other.data[i] + cols, data[i]);
            }
        } else {
            data = nullptr;
            rows = 0;
            cols = 0;
        }
    }
    return *this;
}

ImageData::ImageData(ImageData&& other) noexcept
    : data(other.data), rows(other.rows), cols(other.cols) {
    other.data = nullptr;
    other.rows = 0;
    other.cols = 0;
}

ImageData& ImageData::operator=(ImageData&& other) noexcept {
    if (this != &other) {
        // Clean up existing data
        if (data) {
            for (int i = 0; i < rows; i++) {
                delete[] data[i];
            }
            delete[] data;
        }

        data = other.data;
        rows = other.rows;
        cols = other.cols;

        other.data = nullptr;
        other.rows = 0;
        other.cols = 0;
    }
    return *this;
}

// DarkLinePointerProcessor helper functions implementation
std::unique_ptr<ImageData> DarkLinePointerProcessor::createImageCopy(const ImageData& source) {
    auto newImage = std::make_unique<ImageData>();
    newImage->rows = source.rows;
    newImage->cols = source.cols;
    newImage->data = new double*[source.rows];

    for (int i = 0; i < source.rows; i++) {
        newImage->data[i] = new double[source.cols];
        std::copy(source.data[i], source.data[i] + source.cols, newImage->data[i]);
    }

    return newImage;
}

void DarkLinePointerProcessor::copyImageData(const ImageData& source, ImageData& destination) {
    // Clean up existing destination data
    if (destination.data) {
        for (int i = 0; i < destination.rows; i++) {
            delete[] destination.data[i];
        }
        delete[] destination.data;
    }

    destination.rows = source.rows;
    destination.cols = source.cols;
    destination.data = new double*[source.rows];

    for (int i = 0; i < source.rows; i++) {
        destination.data[i] = new double[source.cols];
        std::copy(source.data[i], source.data[i] + source.cols, destination.data[i]);
    }
}

void DarkLinePointerProcessor::resizeImageData(ImageData& image, int newRows, int newCols) {
    if (newRows <= 0 || newCols <= 0) {
        throw std::invalid_argument("Invalid dimensions for image resize");
    }

    // Create new data array with new dimensions
    double** newData = new double*[newRows];
    for (int i = 0; i < newRows; i++) {
        newData[i] = new double[newCols];
        // Initialize with zeros
        std::fill(newData[i], newData[i] + newCols, 0.0);
    }

    // Copy existing data within bounds
    int copyRows = std::min(image.rows, newRows);
    int copyCols = std::min(image.cols, newCols);

    for (int i = 0; i < copyRows; i++) {
        std::copy(image.data[i], image.data[i] + copyCols, newData[i]);
    }

    // Clean up old data
    for (int i = 0; i < image.rows; i++) {
        delete[] image.data[i];
    }
    delete[] image.data;

    // Update image with new data
    image.data = newData;
    image.rows = newRows;
    image.cols = newCols;
}

// DarkLineArray helper functions
DarkLineArray* DarkLinePointerProcessor::createDarkLineArray(int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        throw std::invalid_argument("Invalid dimensions for DarkLineArray creation");
    }

    DarkLineArray* array = new DarkLineArray();
    array->rows = rows;
    array->cols = cols;

    try {
        array->lines = new DarkLine*[rows];
        for (int i = 0; i < rows; i++) {
            array->lines[i] = new DarkLine[cols];
            // Initialize with default values
            for (int j = 0; j < cols; j++) {
                array->lines[i][j] = DarkLine();
            }
        }
    } catch (...) {
        destroyDarkLineArray(array);
        throw;
    }

    return array;
}

void DarkLinePointerProcessor::destroyDarkLineArray(DarkLineArray* array) {
    if (array) {
        if (array->lines) {
            for (int i = 0; i < array->rows; i++) {
                delete[] array->lines[i];
            }
            delete[] array->lines;
            array->lines = nullptr;
        }
        array->rows = 0;
        array->cols = 0;
        delete array;
    }
}

void DarkLinePointerProcessor::copyDarkLineArray(const DarkLineArray* source, DarkLineArray* destination) {
    if (!source || !destination) {
        throw std::invalid_argument("Invalid source or destination for DarkLineArray copy");
    }

    // Clean up existing destination data
    if (destination->lines) {
        for (int i = 0; i < destination->rows; i++) {
            delete[] destination->lines[i];
        }
        delete[] destination->lines;
    }

    destination->rows = source->rows;
    destination->cols = source->cols;

    if (source->lines) {
        destination->lines = new DarkLine*[source->rows];
        for (int i = 0; i < source->rows; i++) {
            destination->lines[i] = new DarkLine[source->cols];
            for (int j = 0; j < source->cols; j++) {
                destination->lines[i][j] = source->lines[i][j];
            }
        }
    } else {
        destination->lines = nullptr;
    }
}

// Basic helper functions
int DarkLinePointerProcessor::calculateSearchRadius(int lineWidth) {
    return std::clamp(lineWidth * 2, MIN_SEARCH_RADIUS, MAX_SEARCH_RADIUS);
}

// Part 2: Dark Line Detection Implementation

bool DarkLinePointerProcessor::isInObject(
    const ImageData& image,
    int pos,
    int lineWidth,
    bool isVertical,
    int threadId) {

    if (isVertical) {
        std::cout << "Thread " << threadId << " checking vertical line at x=" << pos << std::endl;

        for (int y = 0; y < image.rows; ++y) {
            // Check left side
            for (int i = 1; i <= VERTICAL_CHECK_RANGE; ++i) {
                if (pos - i >= 0) {
                    double leftPixel = image.data[y][pos - i];
                    if (leftPixel < WHITE_THRESHOLD) {
                        qDebug() << "Thread" << threadId << "- Vertical line at x=" << pos
                                 << ": left pixel at y=" << y << "value=" << leftPixel;
                        return true;
                    }
                }
            }

            // Check right side
            for (int i = 1; i <= VERTICAL_CHECK_RANGE; ++i) {
                if (pos + lineWidth + i - 1 < image.cols) {
                    double rightPixel = image.data[y][pos + lineWidth + i - 1];
                    if (rightPixel < WHITE_THRESHOLD) {
                        qDebug() << "Thread" << threadId << "- Vertical line at x=" << pos
                                 << ": right pixel at y=" << y << "value=" << rightPixel;
                        return true;
                    }
                }
            }
        }
    } else {
        std::cout << "Thread " << threadId << " checking horizontal line at y=" << pos << std::endl;

        // Sample pixels above the line
        for (int i = 1; i <= HORIZONTAL_CHECK_RANGE; ++i) {
            if (pos - i >= 0) {
                // Sample at regular intervals across the width
                for (int x = 0; x < image.cols; x += image.cols/10) {
                    double topPixel = image.data[pos - i][x];
                    if (topPixel < WHITE_THRESHOLD) {
                        qDebug() << "Thread" << threadId << "- Found dark pixel above line at x=" << x;
                        return true;
                    }
                }
            }
        }

        // Sample pixels below the line
        for (int i = 1; i <= HORIZONTAL_CHECK_RANGE; ++i) {
            if (pos + lineWidth + i - 1 < image.rows) {
                for (int x = 0; x < image.cols; x += image.cols/10) {
                    double bottomPixel = image.data[pos + lineWidth + i - 1][x];
                    if (bottomPixel < WHITE_THRESHOLD) {
                        qDebug() << "Thread" << threadId << "- Found dark pixel below line at x=" << x;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

DarkLineArray* DarkLinePointerProcessor::detectDarkLines(const ImageData& image) {
    if (!image.data || image.rows == 0 || image.cols == 0) {
        return nullptr;
    }

    const int numThreads = std::thread::hardware_concurrency();
    std::mutex detectedLinesMutex;

    // Create temporary arrays for detection
    bool* isBlackColumn = new bool[image.cols]();
    bool* isBlackRow = new bool[image.rows]();

    // Detect vertical lines using multiple threads
    std::vector<std::thread> verticalThreads;
    int columnsPerThread = image.cols / numThreads;

    std::cout << "Starting vertical line detection with " << numThreads << " threads" << std::endl;

    // First pass: Identify black columns
    for (int t = 0; t < numThreads; ++t) {
        int startX = t * columnsPerThread;
        int endX = (t == numThreads - 1) ? image.cols : startX + columnsPerThread;

        verticalThreads.emplace_back([=, &isBlackColumn, &image]() {
            for (int x = startX; x < endX; ++x) {
                int blackPixelCount = 0;
                for (int y = 0; y < image.rows; ++y) {
                    if (image.data[y][x] <= BLACK_THRESHOLD) {
                        blackPixelCount++;
                    }
                }
                double blackRatio = static_cast<double>(blackPixelCount) / image.rows;
                if (blackRatio > (1.0 - NOISE_TOLERANCE)) {
                    isBlackColumn[x] = true;
                }
            }
        });
    }

    // Wait for vertical detection threads
    for (auto& thread : verticalThreads) {
        thread.join();
    }

    // Count vertical lines to allocate array
    int verticalLineCount = 0;
    std::vector<std::pair<int, int>> verticalLines; // <startX, width>
    int startX = -1;

    for (int x = 0; x < image.cols; ++x) {
        if (isBlackColumn[x]) {
            if (startX == -1) startX = x;
        } else if (startX != -1) {
            int width = x - startX;
            if (width >= MIN_LINE_WIDTH) {
                verticalLines.push_back({startX, width});
                verticalLineCount++;
            }
            startX = -1;
        }
    }

    // Check for line extending to edge
    if (startX != -1) {
        int width = image.cols - startX;
        if (width >= MIN_LINE_WIDTH) {
            verticalLines.push_back({startX, width});
            verticalLineCount++;
        }
    }

    // Detect horizontal lines
    std::vector<std::thread> horizontalThreads;
    int rowsPerThread = image.rows / numThreads;

    // First pass: Identify black rows
    for (int t = 0; t < numThreads; ++t) {
        int startY = t * rowsPerThread;
        int endY = (t == numThreads - 1) ? image.rows : startY + rowsPerThread;

        horizontalThreads.emplace_back([=, &isBlackRow, &image]() {
            for (int y = startY; y < endY; ++y) {
                int blackPixelCount = 0;
                for (int x = 0; x < image.cols; ++x) {
                    if (image.data[y][x] <= BLACK_THRESHOLD) {
                        blackPixelCount++;
                    }
                }
                double blackRatio = static_cast<double>(blackPixelCount) / image.cols;
                if (blackRatio > (1.0 - NOISE_TOLERANCE)) {
                    isBlackRow[y] = true;
                }
            }
        });
    }

    // Wait for horizontal detection threads
    for (auto& thread : horizontalThreads) {
        thread.join();
    }

    // Count horizontal lines
    int horizontalLineCount = 0;
    std::vector<std::pair<int, int>> horizontalLines; // <startY, width>
    int startY = -1;

    for (int y = 0; y < image.rows; ++y) {
        if (isBlackRow[y]) {
            if (startY == -1) startY = y;
        } else if (startY != -1) {
            int width = y - startY;
            if (width >= MIN_LINE_WIDTH) {
                horizontalLines.push_back({startY, width});
                horizontalLineCount++;
            }
            startY = -1;
        }
    }

    // Check for line extending to edge
    if (startY != -1) {
        int width = image.rows - startY;
        if (width >= MIN_LINE_WIDTH) {
            horizontalLines.push_back({startY, width});
            horizontalLineCount++;
        }
    }

    // Clean up temporary arrays
    delete[] isBlackColumn;
    delete[] isBlackRow;

    // Create DarkLineArray to store results
    DarkLineArray* result = createDarkLineArray(verticalLineCount + horizontalLineCount, 1);

    // Fill the array with detected lines
    int currentIndex = 0;

    // Add vertical lines
    for (const auto& [x, width] : verticalLines) {
        DarkLine& line = result->lines[currentIndex][0];
        line.x = x;
        line.y = 0;
        line.startY = 0;
        line.endY = image.rows - 1;
        line.width = width;
        line.isVertical = true;
        line.inObject = isInObject(image, x, width, true, 0);
        currentIndex++;
    }

    // Add horizontal lines
    for (const auto& [y, width] : horizontalLines) {
        DarkLine& line = result->lines[currentIndex][0];
        line.x = 0;
        line.y = y;
        line.startX = 0;
        line.endX = image.cols - 1;
        line.width = width;
        line.isVertical = false;
        line.inObject = isInObject(image, y, width, false, 0);
        currentIndex++;
    }

    std::cout << "Dark line detection completed. Found " << result->rows << " lines." << std::endl;
    return result;
}

// Part 3: Basic Line Removal Functions Implementation

double DarkLinePointerProcessor::findReplacementValue(
    const ImageData& image,
    int x, int y,
    bool isVertical,
    int lineWidth) {

    std::vector<double> validValues;
    int searchRadius = calculateSearchRadius(lineWidth);
    validValues.reserve(searchRadius * 2);

    if (isVertical) {
        // Look for valid pixels horizontally
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Look left
            int leftX = x - offset;
            if (leftX >= 0 && image.data[y][leftX] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[y][leftX]);
            }

            // Look right
            int rightX = x + offset;
            if (rightX < image.cols && image.data[y][rightX] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[y][rightX]);
            }
        }
    } else {
        // Look for valid pixels vertically
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Look up
            int upY = y - offset;
            if (upY >= 0 && image.data[upY][x] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[upY][x]);
            }

            // Look down
            int downY = y + offset;
            if (downY < image.rows && image.data[downY][x] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[downY][x]);
            }
        }
    }

    // Return median value if we found valid pixels, otherwise return original value
    if (!validValues.empty()) {
        std::sort(validValues.begin(), validValues.end());
        return validValues[validValues.size() / 2]; // Median value
    }
    return image.data[y][x]; // Keep original if no valid replacements found
}

double DarkLinePointerProcessor::findStitchValue(
    const ImageData& image,
    const DarkLine& line,
    int x, int y) {

    if (line.isVertical) {
        int leftX = line.x - 1;
        int rightX = line.x + line.width;

        if (leftX >= 0 && rightX < image.cols) {
            // Use weighted average based on position within the line
            double relativePos = static_cast<double>(x - line.x) / line.width;
            return (1.0 - relativePos) * image.data[y][leftX] +
                   relativePos * image.data[y][rightX];
        } else if (leftX >= 0) {
            return image.data[y][leftX];
        } else if (rightX < image.cols) {
            return image.data[y][rightX];
        }
    } else {
        int topY = line.y - 1;
        int bottomY = line.y + line.width;

        if (topY >= 0 && bottomY < image.rows) {
            // Use weighted average based on position within the line
            double relativePos = static_cast<double>(y - line.y) / line.width;
            return (1.0 - relativePos) * image.data[topY][x] +
                   relativePos * image.data[bottomY][x];
        } else if (topY >= 0) {
            return image.data[topY][x];
        } else if (bottomY < image.rows) {
            return image.data[bottomY][x];
        }
    }

    return image.data[y][x]; // Return original value if no stitch is possible
}

void DarkLinePointerProcessor::removeDarkLines(ImageData& image, const DarkLineArray* lines) {
    if (!lines || !lines->lines || lines->rows == 0) {
        return;
    }

    try {
        // Create new image array for modifications
        auto tempImage = createImageCopy(image);
        bool modificationsApplied = false;

        // Process each line in the array
        for (int i = 0; i < lines->rows; i++) {
            for (int j = 0; j < lines->cols; j++) {
                const DarkLine& line = lines->lines[i][j];

                // Skip lines that are part of objects
                if (line.inObject) continue;

                if (line.isVertical) {
                    // Process vertical lines
                    for (int x = line.x; x < std::min(line.x + line.width, image.cols); ++x) {
                        for (int y = line.startY; y <= line.endY && y < image.rows; ++y) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, true, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                } else {
                    // Process horizontal lines
                    for (int y = line.y; y < std::min(line.y + line.width, image.rows); ++y) {
                        for (int x = line.startX; x <= line.endX && x < image.cols; ++x) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, false, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                }
            }
        }

        // Only update the original image if modifications were made
        if (modificationsApplied) {
            copyImageData(*tempImage, image);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in removeDarkLines: " << e.what() << std::endl;
        throw;
    }
}

void DarkLinePointerProcessor::removeAllDarkLines(ImageData& image, DarkLineArray* lines) {
    if (!lines || !lines->lines || lines->rows == 0) {
        return;
    }

    try {
        // Create temporary image for processing
        auto tempImage = createImageCopy(image);
        bool modificationsApplied = false;

        // Process all lines in the array
        for (int i = 0; i < lines->rows; i++) {
            for (int j = 0; j < lines->cols; j++) {
                const DarkLine& line = lines->lines[i][j];

                if (line.isVertical) {
                    for (int x = line.x; x < std::min(line.x + line.width, image.cols); ++x) {
                        for (int y = line.startY; y <= line.endY && y < image.rows; ++y) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, true, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                } else {
                    for (int y = line.y; y < std::min(line.y + line.width, image.rows); ++y) {
                        for (int x = line.startX; x <= line.endX && x < image.cols; ++x) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, false, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                }
            }
        }

        // Update original image if modifications were made
        if (modificationsApplied) {
            copyImageData(*tempImage, image);

            // Clear the lines array
            destroyDarkLineArray(lines);
            lines = createDarkLineArray(0, 0);
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in removeAllDarkLines: " << e.what() << std::endl;
        throw;
    }
}

// Part 4A: Selective Line Removal Implementation

void DarkLinePointerProcessor::removeDarkLinesSelective(
    ImageData& image,
    const DarkLineArray* lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (!lines || !lines->lines || lines->rows == 0) {
        return;
    }

    try {
        auto tempImage = createImageCopy(image);
        bool modificationsApplied = false;

        // Process each line based on selection criteria
        for (int i = 0; i < lines->rows; i++) {
            for (int j = 0; j < lines->cols; j++) {
                const DarkLine& line = lines->lines[i][j];

                // Skip lines that don't match our removal criteria
                if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
                    continue;
                }

                if (method == RemovalMethod::DIRECT_STITCH) {
                    // Handle direct stitching method
                    if (line.isVertical) {
                        // Calculate new dimensions for vertical line removal
                        int newWidth = image.cols - line.width;

                        // Create temporary buffer for the current row
                        std::vector<double> rowBuffer(newWidth);

                        for (int y = 0; y < image.rows; ++y) {
                            // Copy data before the line
                            std::copy(tempImage->data[y],
                                      tempImage->data[y] + line.x,
                                      rowBuffer.begin());

                            // Copy data after the line
                            std::copy(tempImage->data[y] + line.x + line.width,
                                      tempImage->data[y] + image.cols,
                                      rowBuffer.begin() + line.x);

                            // Copy processed row back to image
                            std::copy(rowBuffer.begin(), rowBuffer.end(), tempImage->data[y]);
                        }

                        // Update image dimensions
                        tempImage->cols = newWidth;
                        modificationsApplied = true;

                    } else {
                        // Handle horizontal line removal
                        int newHeight = image.rows - line.width;

                        // Shift rows up to remove the line
                        for (int y = line.y; y < newHeight; ++y) {
                            std::copy(tempImage->data[y + line.width],
                                      tempImage->data[y + line.width] + image.cols,
                                      tempImage->data[y]);
                        }

                        // Update image dimensions
                        tempImage->rows = newHeight;
                        modificationsApplied = true;
                    }
                } else {  // NEIGHBOR_VALUES method
                    if (line.isVertical) {
                        for (int x = line.x; x < std::min(line.x + line.width, image.cols); ++x) {
                            for (int y = line.startY; y <= line.endY && y < image.rows; ++y) {
                                if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                    tempImage->data[y][x] = findReplacementValue(image, x, y, true, line.width);
                                    modificationsApplied = true;
                                }
                            }
                        }
                    } else {
                        for (int y = line.y; y < std::min(line.y + line.width, image.rows); ++y) {
                            for (int x = line.startX; x <= line.endX && x < image.cols; ++x) {
                                if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                    tempImage->data[y][x] = findReplacementValue(image, x, y, false, line.width);
                                    modificationsApplied = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Update the original image if modifications were made
        if (modificationsApplied) {
            copyImageData(*tempImage, image);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error in removeDarkLinesSelective: " << e.what() << std::endl;
        throw;
    }
}

void DarkLinePointerProcessor::removeInObjectDarkLines(
    ImageData& image,
    DarkLineArray* lines) {

    if (!lines || !lines->lines || lines->rows == 0) {
        return;
    }

    try {
        auto tempImage = createImageCopy(image);
        bool modificationsApplied = false;

        // Create array to track which lines to keep
        bool* keepLine = new bool[lines->rows]();
        int remainingLines = 0;

        // Process and count remaining lines
        for (int i = 0; i < lines->rows; i++) {
            for (int j = 0; j < lines->cols; j++) {
                const DarkLine& line = lines->lines[i][j];
                if (!line.inObject) {
                    // Keep non-object lines
                    keepLine[i] = true;
                    remainingLines++;
                    continue;
                }

                // Process in-object lines
                if (line.isVertical) {
                    for (int x = line.x; x < std::min(line.x + line.width, image.cols); ++x) {
                        for (int y = line.startY; y <= line.endY && y < image.rows; ++y) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, true, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                } else {
                    for (int y = line.y; y < std::min(line.y + line.width, image.rows); ++y) {
                        for (int x = line.startX; x <= line.endX && x < image.cols; ++x) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, false, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                }
            }
        }

        // Update the original image if modifications were made
        if (modificationsApplied) {
            copyImageData(*tempImage, image);

            // Create new array with only remaining lines
            DarkLineArray* newLines = createDarkLineArray(remainingLines, lines->cols);
            int newIndex = 0;

            for (int i = 0; i < lines->rows; i++) {
                if (keepLine[i]) {
                    for (int j = 0; j < lines->cols; j++) {
                        newLines->lines[newIndex][j] = lines->lines[i][j];
                    }
                    newIndex++;
                }
            }

            // Replace old lines array with new one
            destroyDarkLineArray(lines);
            *lines = *newLines;
            newLines = nullptr;  // Ownership transferred
        }

        delete[] keepLine;

    } catch (const std::exception& e) {
        std::cerr << "Error in removeInObjectDarkLines: " << e.what() << std::endl;
        throw;
    }
}

void DarkLinePointerProcessor::removeIsolatedDarkLines(
    ImageData& image,
    DarkLineArray* lines) {

    if (!lines || !lines->lines || lines->rows == 0) {
        return;
    }

    try {
        auto tempImage = createImageCopy(image);
        bool modificationsApplied = false;

        // Create array to track which lines to keep
        bool* keepLine = new bool[lines->rows]();
        int remainingLines = 0;

        // Process and count remaining lines
        for (int i = 0; i < lines->rows; i++) {
            for (int j = 0; j < lines->cols; j++) {
                const DarkLine& line = lines->lines[i][j];
                if (line.inObject) {
                    // Keep in-object lines
                    keepLine[i] = true;
                    remainingLines++;
                    continue;
                }

                // Process isolated lines
                if (line.isVertical) {
                    for (int x = line.x; x < std::min(line.x + line.width, image.cols); ++x) {
                        for (int y = line.startY; y <= line.endY && y < image.rows; ++y) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, true, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                } else {
                    for (int y = line.y; y < std::min(line.y + line.width, image.rows); ++y) {
                        for (int x = line.startX; x <= line.endX && x < image.cols; ++x) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempImage->data[y][x] = findReplacementValue(image, x, y, false, line.width);
                                modificationsApplied = true;
                            }
                        }
                    }
                }
            }
        }

        // Update the original image if modifications were made
        if (modificationsApplied) {
            copyImageData(*tempImage, image);

            // Create new array with only remaining lines
            DarkLineArray* newLines = createDarkLineArray(remainingLines, lines->cols);
            int newIndex = 0;

            for (int i = 0; i < lines->rows; i++) {
                if (keepLine[i]) {
                    for (int j = 0; j < lines->cols; j++) {
                        newLines->lines[newIndex][j] = lines->lines[i][j];
                    }
                    newIndex++;
                }
            }

            // Replace old lines array with new one
            destroyDarkLineArray(lines);
            *lines = *newLines;
            newLines = nullptr;  // Ownership transferred
        }

        delete[] keepLine;

    } catch (const std::exception& e) {
        std::cerr << "Error in removeIsolatedDarkLines: " << e.what() << std::endl;
        throw;
    }
}

void DarkLinePointerProcessor::removeDarkLinesSequential(
    ImageData& image,
    DarkLineArray* lines,
    DarkLine** selectedLines,
    int selectedCount,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (!lines || !lines->lines || lines->rows == 0) {
        return;
    }

    try {
        // Create temporary image for modifications
        auto tempImage = createImageCopy(image);
        bool modificationsApplied = false;

        if (removeInObject && selectedLines && selectedCount > 0) {
            // Process selected in-object lines
            for (int i = 0; i < selectedCount; i++) {
                const DarkLine* line = selectedLines[i];
                if (!line->inObject) continue;  // Skip if not an in-object line

                if (method == RemovalMethod::DIRECT_STITCH) {
                    if (line->isVertical) {
                        // Handle vertical line removal with direct stitch
                        for (int y = line->startY; y <= line->endY && y < tempImage->rows; ++y) {
                            // Shift pixels left to close the gap
                            for (int x = line->x; x < tempImage->cols - line->width; ++x) {
                                tempImage->data[y][x] = tempImage->data[y][x + line->width];
                            }
                        }
                        modificationsApplied = true;
                    } else {
                        // Handle horizontal line removal with direct stitch
                        for (int y = line->y; y < std::min(line->y + line->width, tempImage->rows); ++y) {
                            for (int x = line->startX; x <= line->endX && x < tempImage->cols; ++x) {
                                if (y + line->width < tempImage->rows) {
                                    tempImage->data[y][x] = tempImage->data[y + line->width][x];
                                }
                            }
                        }
                        modificationsApplied = true;
                    }
                } else { // NEIGHBOR_VALUES method
                    if (line->isVertical) {
                        for (int x = line->x; x < std::min(line->x + line->width, tempImage->cols); ++x) {
                            for (int y = line->startY; y <= line->endY && y < tempImage->rows; ++y) {
                                if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                    tempImage->data[y][x] = findReplacementValue(image, x, y, true, line->width);
                                    modificationsApplied = true;
                                }
                            }
                        }
                    } else {
                        for (int y = line->y; y < std::min(line->y + line->width, tempImage->rows); ++y) {
                            for (int x = line->startX; x <= line->endX && x < tempImage->cols; ++x) {
                                if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                    tempImage->data[y][x] = findReplacementValue(image, x, y, false, line->width);
                                    modificationsApplied = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        if (removeIsolated) {
            // Process all isolated lines
            for (int i = 0; i < lines->rows; i++) {
                for (int j = 0; j < lines->cols; j++) {
                    const DarkLine& line = lines->lines[i][j];
                    if (line.inObject) continue;  // Skip in-object lines

                    if (method == RemovalMethod::DIRECT_STITCH) {
                        if (line.isVertical) {
                            for (int y = line.startY; y <= line.endY && y < tempImage->rows; ++y) {
                                for (int x = line.x; x < tempImage->cols - line.width; ++x) {
                                    tempImage->data[y][x] = tempImage->data[y][x + line.width];
                                }
                            }
                        } else {
                            for (int y = line.y; y < std::min(line.y + line.width, tempImage->rows); ++y) {
                                for (int x = line.startX; x <= line.endX && x < tempImage->cols; ++x) {
                                    if (y + line.width < tempImage->rows) {
                                        tempImage->data[y][x] = tempImage->data[y + line.width][x];
                                    }
                                }
                            }
                        }
                        modificationsApplied = true;
                    } else {
                        if (line.isVertical) {
                            for (int x = line.x; x < std::min(line.x + line.width, tempImage->cols); ++x) {
                                for (int y = line.startY; y <= line.endY && y < tempImage->rows; ++y) {
                                    if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                        tempImage->data[y][x] = findReplacementValue(image, x, y, true, line.width);
                                        modificationsApplied = true;
                                    }
                                }
                            }
                        } else {
                            for (int y = line.y; y < std::min(line.y + line.width, tempImage->rows); ++y) {
                                for (int x = line.startX; x <= line.endX && x < tempImage->cols; ++x) {
                                    if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                        tempImage->data[y][x] = findReplacementValue(image, x, y, false, line.width);
                                        modificationsApplied = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Update the original image if modifications were made
        if (modificationsApplied) {
            copyImageData(*tempImage, image);
        }

    } catch (const std::exception& e) {
        std::cerr << "Error in removeDarkLinesSequential: " << e.what() << std::endl;
        throw;
    }
}
