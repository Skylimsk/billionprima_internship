#include "dark_line.h"
#include <QDebug>
#include <algorithm>
#include <iostream>

bool DarkLineProcessor::isInObject(const std::vector<std::vector<uint16_t>>& image, int pos, int lineWidth, bool isVertical, int threadId) {
    int height = image.size();
    int width = image[0].size();

    if (isVertical) {
        std::cout << "Thread " << threadId << " checking vertical line at x=" << pos << std::endl;
        for (int y = 0; y < height; ++y) {
            // Check left side
            for (int i = 1; i <= VERTICAL_CHECK_RANGE; ++i) {
                if (pos - i >= 0) {
                    uint16_t leftPixel = image[y][pos - i];
                    if (leftPixel < WHITE_THRESHOLD) {
                        qDebug() << "Thread" << threadId << "- Vertical line at x=" << pos
                                 << ": left pixel at y=" << y << "value=" << leftPixel;
                        return true;
                    }
                }
            }

            // Check right side
            for (int i = 1; i <= VERTICAL_CHECK_RANGE; ++i) {
                if (pos + lineWidth + i - 1 < width) {
                    uint16_t rightPixel = image[y][pos + lineWidth + i - 1];
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
                for (int x = 0; x < width; x += width/10) {
                    uint16_t topPixel = image[pos - i][x];
                    if (topPixel < WHITE_THRESHOLD) {
                        qDebug() << "Thread" << threadId << "- Found non-white pixel above line at x=" << x;
                        return true;
                    }
                }
            }
        }

        // Sample pixels below the line
        for (int i = 1; i <= HORIZONTAL_CHECK_RANGE; ++i) {
            if (pos + lineWidth + i - 1 < height) {
                for (int x = 0; x < width; x += width/10) {
                    uint16_t bottomPixel = image[pos + lineWidth + i - 1][x];
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

uint16_t DarkLineProcessor::findReplacementValue(
    const std::vector<std::vector<uint16_t>>& image,
    int x, int y,
    bool isVertical,
    int lineWidth) {

    int height = image.size();
    int width = image[0].size();
    std::vector<uint16_t> validValues;

    // Calculate dynamic search radius based on line width
    int searchRadius = calculateSearchRadius(lineWidth);
    validValues.reserve(searchRadius * 2);

    if (isVertical) {
        // Look left and right
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Check left
            int leftX = x - offset;
            if (leftX >= 0 && image[y][leftX] > MIN_BRIGHTNESS) {
                validValues.push_back(image[y][leftX]);
            }

            // Check right
            int rightX = x + offset;
            if (rightX < width && image[y][rightX] > MIN_BRIGHTNESS) {
                validValues.push_back(image[y][rightX]);
            }
        }
    } else {
        // Look up and down
        for (int offset = 1; offset <= searchRadius; ++offset) {
            // Check up
            int upY = y - offset;
            if (upY >= 0 && image[upY][x] > MIN_BRIGHTNESS) {
                validValues.push_back(image[upY][x]);
            }

            // Check down
            int downY = y + offset;
            if (downY < height && image[downY][x] > MIN_BRIGHTNESS) {
                validValues.push_back(image[downY][x]);
            }
        }
    }

    if (validValues.empty()) {
        return image[y][x];  // Keep original if no valid replacements found
    }

    // Use median value for replacement
    std::sort(validValues.begin(), validValues.end());
    return validValues[validValues.size() / 2];
}

std::vector<DarkLineProcessor::DarkLine> DarkLineProcessor::detectDarkLines(
    const std::vector<std::vector<uint16_t>>& image) {

    std::vector<DarkLine> detectedLines;
    if (image.empty()) return detectedLines;

    int height = image.size();
    int width = image[0].size();
    const int numThreads = std::thread::hardware_concurrency();

    std::mutex detectedLinesMutex;

    // Detect vertical lines using multiple threads
    std::vector<bool> isBlackColumn(width, false);
    std::vector<std::thread> verticalThreads;
    int columnsPerThread = width / numThreads;

    std::cout << "Starting vertical line detection with " << numThreads << " threads" << std::endl;

    for (int t = 0; t < numThreads; ++t) {
        int startX = t * columnsPerThread;
        int endX = (t == numThreads - 1) ? width : startX + columnsPerThread;

        verticalThreads.emplace_back([=, &isBlackColumn, &image]() {
            std::cout << "Thread " << t << " processing columns " << startX << " to " << endX << std::endl;
            for (int x = startX; x < endX; ++x) {
                int blackPixelCount = 0;
                for (int y = 0; y < height; ++y) {
                    if (image[y][x] <= BLACK_THRESHOLD) {
                        blackPixelCount++;
                    }
                }
                float blackRatio = static_cast<float>(blackPixelCount) / height;
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
    for (int x = 0; x < width; ++x) {
        if (isBlackColumn[x]) {
            if (startX == -1) startX = x;
        } else if (startX != -1) {
            DarkLine line;
            line.x = startX;
            line.startY = 0;
            line.endY = height - 1;
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
        line.endY = height - 1;
        line.width = width - startX;
        line.isVertical = true;
        line.inObject = isInObject(image, startX, line.width, true, 0);

        if (line.width >= MIN_LINE_WIDTH) {
            std::lock_guard<std::mutex> lock(detectedLinesMutex);
            detectedLines.push_back(line);
        }
    }

    // Detect horizontal lines using multiple threads
    std::vector<bool> isBlackRow(height, false);
    std::vector<std::thread> horizontalThreads;
    int rowsPerThread = height / numThreads;

    std::cout << "Starting horizontal line detection with " << numThreads << " threads" << std::endl;

    for (int t = 0; t < numThreads; ++t) {
        int startY = t * rowsPerThread;
        int endY = (t == numThreads - 1) ? height : startY + rowsPerThread;

        horizontalThreads.emplace_back([=, &isBlackRow, &image]() {
            std::cout << "Thread " << t << " processing rows " << startY << " to " << endY << std::endl;
            for (int y = startY; y < endY; ++y) {
                int blackPixelCount = 0;
                for (int x = 0; x < width; ++x) {
                    if (image[y][x] <= BLACK_THRESHOLD) {
                        blackPixelCount++;
                    }
                }
                float blackRatio = static_cast<float>(blackPixelCount) / width;
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
    for (int y = 0; y < height; ++y) {
        if (isBlackRow[y]) {
            if (startY == -1) startY = y;
        } else if (startY != -1) {
            DarkLine line;
            line.y = startY;
            line.startX = 0;
            line.endX = width - 1;
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
        line.endX = width - 1;
        line.width = height - startY;
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

void DarkLineProcessor::removeDarkLines(std::vector<std::vector<uint16_t>>& image,
                                        const std::vector<DarkLine>& lines) {
    if (lines.empty()) return;

    std::vector<std::vector<uint16_t>> newImage = image;
    int height = image.size();
    int width = image[0].size();

    // Process each line with its exact detected width
    for (const auto& line : lines) {
        // Skip lines that are part of objects
        if (line.inObject) {
            continue;
        }

        if (line.isVertical) {
            // Process vertical line
            for (int x = line.x; x < line.x + line.width; ++x) {
                for (int y = 0; y < height; ++y) {
                    if (image[y][x] <= MIN_BRIGHTNESS) {
                        newImage[y][x] = findReplacementValue(image, x, y, true, line.width);
                    }
                }
            }
        } else {
            // Process horizontal line
            for (int y = line.y; y < line.y + line.width; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (image[y][x] <= MIN_BRIGHTNESS) {
                        newImage[y][x] = findReplacementValue(image, x, y, false, line.width);
                    }
                }
            }
        }
    }

    // Update the image
    image = std::move(newImage);
}

void DarkLineProcessor::removeAllDarkLines(std::vector<std::vector<uint16_t>>& image,
                                           std::vector<DarkLine>& detectedLines) {
    // Remove both in-object and isolated lines
    removeDarkLinesSelective(image, detectedLines, true, true);
}

void DarkLineProcessor::removeInObjectDarkLines(std::vector<std::vector<uint16_t>>& image,
                                                std::vector<DarkLine>& detectedLines) {
    // Remove only in-object lines
    removeDarkLinesSelective(image, detectedLines, true, false);
}

void DarkLineProcessor::removeIsolatedDarkLines(std::vector<std::vector<uint16_t>>& image,
                                                std::vector<DarkLine>& detectedLines) {
    // Remove only isolated lines
    removeDarkLinesSelective(image, detectedLines, false, true);
}

// Modified findStitchValue function for direct stitching
uint16_t DarkLineProcessor::findStitchValue(const std::vector<std::vector<uint16_t>>& image,
                                            const DarkLine& line,
                                            int x, int y) {
    int height = image.size();
    int width = image[0].size();

    if (line.isVertical) {
        // For vertical lines, find values from both sides
        int leftX = line.x - 1;
        int rightX = line.x + line.width;

        if (leftX >= 0 && rightX < width) {
            // If we're closer to the left edge, use left value
            if (x - line.x < line.width / 2) {
                return image[y][leftX];
            } else {
                // Otherwise use right value
                return image[y][rightX];
            }
        } else if (leftX >= 0) {
            return image[y][leftX];
        } else if (rightX < width) {
            return image[y][rightX];
        }
    } else {
        // For horizontal lines, find values from top and bottom
        int topY = line.y - 1;
        int bottomY = line.y + line.width;

        if (topY >= 0 && bottomY < height) {
            // If we're closer to the top, use top value
            if (y - line.y < line.width / 2) {
                return image[topY][x];
            } else {
                // Otherwise use bottom value
                return image[bottomY][x];
            }
        } else if (topY >= 0) {
            return image[topY][x];
        } else if (bottomY < height) {
            return image[bottomY][x];
        }
    }

    return image[y][x]; // Return original value if no stitch is possible
}

void DarkLineProcessor::removeDarkLinesSelective(
    std::vector<std::vector<uint16_t>>& image,
    const std::vector<DarkLine>& lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (lines.empty()) return;
    std::vector<std::vector<uint16_t>> newImage = image;
    int height = image.size();
    int width = image[0].size();

    for (const auto& line : lines) {
        // Skip lines based on selection criteria
        if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
            continue;
        }

        if (method == RemovalMethod::DIRECT_STITCH) {
            // Pure direct stitch implementation
            if (line.isVertical) {
                // For vertical lines, shift all content after the line to the left
                for (int y = 0; y < height; ++y) {
                    for (int x = line.x; x < width - line.width; ++x) {
                        newImage[y][x] = image[y][x + line.width];
                    }
                }
            } else {
                // For horizontal lines, shift all content after the line upward
                for (int y = line.y; y < height - line.width; ++y) {
                    for (int x = 0; x < width; ++x) {
                        newImage[y][x] = image[y + line.width][x];
                    }
                }
            }
        } else {
            // Original neighbor values method for lines
            if (line.isVertical) {
                for (int x = line.x; x < line.x + line.width; ++x) {
                    for (int y = 0; y < height; ++y) {
                        if (image[y][x] <= MIN_BRIGHTNESS) {
                            newImage[y][x] = findReplacementValue(image, x, y, true, line.width);
                        }
                    }
                }
            } else {
                for (int y = line.y; y < line.y + line.width; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (image[y][x] <= MIN_BRIGHTNESS) {
                            newImage[y][x] = findReplacementValue(image, x, y, false, line.width);
                        }
                    }
                }
            }
        }
    }

    // Resize the image if any lines were removed using direct stitch
    if (method == RemovalMethod::DIRECT_STITCH) {
        int totalVerticalWidth = 0;
        int totalHorizontalHeight = 0;

        // Calculate total width/height to remove
        for (const auto& line : lines) {
            if ((line.inObject && removeInObject) || (!line.inObject && removeIsolated)) {
                if (line.isVertical) {
                    totalVerticalWidth += line.width;
                } else {
                    totalHorizontalHeight += line.width;
                }
            }
        }

        // Resize the image
        if (totalVerticalWidth > 0 || totalHorizontalHeight > 0) {
            int newWidth = width - totalVerticalWidth;
            int newHeight = height - totalHorizontalHeight;
            std::vector<std::vector<uint16_t>> resizedImage(newHeight, std::vector<uint16_t>(newWidth));

            // Copy the valid portion of the image
            for (int y = 0; y < newHeight; ++y) {
                for (int x = 0; x < newWidth; ++x) {
                    resizedImage[y][x] = newImage[y][x];
                }
            }
            newImage = std::move(resizedImage);
        }
    }

    image = std::move(newImage);
}

void DarkLineProcessor::removeDarkLinesSequential(
    std::vector<std::vector<uint16_t>>& image,
    const std::vector<DarkLine>& lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (lines.empty()) return;

    int height = image.size();
    int width = image[0].size();

    // 处理每一条线
    for (const auto& line : lines) {
        // 根据选择条件跳过不需要处理的线
        if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
            continue;
        }

        if (method == RemovalMethod::DIRECT_STITCH) {
            // 每处理一条线创建新的图像
            std::vector<std::vector<uint16_t>> newImage = image;

            if (line.isVertical) {
                // 竖直线：将线右侧的内容向左移动
                for (int y = 0; y < height; ++y) {
                    for (int x = line.x; x < width - line.width; ++x) {
                        newImage[y][x] = image[y][x + line.width];
                    }
                }

                // 调整图像宽度
                width -= line.width;
                std::vector<std::vector<uint16_t>> resizedImage(height, std::vector<uint16_t>(width));
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        resizedImage[y][x] = newImage[y][x];
                    }
                }
                image = std::move(resizedImage);
            } else {
                // 水平线：将线下方的内容向上移动
                for (int y = line.y; y < height - line.width; ++y) {
                    for (int x = 0; x < width; ++x) {
                        newImage[y][x] = image[y + line.width][x];
                    }
                }

                // 调整图像高度
                height -= line.width;
                std::vector<std::vector<uint16_t>> resizedImage(height, std::vector<uint16_t>(width));
                for (int y = 0; y < height; ++y) {
                    for (int x = 0; x < width; ++x) {
                        resizedImage[y][x] = newImage[y][x];
                    }
                }
                image = std::move(resizedImage);
            }
        } else {
            // 使用原来的邻近值方法
            std::vector<std::vector<uint16_t>> newImage = image;
            if (line.isVertical) {
                for (int x = line.x; x < line.x + line.width; ++x) {
                    for (int y = 0; y < height; ++y) {
                        if (image[y][x] <= MIN_BRIGHTNESS) {
                            newImage[y][x] = findReplacementValue(image, x, y, true, line.width);
                        }
                    }
                }
            } else {
                for (int y = line.y; y < line.y + line.width; ++y) {
                    for (int x = 0; x < width; ++x) {
                        if (image[y][x] <= MIN_BRIGHTNESS) {
                            newImage[y][x] = findReplacementValue(image, x, y, false, line.width);
                        }
                    }
                }
            }
            image = std::move(newImage);
        }
    }
}

int DarkLineProcessor::calculateSearchRadius(int lineWidth) {
    // Base the search radius on the line width
    // Minimum radius of 10 pixels, maximum of 200 pixels
    // Use a multiplier of 2 for the line width
    const int minRadius = 10;
    const int maxRadius = 200;
    int radius = lineWidth * 2;

    // Ensure the radius stays within reasonable bounds
    return std::clamp(radius, minRadius, maxRadius);
}
