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
        qDebug() << "\nThread" << threadId << "- Analyzing horizontal line at y=" << pos
                 << " with width=" << lineWidth;

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
    const int minRadius = 10;
    const int maxRadius = 200;
    int radius = lineWidth * 2;
    return std::clamp(radius, minRadius, maxRadius);
}

double DarkLinePointerProcessor::findReplacementValue(
    const ImageData& image,
    int x, int y,
    bool isVertical,
    int lineWidth) {

    std::vector<double> validValues;
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

double DarkLinePointerProcessor::findStitchValue(
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
                double blackRatio = static_cast<double>(blackPixelCount) / image.rows;
                if (blackRatio > (1.0 - NOISE_TOLERANCE)) {
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

    // Check for line that extends to the edge
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
                double blackRatio = static_cast<double>(blackPixelCount) / image.cols;
                if (blackRatio > (1.0 - NOISE_TOLERANCE)) {
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

void DarkLinePointerProcessor::removeDarkLines(
    ImageData& image,
    const std::vector<DarkLine>& lines) {

    if (lines.empty()) return;

    // 创建新的图像数组用于修改
    double** newData = new double*[image.rows];
    for (int i = 0; i < image.rows; i++) {
        newData[i] = new double[image.cols];
        // 复制原始数据
        std::copy(image.data[i], image.data[i] + image.cols, newData[i]);
    }

    // 处理每条线
    for (const auto& line : lines) {
        // 跳过属于对象一部分的线
        if (line.inObject) {
            continue;
        }

        if (line.isVertical) {
            // 处理垂直线
            for (int x = line.x; x < line.x + line.width; ++x) {
                for (int y = 0; y < image.rows; ++y) {
                    if (image.data[y][x] <= MIN_BRIGHTNESS) {
                        newData[y][x] = findReplacementValue(image, x, y, true, line.width);
                    }
                }
            }
        } else {
            // 处理水平线
            for (int y = line.y; y < line.y + line.width; ++y) {
                for (int x = 0; x < image.cols; ++x) {
                    if (image.data[y][x] <= MIN_BRIGHTNESS) {
                        newData[y][x] = findReplacementValue(image, x, y, false, line.width);
                    }
                }
            }
        }
    }

    // 释放旧数据
    for (int i = 0; i < image.rows; i++) {
        delete[] image.data[i];
    }
    delete[] image.data;

    // 更新图像数据
    image.data = newData;
}

void DarkLinePointerProcessor::removeDarkLinesSequential(
    ImageData& image,
    const std::vector<DarkLine>& lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (lines.empty()) return;

    // 处理每条线
    for (const auto& line : lines) {
        // 根据选择条件跳过不需要处理的线
        if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
            continue;
        }

        if (method == RemovalMethod::DIRECT_STITCH) {
            // 创建当前迭代的新数组
            double** newData;
            int newRows = image.rows;
            int newCols = image.cols;

            if (line.isVertical) {
                // 计算移除垂直线后的新尺寸
                newCols = image.cols - line.width;
                newData = new double*[newRows];
                for (int i = 0; i < newRows; i++) {
                    newData[i] = new double[newCols];
                }

                // 复制数据并拼接
                for (int y = 0; y < newRows; ++y) {
                    // 复制线条前的数据
                    for (int x = 0; x < line.x; ++x) {
                        newData[y][x] = image.data[y][x];
                    }
                    // 复制线条后的数据（带偏移）
                    for (int x = line.x; x < newCols; ++x) {
                        newData[y][x] = image.data[y][x + line.width];
                    }
                }
            } else {
                // 计算移除水平线后的新尺寸
                newRows = image.rows - line.width;
                newData = new double*[newRows];
                for (int i = 0; i < newRows; i++) {
                    newData[i] = new double[newCols];
                }

                // 复制线条前的数据
                for (int y = 0; y < line.y; ++y) {
                    std::copy(image.data[y], image.data[y] + newCols, newData[y]);
                }

                // 复制线条后的数据（带偏移）
                for (int y = line.y; y < newRows; ++y) {
                    std::copy(image.data[y + line.width],
                              image.data[y + line.width] + newCols,
                              newData[y]);
                }
            }

            // 释放旧数据
            for (int i = 0; i < image.rows; i++) {
                delete[] image.data[i];
            }
            delete[] image.data;

            // 更新图像尺寸和数据
            image.rows = newRows;
            image.cols = newCols;
            image.data = newData;

        } else {  // NEIGHBOR_VALUES 方法
            // 创建当前迭代的临时数组
            double** newData = new double*[image.rows];
            for (int i = 0; i < image.rows; i++) {
                newData[i] = new double[image.cols];
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

            // 释放旧数据
            for (int i = 0; i < image.rows; i++) {
                delete[] image.data[i];
            }
            delete[] image.data;

            // 更新图像数据
            image.data = newData;
        }
    }
}

void DarkLinePointerProcessor::removeDarkLinesSelective(
    ImageData& image,
    const std::vector<DarkLine>& lines,
    bool removeInObject,
    bool removeIsolated,
    RemovalMethod method) {

    if (lines.empty()) return;

    // 创建新的图像数组用于修改
    double** newData = new double*[image.rows];
    for (int i = 0; i < image.rows; i++) {
        newData[i] = new double[image.cols];
        std::copy(image.data[i], image.data[i] + image.cols, newData[i]);
    }

    // 处理每条线
    for (const auto& line : lines) {
        // 根据选择条件跳过不需要处理的线
        if ((line.inObject && !removeInObject) || (!line.inObject && !removeIsolated)) {
            continue;
        }

        if (method == RemovalMethod::DIRECT_STITCH) {
            // 计算新尺寸
            int newWidth = image.cols;
            int newHeight = image.rows;

            if (line.isVertical) {
                newWidth -= line.width;
            } else {
                newHeight -= line.width;
            }

            // 创建临时数组
            double** tempData = new double*[newHeight];
            for (int i = 0; i < newHeight; i++) {
                tempData[i] = new double[newWidth];
            }

            if (line.isVertical) {
                // 对于垂直线，向左移动所有内容
                for (int y = 0; y < newHeight; ++y) {
                    // 复制线条左侧的内容
                    for (int x = 0; x < line.x; ++x) {
                        tempData[y][x] = newData[y][x];
                    }
                    // 复制线条右侧的内容（带偏移）
                    for (int x = line.x; x < newWidth; ++x) {
                        tempData[y][x] = newData[y][x + line.width];
                    }
                }
            } else {
                // 对于水平线，向上移动所有内容
                // 复制线条上方的内容
                for (int y = 0; y < line.y; ++y) {
                    for (int x = 0; x < newWidth; ++x) {
                        tempData[y][x] = newData[y][x];
                    }
                }
                // 复制线条下方的内容（带偏移）
                for (int y = line.y; y < newHeight; ++y) {
                    for (int x = 0; x < newWidth; ++x) {
                        tempData[y][x] = newData[y + line.width][x];
                    }
                }
            }

            // 释放当前的 newData
            for (int i = 0; i < image.rows; i++) {
                delete[] newData[i];
            }
            delete[] newData;

            // 更新 newData 为临时数组
            newData = tempData;
            image.rows = newHeight;
            image.cols = newWidth;

        } else {  // NEIGHBOR_VALUES
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

    // 释放原始数据
    for (int i = 0; i < image.rows; i++) {
        delete[] image.data[i];
    }
    delete[] image.data;

    // 更新图像为新数据
    image.data = newData;
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
