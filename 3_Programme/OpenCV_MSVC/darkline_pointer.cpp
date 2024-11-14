#include "darkline_pointer.h"
#include <QDebug>
#include <iostream>
#include <algorithm>

// Array management helpers
void DarkLinePointerProcessor::addDarkLine(DarkLinePtrArray* array, const DarkLinePtr& line) {
    if (array->count >= array->capacity) {
        expandArray(array);
    }
    array->lines[array->count++] = line;
}

void DarkLinePointerProcessor::expandArray(DarkLinePtrArray* array) {
    int newCapacity = array->capacity == 0 ? INITIAL_CAPACITY : array->capacity * 2;
    DarkLinePtr* newLines = new DarkLinePtr[newCapacity];

    // Copy existing lines
    if (array->lines != nullptr) {
        for (int i = 0; i < array->count; i++) {
            newLines[i] = array->lines[i];
        }
        delete[] array->lines;
    }

    array->lines = newLines;
    array->capacity = newCapacity;
}

bool DarkLinePointerProcessor::isInObject(const DarkLineImageData& image, int pos, int lineWidth, bool isVertical, int threadId) {
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
                for (int x = 0; x < image.cols; x += image.cols/10) {
                    double topPixel = image.data[pos - i][x];
                    if (topPixel < WHITE_THRESHOLD) {
                        qDebug() << "Thread" << threadId << "- Found non-white pixel above line at x=" << x;
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
                        qDebug() << "Thread" << threadId << "- Found non-white pixel below line at x=" << x;
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

int DarkLinePointerProcessor::calculateSearchRadius(int lineWidth) {
    // Base the search radius on the line width
    // Minimum radius of 10 pixels, maximum of 200 pixels
    // Use a multiplier of 2 for the line width
    const int minRadius = 10;
    const int maxRadius = 200;
    int radius = lineWidth * 2;

    // Ensure the radius stays within reasonable bounds
    return std::clamp(radius, minRadius, maxRadius);
}

double DarkLinePointerProcessor::findReplacementValue(
    const DarkLineImageData& image,
    int x, int y,
    bool isVertical,
    int lineWidth) {

    // Calculate dynamic search radius based on line width
    int searchRadius = calculateSearchRadius(lineWidth);
    double* validValues = new double[searchRadius * 2];
    int validCount = 0;

    if (isVertical) {
        // Look left and right
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Check left
            int leftX = x - offset;
            if (leftX >= 0 && image.data[y][leftX] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[y][leftX];
            }

            // Check right
            int rightX = x + offset;
            if (rightX < image.cols && image.data[y][rightX] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[y][rightX];
            }
        }
    } else {
        // Look up and down
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Check up
            int upY = y - offset;
            if (upY >= 0 && image.data[upY][x] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[upY][x];
            }

            // Check down
            int downY = y + offset;
            if (downY < image.rows && image.data[downY][x] > MIN_BRIGHTNESS) {
                validValues[validCount++] = image.data[downY][x];
            }
        }
    }

    double result;
    if (validCount == 0) {
        result = image.data[y][x];  // Keep original if no valid replacements found
    } else {
        std::sort(validValues, validValues + validCount);
        result = validValues[validCount / 2];  // Use median value
    }

    delete[] validValues;
    return result;
}

double DarkLinePointerProcessor::findStitchValue(
    const DarkLineImageData& image,
    const DarkLinePtr& line,
    int x, int y) {

    if (line.isVertical) {
        // For vertical lines, find values from both sides
        int leftX = line.x - 1;
        int rightX = line.x + line.width;

        if (leftX >= 0 && rightX < image.cols) {
            // If we're closer to the left edge, use left value
            if (x - line.x < line.width / 2) {
                return image.data[y][leftX];
            } else {
                return image.data[y][rightX];
            }
        } else if (leftX >= 0) {
            return image.data[y][leftX];
        } else if (rightX < image.cols) {
            return image.data[y][rightX];
        }
    } else {
        // For horizontal lines, find values from top and bottom
        int topY = line.y - 1;
        int bottomY = line.y + line.width;

        if (topY >= 0 && bottomY < image.rows) {
            // If we're closer to the top, use top value
            if (y - line.y < line.width / 2) {
                return image.data[topY][x];
            } else {
                return image.data[bottomY][x];
            }
        } else if (topY >= 0) {
            return image.data[topY][x];
        } else if (bottomY < image.rows) {
            return image.data[bottomY][x];
        }
    }

    return image.data[y][x]; // Return original value if no stitch is possible
}

DarkLinePtrArray* DarkLinePointerProcessor::detectDarkLines(const DarkLineImageData& image) {
    if (!image.data) {
        return new DarkLinePtrArray();  // Return empty array if no data
    }

    DarkLinePtrArray* detectedLines = new DarkLinePtrArray();
    const int numThreads = std::thread::hardware_concurrency();
    std::mutex detectedLinesMutex;

    // Detect vertical lines using multiple threads
    bool* isBlackColumn = new bool[image.cols]();  // Initialize all to false
    std::thread* verticalThreads = new std::thread[numThreads];
    int columnsPerThread = image.cols / numThreads;

    std::cout << "Starting vertical line detection with " << numThreads << " threads" << std::endl;

    for (int t = 0; t < numThreads; ++t) {
        int startX = t * columnsPerThread;
        int endX = (t == numThreads - 1) ? image.cols : startX + columnsPerThread;

        verticalThreads[t] = std::thread([=, &isBlackColumn, &image]() {
            std::cout << "Thread " << t << " processing columns " << startX << " to " << endX << std::endl;
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
            std::cout << "Thread " << t << " completed vertical line detection" << std::endl;
        });
    }

    // Wait for all vertical threads to complete
    for (int t = 0; t < numThreads; ++t) {
        verticalThreads[t].join();
    }
    delete[] verticalThreads;

    // Form vertical lines
    int startX = -1;
    for (int x = 0; x < image.cols; ++x) {
        if (isBlackColumn[x]) {
            if (startX == -1) startX = x;
        } else if (startX != -1) {
            DarkLinePtr line;
            line.x = startX;
            line.startY = 0;
            line.endY = image.rows - 1;
            line.width = x - startX;
            line.isVertical = true;
            line.inObject = isInObject(image, startX, line.width, true, 0);

            if (line.width >= MIN_LINE_WIDTH) {
                std::lock_guard<std::mutex> lock(detectedLinesMutex);
                addDarkLine(detectedLines, line);
            }
            startX = -1;
        }
    }
    // Check for line that extends to the edge
    if (startX != -1) {
        DarkLinePtr line;
        line.x = startX;
        line.startY = 0;
        line.endY = image.rows - 1;
        line.width = image.cols - startX;
        line.isVertical = true;
        line.inObject = isInObject(image, startX, line.width, true, 0);

        if (line.width >= MIN_LINE_WIDTH) {
            std::lock_guard<std::mutex> lock(detectedLinesMutex);
            addDarkLine(detectedLines, line);
        }
    }

    delete[] isBlackColumn;

    // Detect horizontal lines using multiple threads
    bool* isBlackRow = new bool[image.rows]();  // Initialize all to false
    std::thread* horizontalThreads = new std::thread[numThreads];
    int rowsPerThread = image.rows / numThreads;

    std::cout << "Starting horizontal line detection with " << numThreads << " threads" << std::endl;

    for (int t = 0; t < numThreads; ++t) {
        int startY = t * rowsPerThread;
        int endY = (t == numThreads - 1) ? image.rows : startY + rowsPerThread;

        horizontalThreads[t] = std::thread([=, &isBlackRow, &image]() {
            std::cout << "Thread " << t << " processing rows " << startY << " to " << endY << std::endl;
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
            std::cout << "Thread " << t << " completed horizontal line detection" << std::endl;
        });
    }

    // Wait for all horizontal threads to complete
    for (int t = 0; t < numThreads; ++t) {
        horizontalThreads[t].join();
    }
    delete[] horizontalThreads;

    // Form horizontal lines
    int startY = -1;
    for (int y = 0; y < image.rows; ++y) {
        if (isBlackRow[y]) {
            if (startY == -1) startY = y;
        } else if (startY != -1) {
            DarkLinePtr line;
            line.y = startY;
            line.startX = 0;
            line.endX = image.cols - 1;
            line.width = y - startY;
            line.isVertical = false;
            line.inObject = isInObject(image, startY, line.width, false, 0);

            if (line.width >= MIN_LINE_WIDTH) {
                std::lock_guard<std::mutex> lock(detectedLinesMutex);
                addDarkLine(detectedLines, line);
            }
            startY = -1;
        }
    }
    // Check for line that extends to the edge
    if (startY != -1) {
        DarkLinePtr line;
        line.y = startY;
        line.startX = 0;
        line.endX = image.cols - 1;
        line.width = image.rows - startY;
        line.isVertical = false;
        line.inObject = isInObject(image, startY, line.width, false, 0);

        if (line.width >= MIN_LINE_WIDTH) {
            std::lock_guard<std::mutex> lock(detectedLinesMutex);
            addDarkLine(detectedLines, line);
        }
    }

    delete[] isBlackRow;

    std::cout << "Dark line detection completed. Found " << detectedLines->count << " lines." << std::endl;
    return detectedLines;
}

void DarkLinePointerProcessor::removeDarkLines(DarkLineImageData& image, const DarkLinePtrArray* lines) {
    if (!lines || lines->count == 0 || !image.data) return;

    // Create new image data
    double** newData = new double*[image.rows];
    for (int i = 0; i < image.rows; ++i) {
        newData[i] = new double[image.cols];
        // Copy original data
        for (int j = 0; j < image.cols; ++j) {
            newData[i][j] = image.data[i][j];
        }
    }

    // Process each line with its exact detected width
    for (int i = 0; i < lines->count; ++i) {
        const DarkLinePtr& line = lines->lines[i];
        // Skip lines that are part of objects
        if (line.inObject) {
            continue;
        }

        if (line.isVertical) {
            // Process vertical line
            for (int x = line.x; x < line.x + line.width; ++x) {
                for (int y = 0; y < image.rows; ++y) {
                    if (image.data[y][x] <= MIN_BRIGHTNESS) {
                        newData[y][x] = findReplacementValue(image, x, y, true, line.width);
                    }
                }
            }
        } else {
            // Process horizontal line
            for (int y = line.y; y < line.y + line.width; ++y) {
                for (int x = 0; x < image.cols; ++x) {
                    if (image.data[y][x] <= MIN_BRIGHTNESS) {
                        newData[y][x] = findReplacementValue(image, x, y, false, line.width);
                    }
                }
            }
        }
    }

    // Delete old data
    for (int i = 0; i < image.rows; ++i) {
        delete[] image.data[i];
    }
    delete[] image.data;

    // Update image with new data
    image.data = newData;
}

void DarkLinePointerProcessor::removeDarkLinesSelective(
    DarkLineImageData& image,
    const DarkLinePtrArray* lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (!lines || lines->count == 0 || !image.data) return;

    // Create new image data
    double** newData = new double*[image.rows];
    for (int i = 0; i < image.rows; ++i) {
        newData[i] = new double[image.cols];
        for (int j = 0; j < image.cols; ++j) {
            newData[i][j] = image.data[i][j];
        }
    }

    // Count total width/height to remove for DIRECT_STITCH method
    int totalVerticalWidth = 0;
    int totalHorizontalHeight = 0;

    if (method == RemovalMethod::DIRECT_STITCH) {
        for (int i = 0; i < lines->count; ++i) {
            const DarkLinePtr& line = lines->lines[i];
            if ((line.inObject && removeInObject) || (!line.inObject && removeIsolated)) {
                if (line.isVertical) {
                    totalVerticalWidth += line.width;
                } else {
                    totalHorizontalHeight += line.width;
                }
            }
        }
    }

    // Process each line
    for (int i = 0; i < lines->count; ++i) {
        const DarkLinePtr& line = lines->lines[i];
        // Skip lines based on selection criteria
        if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
            continue;
        }

        if (method == RemovalMethod::DIRECT_STITCH) {
            if (line.isVertical) {
                // For vertical lines, shift all content after the line to the left
                for (int y = 0; y < image.rows; ++y) {
                    for (int x = line.x; x < image.cols - line.width; ++x) {
                        newData[y][x] = image.data[y][x + line.width];
                    }
                }
            } else {
                // For horizontal lines, shift all content after the line upward
                for (int y = line.y; y < image.rows - line.width; ++y) {
                    for (int x = 0; x < image.cols; ++x) {
                        newData[y][x] = image.data[y + line.width][x];
                    }
                }
            }
        } else {
            // Use neighbor values method
            if (line.isVertical) {
                for (int x = line.x; x < line.x + line.width; ++x) {
                    for (int y = 0; y < image.rows; ++y) {
                        if (image.data[y][x] <= MIN_BRIGHTNESS) {
                            newData[y][x] = findReplacementValue(image, x, y, true, line.width);
                        }
                    }
                }
            } else {
                for (int y = line.y; y < line.y + line.width; ++y) {
                    for (int x = 0; x < image.cols; ++x) {
                        if (image.data[y][x] <= MIN_BRIGHTNESS) {
                            newData[y][x] = findReplacementValue(image, x, y, false, line.width);
                        }
                    }
                }
            }
        }
    }

    // Resize image if using DIRECT_STITCH method
    if (method == RemovalMethod::DIRECT_STITCH &&
        (totalVerticalWidth > 0 || totalHorizontalHeight > 0)) {

        int newWidth = image.cols - totalVerticalWidth;
        int newHeight = image.rows - totalHorizontalHeight;

        // Create resized data array
        double** resizedData = new double*[newHeight];
        for (int i = 0; i < newHeight; ++i) {
            resizedData[i] = new double[newWidth];
            for (int j = 0; j < newWidth; ++j) {
                resizedData[i][j] = newData[i][j];
            }
        }

        // Delete intermediate new data
        for (int i = 0; i < image.rows; ++i) {
            delete[] newData[i];
        }
        delete[] newData;

        // Update image dimensions and data
        newData = resizedData;
        image.cols = newWidth;
        image.rows = newHeight;
    }

    // Delete old data and update image
    for (int i = 0; i < image.rows; ++i) {
        delete[] image.data[i];
    }
    delete[] image.data;

    image.data = newData;
}
