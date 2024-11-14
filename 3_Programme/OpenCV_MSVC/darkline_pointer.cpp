#include "darkline_pointer.h"
#include <QDebug>
#include <algorithm>
#include <iostream>

bool DarkLinePointerProcessor::isInObject(const ImageData& image, int pos, int lineWidth, bool isVertical, int threadId) {
    if (isVertical) {
        std::cout << "Thread " << threadId << " checking vertical line at x=" << pos << std::endl;
        for (int y = 0; y < image.rows; ++y) {
            // Check left side
            for (int i = 1; i <= VERTICAL_CHECK_RANGE; ++i) {
                if (pos - i >= 0) {
                    uint16_t leftPixel = image.data[y][pos - i];
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
                    uint16_t rightPixel = image.data[y][pos + lineWidth + i - 1];
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
        qDebug() << "\nThread" << threadId << "- Analyzing horizontal line at y=" << pos
                 << " with width=" << lineWidth;

        // Sample pixels above the line
        for (int i = 1; i <= HORIZONTAL_CHECK_RANGE; ++i) {
            if (pos - i >= 0) {
                for (int x = 0; x < image.cols; x += image.cols/10) {
                    uint16_t topPixel = image.data[pos - i][x];
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
                    uint16_t bottomPixel = image.data[pos + lineWidth + i - 1][x];
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

uint16_t DarkLinePointerProcessor::findReplacementValue(
    const ImageData& image,
    int x, int y,
    bool isVertical,
    int lineWidth) {

    std::vector<uint16_t> validValues;
    int searchRadius = calculateSearchRadius(lineWidth);
    validValues.reserve(searchRadius * 2);

    if (isVertical) {
        // Look left and right
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Check left
            int leftX = x - offset;
            if (leftX >= 0 && image.data[y][leftX] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[y][leftX]);
            }

            // Check right
            int rightX = x + offset;
            if (rightX < image.cols && image.data[y][rightX] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[y][rightX]);
            }
        }
    } else {
        // Look up and down
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Check up
            int upY = y - offset;
            if (upY >= 0 && image.data[upY][x] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[upY][x]);
            }

            // Check down
            int downY = y + offset;
            if (downY < image.rows && image.data[downY][x] > MIN_BRIGHTNESS) {
                validValues.push_back(image.data[downY][x]);
            }
        }
    }

    if (validValues.empty()) {
        return image.data[y][x];  // Keep original if no valid replacements found
    }

    // Use median value for replacement
    std::sort(validValues.begin(), validValues.end());
    return validValues[validValues.size() / 2];
}

int DarkLinePointerProcessor::calculateSearchRadius(int lineWidth) {
    const int minRadius = 10;
    const int maxRadius = 200;
    int radius = lineWidth * 2;
    return std::clamp(radius, minRadius, maxRadius);
}

std::vector<DarkLinePointerProcessor::DarkLine> DarkLinePointerProcessor::detectDarkLines(
    const ImageData& image) {

    std::vector<DarkLine> detectedLines;
    if (!image.data || image.rows == 0 || image.cols == 0) return detectedLines;

    const int numThreads = std::thread::hardware_concurrency();
    std::mutex detectedLinesMutex;

    // Detect vertical lines using multiple threads
    std::vector<bool> isBlackColumn(image.cols, false);
    std::vector<std::thread> verticalThreads;
    int columnsPerThread = image.cols / numThreads;

    std::cout << "Starting vertical line detection with " << numThreads << " threads" << std::endl;

    for (int t = 0; t < numThreads; ++t) {
        int startX = t * columnsPerThread;
        int endX = (t == numThreads - 1) ? image.cols : startX + columnsPerThread;

        verticalThreads.emplace_back([=, &isBlackColumn, &image]() {
            std::cout << "Thread " << t << " processing columns " << startX << " to " << endX << std::endl;
            for (int x = startX; x < endX; ++x) {
                int blackPixelCount = 0;
                for (int y = 0; y < image.rows; ++y) {
                    if (image.data[y][x] <= BLACK_THRESHOLD) {
                        blackPixelCount++;
                    }
                }
                float blackRatio = static_cast<float>(blackPixelCount) / image.rows;
                if (blackRatio > (1.0f - NOISE_TOLERANCE)) {
                    isBlackColumn[x] = true;
                }
            }
            std::cout << "Thread " << t << " completed vertical line detection" << std::endl;
        });
    }

    for (auto& thread : verticalThreads) {
        thread.join();
    }

    // Form vertical lines
    int startX = -1;
    for (int x = 0; x < image.cols; ++x) {
        if (isBlackColumn[x]) {
            if (startX == -1) startX = x;
        } else if (startX != -1) {
            DarkLine line;
            line.x = startX;
            line.startY = 0;
            line.endY = image.rows - 1;
            line.width = x - startX;
            line.isVertical = true;
            line.inObject = isInObject(image, startX, line.width, true, 0);

            if (line.width >= MIN_LINE_WIDTH) {
                std::lock_guard<std::mutex> lock(detectedLinesMutex);
                detectedLines.push_back(line);
            }
            startX = -1;
        }
    }
    if (startX != -1) {
        DarkLine line;
        line.x = startX;
        line.startY = 0;
        line.endY = image.rows - 1;
        line.width = image.cols - startX;
        line.isVertical = true;
        line.inObject = isInObject(image, startX, line.width, true, 0);

        if (line.width >= MIN_LINE_WIDTH) {
            std::lock_guard<std::mutex> lock(detectedLinesMutex);
            detectedLines.push_back(line);
        }
    }

    // Detect horizontal lines using multiple threads
    std::vector<bool> isBlackRow(image.rows, false);
    std::vector<std::thread> horizontalThreads;
    int rowsPerThread = image.rows / numThreads;

    std::cout << "Starting horizontal line detection with " << numThreads << " threads" << std::endl;

    for (int t = 0; t < numThreads; ++t) {
        int startY = t * rowsPerThread;
        int endY = (t == numThreads - 1) ? image.rows : startY + rowsPerThread;

        horizontalThreads.emplace_back([=, &isBlackRow, &image]() {
            std::cout << "Thread " << t << " processing rows " << startY << " to " << endY << std::endl;
            for (int y = startY; y < endY; ++y) {
                int blackPixelCount = 0;
                for (int x = 0; x < image.cols; ++x) {
                    if (image.data[y][x] <= BLACK_THRESHOLD) {
                        blackPixelCount++;
                    }
                }
                float blackRatio = static_cast<float>(blackPixelCount) / image.cols;
                if (blackRatio > (1.0f - NOISE_TOLERANCE)) {
                    isBlackRow[y] = true;
                }
            }
            std::cout << "Thread " << t << " completed horizontal line detection" << std::endl;
        });
    }

    for (auto& thread : horizontalThreads) {
        thread.join();
    }

    // Form horizontal lines
    int startY = -1;
    for (int y = 0; y < image.rows; ++y) {
        if (isBlackRow[y]) {
            if (startY == -1) startY = y;
        } else if (startY != -1) {
            DarkLine line;
            line.y = startY;
            line.startX = 0;
            line.endX = image.cols - 1;
            line.width = y - startY;
            line.isVertical = false;
            line.inObject = isInObject(image, startY, line.width, false, 0);

            if (line.width >= MIN_LINE_WIDTH) {
                std::lock_guard<std::mutex> lock(detectedLinesMutex);
                detectedLines.push_back(line);
            }
            startY = -1;
        }
    }
    if (startY != -1) {
        DarkLine line;
        line.y = startY;
        line.startX = 0;
        line.endX = image.cols - 1;
        line.width = image.rows - startY;
        line.isVertical = false;
        line.inObject = isInObject(image, startY, line.width, false, 0);

        if (line.width >= MIN_LINE_WIDTH) {
            std::lock_guard<std::mutex> lock(detectedLinesMutex);
            detectedLines.push_back(line);
        }
    }

    std::cout << "Dark line detection completed. Found " << detectedLines.size() << " lines." << std::endl;
    return detectedLines;
}

// Helper function for direct stitching
uint16_t DarkLinePointerProcessor::findStitchValue(
    const ImageData& image,
    const DarkLine& line,
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

void DarkLinePointerProcessor::removeDarkLines(ImageData& image, const std::vector<DarkLine>& lines) {
    if (lines.empty()) return;

    // Create a new image array for the modified image
    uint16_t** newData = new uint16_t*[image.rows];
    for (int i = 0; i < image.rows; i++) {
        newData[i] = new uint16_t[image.cols];
        // Copy original data
        std::copy(image.data[i], image.data[i] + image.cols, newData[i]);
    }

    // Process each line with its exact detected width
    for (const auto& line : lines) {
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

    // Free old data
    for (int i = 0; i < image.rows; i++) {
        delete[] image.data[i];
    }
    delete[] image.data;

    // Update image with new data
    image.data = newData;
}

void DarkLinePointerProcessor::removeDarkLinesSequential(
    ImageData& image,
    const std::vector<DarkLine>& lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (lines.empty()) return;

    // Process each line sequentially
    for (const auto& line : lines) {
        // Skip lines based on selection criteria
        if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
            continue;
        }

        if (method == RemovalMethod::DIRECT_STITCH) {
            // Create new array for current iteration
            uint16_t** newData;
            int newRows = image.rows;
            int newCols = image.cols;

            if (line.isVertical) {
                // Calculate new dimensions after removing vertical line
                newCols = image.cols - line.width;
                newData = new uint16_t*[newRows];
                for (int i = 0; i < newRows; i++) {
                    newData[i] = new uint16_t[newCols];
                }

                // Copy data and stitch
                for (int y = 0; y < newRows; ++y) {
                    // Copy data before the line
                    for (int x = 0; x < line.x; ++x) {
                        newData[y][x] = image.data[y][x];
                    }
                    // Copy data after the line with offset
                    for (int x = line.x; x < newCols; ++x) {
                        newData[y][x] = image.data[y][x + line.width];
                    }
                }
            } else {
                // Calculate new dimensions after removing horizontal line
                newRows = image.rows - line.width;
                newData = new uint16_t*[newRows];
                for (int i = 0; i < newRows; i++) {
                    newData[i] = new uint16_t[newCols];
                }

                // Copy data before the line
                for (int y = 0; y < line.y; ++y) {
                    std::copy(image.data[y], image.data[y] + newCols, newData[y]);
                }

                // Copy data after the line with offset
                for (int y = line.y; y < newRows; ++y) {
                    std::copy(image.data[y + line.width],
                              image.data[y + line.width] + newCols,
                              newData[y]);
                }
            }

            // Free old data
            for (int i = 0; i < image.rows; i++) {
                delete[] image.data[i];
            }
            delete[] image.data;

            // Update image dimensions and data
            image.rows = newRows;
            image.cols = newCols;
            image.data = newData;

        } else {
            // Using neighbor values method
            // Create temporary array for current iteration
            uint16_t** newData = new uint16_t*[image.rows];
            for (int i = 0; i < image.rows; i++) {
                newData[i] = new uint16_t[image.cols];
                std::copy(image.data[i], image.data[i] + image.cols, newData[i]);
            }

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

            // Free old data
            for (int i = 0; i < image.rows; i++) {
                delete[] image.data[i];
            }
            delete[] image.data;

            // Update image with new data
            image.data = newData;
        }
    }
}

void DarkLinePointerProcessor::removeAllDarkLines(ImageData& image, std::vector<DarkLine>& detectedLines) {
    removeDarkLinesSelective(image, detectedLines, true, true);
}

void DarkLinePointerProcessor::removeInObjectDarkLines(ImageData& image, std::vector<DarkLine>& detectedLines) {
    removeDarkLinesSelective(image, detectedLines, true, false);
}

void DarkLinePointerProcessor::removeIsolatedDarkLines(ImageData& image, std::vector<DarkLine>& detectedLines) {
    removeDarkLinesSelective(image, detectedLines, false, true);
}

void DarkLinePointerProcessor::removeDarkLinesSelective(
    ImageData& image,
    const std::vector<DarkLine>& lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (lines.empty()) return;

    // Create a new image array for modifications
    uint16_t** newData = new uint16_t*[image.rows];
    for (int i = 0; i < image.rows; i++) {
        newData[i] = new uint16_t[image.cols];
        std::copy(image.data[i], image.data[i] + image.cols, newData[i]);
    }

    // Process each line
    for (const auto& line : lines) {
        // Skip lines based on selection criteria
        if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
            continue;
        }

        if (method == RemovalMethod::DIRECT_STITCH) {
            // Pure direct stitch implementation
            if (line.isVertical) {
                // Calculate new dimensions after removing the vertical line
                int newWidth = image.cols - line.width;

                // For vertical lines, shift all content after the line to the left
                for (int y = 0; y < image.rows; ++y) {
                    // Copy the content after the line to the left
                    for (int x = line.x; x < newWidth; ++x) {
                        newData[y][x] = image.data[y][x + line.width];
                    }
                }

                // Resize columns
                for (int y = 0; y < image.rows; ++y) {
                    uint16_t* tempRow = new uint16_t[newWidth];
                    std::copy(newData[y], newData[y] + newWidth, tempRow);
                    delete[] newData[y];
                    newData[y] = tempRow;
                }
                image.cols = newWidth;
            } else {
                // Calculate new dimensions after removing the horizontal line
                int newHeight = image.rows - line.width;

                // For horizontal lines, shift all content after the line upward
                for (int y = line.y; y < newHeight; ++y) {
                    for (int x = 0; x < image.cols; ++x) {
                        newData[y][x] = image.data[y + line.width][x];
                    }
                }

                // Resize rows
                uint16_t** tempData = new uint16_t*[newHeight];
                for (int y = 0; y < newHeight; ++y) {
                    tempData[y] = newData[y];
                }
                // Delete extra rows
                for (int y = newHeight; y < image.rows; ++y) {
                    delete[] newData[y];
                }
                delete[] newData;
                newData = tempData;
                image.rows = newHeight;
            }
        } else {
            // Neighbor values method
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

    // Free old data
    for (int i = 0; i < image.rows; i++) {
        delete[] image.data[i];
    }
    delete[] image.data;

    // Update image with new data
    image.data = newData;
}
