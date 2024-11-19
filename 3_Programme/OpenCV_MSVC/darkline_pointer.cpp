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
    std::cout << "Creating DarkLineArray with dimensions: " << rows << "x" << cols << std::endl;

    if (rows <= 0 || cols <= 0) {
        std::cout << "Creating empty array for invalid dimensions" << std::endl;
        DarkLineArray* emptyArray = new DarkLineArray();
        emptyArray->rows = 0;
        emptyArray->cols = 0;
        emptyArray->lines = nullptr;
        return emptyArray;
    }

    try {
        return createSafeDarkLineArray(rows, cols);
    } catch (const std::exception& e) {
        std::cerr << "Error in createDarkLineArray: " << e.what() << std::endl;
        return nullptr;
    }
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
    // 1. Validate input parameters
    if (!image.data || image.rows == 0 || image.cols == 0) {
        std::cerr << "Invalid input image parameters" << std::endl;
        return nullptr;
    }

    try {
        // 2. Initialize detection related variables
        const int numThreads = std::thread::hardware_concurrency();
        std::mutex detectedLinesMutex;
        std::cout << "Starting detection with " << numThreads << " threads" << std::endl;

        // 3. Create temporary arrays for detection
        bool* isBlackColumn = new bool[image.cols]();  // Initialize with false
        bool* isBlackRow = new bool[image.rows]();     // Initialize with false
        std::thread** verticalThreads = new std::thread*[numThreads];
        int columnsPerThread = image.cols / numThreads;

        // 4. First pass: Detect vertical lines using multiple threads
        std::cout << "Starting vertical line detection..." << std::endl;
        for (int t = 0; t < numThreads; ++t) {
            int startX = t * columnsPerThread;
            int endX = (t == numThreads - 1) ? image.cols : startX + columnsPerThread;

            verticalThreads[t] = new std::thread([=, &isBlackColumn, &image]() {
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

        // 5. Wait for vertical detection threads
        for (int t = 0; t < numThreads; ++t) {
            verticalThreads[t]->join();
            delete verticalThreads[t];
        }
        delete[] verticalThreads;

        // 6. Process vertical lines
        std::pair<int, int>* verticalLines = new std::pair<int, int>[image.cols];  // Maximum possible vertical lines
        int verticalLineCount = 0;
        int startX = -1;

        for (int x = 0; x < image.cols; ++x) {
            if (isBlackColumn[x]) {
                if (startX == -1) startX = x;
            } else if (startX != -1) {
                int width = x - startX;
                if (width >= MIN_LINE_WIDTH) {
                    verticalLines[verticalLineCount++] = std::make_pair(startX, width);
                }
                startX = -1;
            }
        }

        // Check for line extending to edge
        if (startX != -1) {
            int width = image.cols - startX;
            if (width >= MIN_LINE_WIDTH) {
                verticalLines[verticalLineCount++] = std::make_pair(startX, width);
            }
        }

        // 7. Detect horizontal lines
        std::thread** horizontalThreads = new std::thread*[numThreads];
        int rowsPerThread = image.rows / numThreads;

        std::cout << "Starting horizontal line detection..." << std::endl;
        for (int t = 0; t < numThreads; ++t) {
            int startY = t * rowsPerThread;
            int endY = (t == numThreads - 1) ? image.rows : startY + rowsPerThread;

            horizontalThreads[t] = new std::thread([=, &isBlackRow, &image]() {
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

        // 8. Wait for horizontal detection threads
        for (int t = 0; t < numThreads; ++t) {
            horizontalThreads[t]->join();
            delete horizontalThreads[t];
        }
        delete[] horizontalThreads;

        // 9. Process horizontal lines
        std::pair<int, int>* horizontalLines = new std::pair<int, int>[image.rows];  // Maximum possible horizontal lines
        int horizontalLineCount = 0;
        int startY = -1;

        for (int y = 0; y < image.rows; ++y) {
            if (isBlackRow[y]) {
                if (startY == -1) startY = y;
            } else if (startY != -1) {
                int width = y - startY;
                if (width >= MIN_LINE_WIDTH) {
                    horizontalLines[horizontalLineCount++] = std::make_pair(startY, width);
                }
                startY = -1;
            }
        }

        // Check for line extending to edge
        if (startY != -1) {
            int width = image.rows - startY;
            if (width >= MIN_LINE_WIDTH) {
                horizontalLines[horizontalLineCount++] = std::make_pair(startY, width);
            }
        }

        // 10. Create result array
        const int totalLines = verticalLineCount + horizontalLineCount;
        std::cout << "Creating result array with " << totalLines << " lines" << std::endl;
        DarkLineArray* result = createDarkLineArray(totalLines, 1);
        if (!result) {
            throw std::runtime_error("Failed to create result array");
        }

        // 11. Fill the result array
        int currentIndex = 0;

        // Add vertical lines
        for (int i = 0; i < verticalLineCount; ++i) {
            const auto& [x, width] = verticalLines[i];
            DarkLine& line = result->lines[currentIndex][0];
            line.x = x;
            line.y = 0;
            line.startY = 0;
            line.endY = image.rows - 1;
            line.width = width;
            line.isVertical = true;
            line.inObject = isInObject(image, x, width, true, currentIndex);
            currentIndex++;
        }

        // Add horizontal lines
        for (int i = 0; i < horizontalLineCount; ++i) {
            const auto& [y, width] = horizontalLines[i];
            DarkLine& line = result->lines[currentIndex][0];
            line.x = 0;
            line.y = y;
            line.startX = 0;
            line.endX = image.cols - 1;
            line.width = width;
            line.isVertical = false;
            line.inObject = isInObject(image, y, width, false, currentIndex);
            currentIndex++;
        }

        // 12. Clean up temporary arrays
        delete[] isBlackColumn;
        delete[] isBlackRow;
        delete[] verticalLines;
        delete[] horizontalLines;

        std::cout << "Dark line detection completed. Found " << result->rows << " lines." << std::endl;
        return result;

    } catch (const std::exception& e) {
        std::cerr << "Error in detectDarkLines: " << e.what() << std::endl;
        return nullptr;
    }
}

double DarkLinePointerProcessor::findReplacementValue(
    const ImageData& image,
    int x, int y,
    bool isVertical,
    int lineWidth) {

    int searchRadius = calculateSearchRadius(lineWidth);
    double* validValues = new double[searchRadius * 2]();
    int validCount = 0;

    if (isVertical) {
        for (int offset = 1; offset <= searchRadius; ++offset) {
            int leftX = x - offset;
            if (leftX >= 0 && image.data[y][leftX] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[y][leftX];
            }

            int rightX = x + offset;
            if (rightX < image.cols && image.data[y][rightX] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[y][rightX];
            }
        }
    } else {
        for (int offset = 1; offset <= searchRadius; ++offset) {
            int upY = y - offset;
            if (upY >= 0 && image.data[upY][x] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[upY][x];
            }

            int downY = y + offset;
            if (downY < image.rows && image.data[downY][x] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[downY][x];
            }
        }
    }

    double result;
    if (validCount > 0) {
        // Sort the valid values using bubble sort
        for (int i = 0; i < validCount - 1; i++) {
            for (int j = 0; j < validCount - i - 1; j++) {
                if (validValues[j] > validValues[j + 1]) {
                    double temp = validValues[j];
                    validValues[j] = validValues[j + 1];
                    validValues[j + 1] = temp;
                }
            }
        }
        result = validValues[validCount / 2]; // Median value
    } else {
        result = image.data[y][x]; // Keep original if no valid replacements found
    }

    delete[] validValues;
    return result;
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

    // 1. 参数验证和初始化日志
    std::cout << "\n=== Starting Sequential Line Removal Process ===" << std::endl;

    // 1.1 验证图像数据
    if (!image.data || image.rows <= 0 || image.cols <= 0) {
        std::ostringstream error;
        error << "Invalid image parameters:"
              << "\n  - Data pointer: " << (image.data ? "valid" : "null")
              << "\n  - Rows: " << image.rows
              << "\n  - Cols: " << image.cols;
        throw std::invalid_argument(error.str());
    }

    // 1.2 验证线条数组
    if (!lines || !selectedLines || selectedCount <= 0) {
        std::ostringstream error;
        error << "Invalid line parameters:"
              << "\n  - Lines array: " << (lines ? "valid" : "null")
              << "\n  - Selected lines: " << (selectedLines ? "valid" : "null")
              << "\n  - Selected count: " << selectedCount;
        throw std::invalid_argument(error.str());
    }

    // 1.3 创建临时数组
    DarkLine** validLines = new DarkLine*[selectedCount];
    int validLinesCount = 0;

    // 为每行分配列内存
    for (int i = 0; i < selectedCount; i++) {
        validLines[i] = new DarkLine[1];  // 每行只需要一列
    }

    // 1.4 打印初始配置信息
    std::cout << "Initial Configuration:" << std::endl
              << "  - Image dimensions: " << image.rows << "x" << image.cols << std::endl
              << "  - Selected lines count: " << selectedCount << std::endl
              << "  - Remove in-object lines: " << (removeInObject ? "yes" : "no") << std::endl
              << "  - Remove isolated lines: " << (removeIsolated ? "yes" : "no") << std::endl
              << "  - Method: " << (method == RemovalMethod::DIRECT_STITCH ? "Direct Stitch" : "Neighbor Values")
              << std::endl;

    try {
        // 2. 验证和收集要处理的线条
        std::cout << "\nValidating selected lines..." << std::endl;
        int skippedLines = 0;
        int validatedLines = 0;

        // 2.1 验证每条线
        for (int i = 0; i < selectedCount; i++) {
            if (!selectedLines[i]) {
                std::cout << "  Warning: Null line pointer at index " << i << std::endl;
                skippedLines++;
                continue;
            }

            bool isValid = true;
            std::ostringstream validationDetails;
            validationDetails << "  Line " << i << " validation:";

            // 2.2 验证线条坐标
            if (selectedLines[i]->isVertical) {
                validationDetails << " [Vertical]"
                                  << " x=" << selectedLines[i]->x
                                  << " startY=" << selectedLines[i]->startY
                                  << " endY=" << selectedLines[i]->endY
                                  << " width=" << selectedLines[i]->width;

                if (selectedLines[i]->x < 0 || selectedLines[i]->x >= image.cols) {
                    validationDetails << " - Invalid x coordinate";
                    isValid = false;
                }
                if (selectedLines[i]->startY < 0 || selectedLines[i]->endY >= image.rows) {
                    validationDetails << " - Invalid Y range";
                    isValid = false;
                }
                if (selectedLines[i]->width <= 0 ||
                    selectedLines[i]->x + selectedLines[i]->width > image.cols) {
                    validationDetails << " - Invalid width";
                    isValid = false;
                }
            } else {
                validationDetails << " [Horizontal]"
                                  << " y=" << selectedLines[i]->y
                                  << " startX=" << selectedLines[i]->startX
                                  << " endX=" << selectedLines[i]->endX
                                  << " width=" << selectedLines[i]->width;

                if (selectedLines[i]->y < 0 || selectedLines[i]->y >= image.rows) {
                    validationDetails << " - Invalid y coordinate";
                    isValid = false;
                }
                if (selectedLines[i]->startX < 0 || selectedLines[i]->endX >= image.cols) {
                    validationDetails << " - Invalid X range";
                    isValid = false;
                }
                if (selectedLines[i]->width <= 0 ||
                    selectedLines[i]->y + selectedLines[i]->width > image.rows) {
                    validationDetails << " - Invalid width";
                    isValid = false;
                }
            }

            validationDetails << (isValid ? " - Valid" : " - Invalid");
            std::cout << validationDetails.str() << std::endl;

            if (!isValid) {
                skippedLines++;
                continue;
            }

            // 2.3 检查是否符合移除条件
            bool shouldProcess = (selectedLines[i]->inObject && removeInObject) ||
                                 (!selectedLines[i]->inObject && removeIsolated);

            if (shouldProcess) {
                validLines[validatedLines][0] = *selectedLines[i];
                validatedLines++;
            }
        }

        // 2.4 打印验证总结
        std::cout << "\nValidation Summary:" << std::endl
                  << "  - Total lines processed: " << selectedCount << std::endl
                  << "  - Valid lines for removal: " << validatedLines << std::endl
                  << "  - Skipped lines: " << skippedLines << std::endl;

        // 2.5 检查是否有有效的线条要处理
        if (validatedLines == 0) {
            // 清理内存
            for (int i = 0; i < selectedCount; i++) {
                delete[] validLines[i];
            }
            delete[] validLines;

            std::cout << "\nNo valid lines to process - operation complete" << std::endl;
            return;
        }
        // 3. 对有效线条进行排序和准备处理
        std::cout << "\nPreparing for line processing..." << std::endl;

        // 3.1 创建用于排序的临时数组
        DarkLine** sortedLines = new DarkLine*[validatedLines];
        for (int i = 0; i < validatedLines; i++) {
            sortedLines[i] = new DarkLine[1];
            sortedLines[i][0] = validLines[i][0];
        }

        // 3.2 根据位置进行排序
        for (int i = 0; i < validatedLines - 1; i++) {
            for (int j = 0; j < validatedLines - i - 1; j++) {
                DarkLine& a = sortedLines[j][0];
                DarkLine& b = sortedLines[j + 1][0];
                bool shouldSwap = false;

                if (a.isVertical == b.isVertical) {
                    shouldSwap = a.isVertical ? (a.x > b.x) : (a.y > b.y);
                } else {
                    shouldSwap = !a.isVertical;  // 垂直线优先
                }

                if (shouldSwap) {
                    // 交换两行
                    DarkLine temp = sortedLines[j][0];
                    sortedLines[j][0] = sortedLines[j + 1][0];
                    sortedLines[j + 1][0] = temp;
                }
            }
        }

        // 3.3 创建移除区段数组
        std::pair<int, int>** verticalSections = new std::pair<int, int>*[validatedLines];
        std::pair<int, int>** horizontalSections = new std::pair<int, int>*[validatedLines];
        int verticalCount = 0;
        int horizontalCount = 0;

        // 为每个数组分配列内存
        for (int i = 0; i < validatedLines; i++) {
            verticalSections[i] = new std::pair<int, int>[1];
            horizontalSections[i] = new std::pair<int, int>[1];
        }

        // 3.4 收集移除区段
        for (int i = 0; i < validatedLines; i++) {
            const DarkLine& line = sortedLines[i][0];
            if (line.isVertical) {
                verticalSections[verticalCount][0] = std::make_pair(line.x, line.width);
                verticalCount++;
            } else {
                horizontalSections[horizontalCount][0] = std::make_pair(line.y, line.width);
                horizontalCount++;
            }
        }

        // 3.5 计算新维度
        int newWidth = image.cols;
        int newHeight = image.rows;

        for (int i = 0; i < verticalCount; i++) {
            newWidth -= verticalSections[i][0].second;
        }
        for (int i = 0; i < horizontalCount; i++) {
            newHeight -= horizontalSections[i][0].second;
        }

        // 3.6 验证新维度
        if (newWidth <= 0 || newHeight <= 0) {
            std::ostringstream error;
            error << "Invalid resulting dimensions: "
                  << newWidth << "x" << newHeight;
            throw std::runtime_error(error.str());
        }

        std::cout << "New dimensions will be: " << newWidth << "x" << newHeight
                  << " (original: " << image.cols << "x" << image.rows << ")" << std::endl;
        // 4. 执行线条移除
        if (method == RemovalMethod::DIRECT_STITCH) {
            // 4.1 创建新的图像缓冲区
            std::cout << "\nAllocating new image buffer..." << std::endl;
            double** newImageData = new double*[newHeight];
            for (int i = 0; i < newHeight; i++) {
                newImageData[i] = new double[newWidth]();  // 零初始化
            }

            // 4.2 复制数据并移除线条
            std::cout << "\nCopying image data and removing lines..." << std::endl;
            int destY = 0;
            for (int srcY = 0; srcY < image.rows && destY < newHeight; srcY++) {
                // 检查是否应该跳过这一行
                bool skipRow = false;
                for (int i = 0; i < horizontalCount; i++) {
                    if (srcY >= horizontalSections[i][0].first &&
                        srcY < horizontalSections[i][0].first + horizontalSections[i][0].second) {
                        skipRow = true;
                        break;
                    }
                }

                if (!skipRow) {
                    int destX = 0;
                    for (int srcX = 0; srcX < image.cols && destX < newWidth; srcX++) {
                        // 检查是否应该跳过这一列
                        bool skipCol = false;
                        for (int i = 0; i < verticalCount; i++) {
                            if (srcX >= verticalSections[i][0].first &&
                                srcX < verticalSections[i][0].first + verticalSections[i][0].second) {
                                skipCol = true;
                                break;
                            }
                        }

                        if (!skipCol) {
                            newImageData[destY][destX++] = image.data[srcY][srcX];
                        }
                    }
                    destY++;
                }
            }

            // 4.3 更新原始图像
            for (int i = 0; i < image.rows; i++) {
                delete[] image.data[i];
            }
            delete[] image.data;

            image.data = newImageData;
            image.rows = newHeight;
            image.cols = newWidth;

        } else {  // NEIGHBOR_VALUES method
            std::cout << "\nProcessing with neighbor values method..." << std::endl;

            // 4.4 创建临时图像
            double** tempData = new double*[image.rows];
            for (int i = 0; i < image.rows; i++) {
                tempData[i] = new double[image.cols];
                std::copy(image.data[i], image.data[i] + image.cols, tempData[i]);
            }

            // 4.5 处理每条线
            for (int i = 0; i < validatedLines; i++) {
                const DarkLine& line = sortedLines[i][0];
                if (line.isVertical) {
                    for (int x = line.x; x < std::min(line.x + line.width, image.cols); ++x) {
                        for (int y = line.startY; y <= line.endY && y < image.rows; ++y) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempData[y][x] = findReplacementValue(image, x, y, true, line.width);
                            }
                        }
                    }
                } else {
                    for (int y = line.y; y < std::min(line.y + line.width, image.rows); ++y) {
                        for (int x = line.startX; x <= line.endX && x < image.cols; ++x) {
                            if (image.data[y][x] <= MIN_BRIGHTNESS) {
                                tempData[y][x] = findReplacementValue(image, x, y, false, line.width);
                            }
                        }
                    }
                }
            }

            // 4.6 更新原始图像
            for (int i = 0; i < image.rows; i++) {
                std::copy(tempData[i], tempData[i] + image.cols, image.data[i]);
                delete[] tempData[i];
            }
            delete[] tempData;
        }

        // 5. 清理内存
        for (int i = 0; i < selectedCount; i++) {
            delete[] validLines[i];
        }
        delete[] validLines;

        for (int i = 0; i < validatedLines; i++) {
            delete[] sortedLines[i];
        }
        delete[] sortedLines;

        for (int i = 0; i < validatedLines; i++) {
            delete[] verticalSections[i];
            delete[] horizontalSections[i];
        }
        delete[] verticalSections;
        delete[] horizontalSections;

        std::cout << "\n=== Sequential line removal completed successfully ===" << std::endl;
        std::cout << "Final image dimensions: " << image.rows << "x" << image.cols << std::endl;

    } catch (const std::exception& e) {
        // 6. 异常处理时的清理
        for (int i = 0; i < selectedCount; i++) {
            delete[] validLines[i];
        }
        delete[] validLines;

        // 重新抛出异常
        throw;
    }
}


void DarkLinePointerProcessor::validateNewDimensions(
    const ImageData& image,
    const std::pair<DarkLine, int>** lines,
    int linesCount,
    bool isVertical,
    int& newSize) {

    int totalReduction = 0;
    for (int i = 0; i < linesCount; i++) {
        for (int j = 0; j < 1; j++) {  // 假设每行只有一列
            if (lines[i][j].first.isVertical == isVertical) {
                totalReduction += lines[i][j].first.width;
            }
        }
    }

    newSize = (isVertical ? image.cols : image.rows) - totalReduction;
    if (newSize <= 0) {
        throw std::runtime_error("Invalid dimensions after line removal");
    }
}

bool DarkLinePointerProcessor::prepareImageBuffer(
    const ImageData& image,
    const std::pair<DarkLine, int>** lines,
    int linesCount,
    ImageData& buffer) {

    try {
        // 计算新维度
        int newWidth = image.cols;
        int newHeight = image.rows;

        for (int i = 0; i < linesCount; i++) {
            for (int j = 0; j < 1; j++) {
                const auto& line = lines[i][j];
                if (line.first.isVertical) {
                    newWidth -= line.first.width;
                } else {
                    newHeight -= line.first.width;
                }
            }
        }

        if (newWidth <= 0 || newHeight <= 0) {
            return false;
        }

        // 分配新缓冲区
        buffer.rows = newHeight;
        buffer.cols = newWidth;
        buffer.data = new double*[newHeight];
        for (int i = 0; i < newHeight; i++) {
            buffer.data[i] = new double[newWidth]();  // 使用()进行零初始化
        }

        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error preparing image buffer: " << e.what() << std::endl;
        return false;
    }
}

void DarkLinePointerProcessor::adjustLineCoordinates(
    DarkLine& line,
    const std::pair<int, int>** verticalSections,
    int verticalCount,
    const std::pair<int, int>** horizontalSections,
    int horizontalCount,
    int newWidth,
    int newHeight) {

    if (line.isVertical) {
        int xAdjustment = 0;
        for (int i = 0; i < verticalCount; i++) {
            for (int j = 0; j < 1; j++) {
                const auto& section = verticalSections[i][j];
                if (line.x > section.first) {
                    xAdjustment += section.second;
                }
            }
        }
        line.x -= xAdjustment;
        line.endY = std::min(line.endY, newHeight - 1);
        line.startY = std::max(line.startY, 0);
        if (line.x + line.width > newWidth) {
            line.width = newWidth - line.x;
        }
    } else {
        int yAdjustment = 0;
        for (int i = 0; i < horizontalCount; i++) {
            for (int j = 0; j < 1; j++) {
                const auto& section = horizontalSections[i][j];
                if (line.y > section.first) {
                    yAdjustment += section.second;
                }
            }
        }
        line.y -= yAdjustment;
        line.endX = std::min(line.endX, newWidth - 1);
        line.startX = std::max(line.startX, 0);
        if (line.y + line.width > newHeight) {
            line.width = newHeight - line.y;
        }
    }
}
DarkLineArray* DarkLinePointerProcessor::createSafeDarkLineArray(int rows, int cols) {
    std::cout << "Creating safe dark line array with dimensions: " << rows << "x" << cols << std::endl;

    try {
        auto newArray = new DarkLineArray();

        if (rows <= 0 || cols <= 0) {
            std::cout << "Creating empty array" << std::endl;
            newArray->rows = 0;
            newArray->cols = 0;
            newArray->lines = nullptr;
            return newArray;
        }

        newArray->rows = rows;
        newArray->cols = cols;
        newArray->lines = new DarkLine*[rows];

        if (!newArray->lines) {
            throw std::bad_alloc();
        }

        std::memset(newArray->lines, 0, rows * sizeof(DarkLine*));

        for (int i = 0; i < rows; i++) {
            newArray->lines[i] = new DarkLine[cols];
            if (!newArray->lines[i]) {
                for (int j = 0; j < i; j++) {
                    delete[] newArray->lines[j];
                }
                delete[] newArray->lines;
                delete newArray;
                throw std::bad_alloc();
            }
            for (int j = 0; j < cols; j++) {
                newArray->lines[i][j] = DarkLine();
            }
        }

        return newArray;

    } catch (const std::exception& e) {
        std::cerr << "Error in createSafeDarkLineArray: " << e.what() << std::endl;
        throw;
    }
}
