#include "adjustments.h"
#include <thread>
#include <iostream>
#include <cmath>
#include <algorithm>
#include "pch.h"

void ImageAdjustments::adjustGammaForRegion(double**& img, float gamma,
                                            int startY, int endY, int startX, int endX, int threadId) {
    float invGamma = 1.0f / gamma;
    std::cout << "Thread " << threadId << " processing gamma correction for rows " << startY << " to " << endY << std::endl;
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            float normalized = img[y][x] / 65535.0f;
            float corrected = std::pow(normalized, invGamma);
            img[y][x] = clamp(corrected * 65535.0f, 0.0, 65535.0); // Using our clamp function
        }
    }
    std::cout << "Thread " << threadId << " completed gamma correction" << std::endl;
}

void ImageAdjustments::processContrastChunk(int top, int bottom, int left, int right,
                                            float contrastFactor, double**& img, int threadId) {
    std::cout << "Thread " << threadId << " processing contrast for rows " << top << " to " << bottom << std::endl;
    const double MAX_PIXEL_VALUE = 65535.0;
    const double midGray = MAX_PIXEL_VALUE / 2.0;
    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            double pixel = img[y][x];
            img[y][x] = clamp((pixel - midGray) * contrastFactor + midGray, 0.0, MAX_PIXEL_VALUE); // Using our clamp function
        }
    }
    std::cout << "Thread " << threadId << " completed contrast adjustment" << std::endl;
}

void ImageAdjustments::processSharpenChunk(int startY, int endY, int width,
                                           double**& img, double** const& tempImg,
                                           float sharpenStrength, int threadId) {
    std::cout << "Thread " << threadId << " processing advanced sharpen for rows " << startY << " to " << endY << std::endl;
    float kernel[3][3] = {
        { 0, -1 * sharpenStrength,  0 },
        { -1 * sharpenStrength, 1 + 4 * sharpenStrength, -1 * sharpenStrength },
        { 0, -1 * sharpenStrength,  0 }
    };

    for (int y = startY; y < endY; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            if (y > 0 && y < endY - 1 && x > 0 && x < width - 1) {
                double newPixelValue = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        newPixelValue += kernel[ky + 1][kx + 1] * tempImg[y + ky][x + kx];
                    }
                }
                img[y][x] = std::clamp(newPixelValue, 0.0, 65535.0);
            }
        }
    }
    std::cout << "Thread " << threadId << " completed advanced sharpen" << std::endl;
}

void ImageAdjustments::adjustContrast(double**& img, int height, int width, float contrastFactor) {
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    std::cout << "Starting contrast adjustment with " << numThreads << " threads" << std::endl;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;

        threads.emplace_back([=, &img]() {
            processContrastChunk(startY, endY, 0, width, contrastFactor, img, i);
        });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::cout << "All threads completed contrast adjustment" << std::endl;
}

void ImageAdjustments::adjustGammaOverall(double**& img, int height, int width, float gamma) {
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int blockHeight = height / numThreads;

    std::cout << "Starting gamma adjustment with " << numThreads << " threads" << std::endl;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * blockHeight;
        int endY = (i == numThreads - 1) ? height : startY + blockHeight;
        threads.emplace_back(adjustGammaForRegion, std::ref(img), gamma, startY, endY, 0, width, i);
    }

    for (auto& thread : threads) {
        thread.join();
    }
    std::cout << "All threads completed gamma adjustment" << std::endl;
}

void ImageAdjustments::sharpenImage(double**& img, int height, int width, float sharpenStrength) {
    // Create temporary image
    double** tempImg = nullptr;
    malloc2D(tempImg, height, width);

    // Copy original image to temp
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tempImg[y][x] = img[y][x];
        }
    }

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    std::cout << "Starting image sharpening with " << numThreads << " threads" << std::endl;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;
        threads.emplace_back(processSharpenChunk, startY, endY, width, std::ref(img), std::ref(tempImg), sharpenStrength, i);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Clean up temporary image
    for (int i = 0; i < height; ++i) {
        free(tempImg[i]);
    }
    free(tempImg);

    std::cout << "All threads completed image sharpening" << std::endl;
}

void ImageAdjustments::processRegion(double**& img, int height, int width, const QRect& region,
                                     std::function<void(int, int, int)> operation) {
    if (!img || height <= 0 || width <= 0) {
        return;
    }

    // Row pointer validation
    for (int i = 0; i < height; ++i) {
        if (!img[i]) {
            return;
        }
    }

    QRect normalizedRegion = region.normalized();
    int left = std::clamp(normalizedRegion.left(), 0, width - 1);
    int top = std::clamp(normalizedRegion.top(), 0, height - 1);
    int right = std::clamp(normalizedRegion.right(), 0, width - 1);
    int bottom = std::clamp(normalizedRegion.bottom(), 0, height - 1);

    if (right <= left || bottom <= top) {
        return;
    }

    const int regionHeight = bottom - top + 1;
    const int numThreads = std::min(std::thread::hardware_concurrency(),
                                    static_cast<unsigned int>(regionHeight));
    std::vector<std::thread> threads;
    int heightPerThread = std::max(1, regionHeight / static_cast<int>(numThreads));

    for (int i = 0; i < numThreads; ++i) {
        int threadStartY = top + (i * heightPerThread);
        int threadEndY = (i == numThreads - 1) ? bottom + 1 : threadStartY + heightPerThread;
        threadStartY = std::clamp(threadStartY, top, bottom + 1);
        threadEndY = std::clamp(threadEndY, threadStartY, bottom + 1);

        if (threadEndY > threadStartY) {
            threads.emplace_back([=, &img, &operation]() {
                for (int y = threadStartY; y < threadEndY; ++y) {
                    for (int x = left; x < right; ++x) {
                        if (x >= 0 && x < width && y >= 0 && y < height) {
                            operation(x, y, i);
                        }
                    }
                }
            });
        }
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// Region-specific operations
void ImageAdjustments::applySharpenToRegion(double**& img, int height, int width, float sharpenStrength,
                                            const QRect& region) {
    if (!img || height <= 0 || width <= 0) return;

    double** tempImage = nullptr;
    malloc2D(tempImage, height, width);
    if (!tempImage) return;

    // Copy original image to temp
    for (int y = 0; y < height; ++y) {
        if (img[y] && tempImage[y]) {
            memcpy(tempImage[y], img[y], width * sizeof(double));
        }
    }

    processRegion(img, height, width, region, [&img, &tempImage, sharpenStrength, width, height](int x, int y, int threadId) {
        if (x > 0 && x < width - 1 && y > 0 && y < height - 1 &&
            img[y] && tempImage[y] && tempImage[y-1] && tempImage[y+1]) {
            double sum = 5 * tempImage[y][x] -
                         tempImage[y-1][x] - tempImage[y+1][x] -
                         tempImage[y][x-1] - tempImage[y][x+1];
            img[y][x] = std::clamp(sum * sharpenStrength + tempImage[y][x], 0.0, 65535.0);
        }
    });

    if (tempImage) {
        for (int i = 0; i < height; ++i) {
            if (tempImage[i]) free(tempImage[i]);
        }
        free(tempImage);
    }
}

void ImageAdjustments::adjustGammaForSelectedRegion(double**& img, int height, int width, float gamma, const QRect& region) {
    if (!img || height <= 0 || width <= 0 || gamma <= 0) return;

    QRect normalizedRegion = region.normalized();
    int left = std::clamp(normalizedRegion.left(), 0, width - 1);
    int top = std::clamp(normalizedRegion.top(), 0, height - 1);
    int right = std::clamp(normalizedRegion.right(), 0, width - 1);
    int bottom = std::clamp(normalizedRegion.bottom(), 0, height - 1);

    if (left >= width || top >= height || right < 0 || bottom < 0 || left > right || top > bottom) {
        return;
    }

    float invGamma = 1.0f / gamma;
    const int regionHeight = bottom - top + 1;
    const int numThreads = std::min(std::thread::hardware_concurrency(),
                                    static_cast<unsigned int>(regionHeight));
    std::vector<std::thread> threads;
    int heightPerThread = std::max(1, regionHeight / static_cast<int>(numThreads));

    for (int i = 0; i < numThreads; ++i) {
        int startY = top + (i * heightPerThread);
        int endY = (i == numThreads - 1) ? bottom + 1 : startY + heightPerThread;
        startY = std::clamp(startY, top, bottom + 1);
        endY = std::clamp(endY, startY, bottom + 1);

        if (startY < endY) {
            threads.emplace_back([=, &img]() {
                for (int y = startY; y < endY; ++y) {
                    if (y >= 0 && y < height && img[y]) {
                        for (int x = left; x <= right; ++x) {
                            if (x >= 0 && x < width) {
                                float normalized = img[y][x] / 65535.0f;
                                float corrected = std::pow(normalized, invGamma);
                                img[y][x] = clamp(corrected * 65535.0f, 0.0, 65535.0);
                            }
                        }
                    }
                }
            });
        }
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}
void ImageAdjustments::applyContrastToRegion(double**& img, int height, int width, float contrastFactor,
                                             const QRect& region) {
    if (!img || height <= 0 || width <= 0) return;

    double midGray = 32767.5;
    processRegion(img, height, width, region, [&img, contrastFactor, midGray](int x, int y, int threadId) {
        if (img[y]) { // 添加空指针检查
            double pixel = img[y][x];
            img[y][x] = std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0, 65535.0);
        }
    });
}

void ImageAdjustments::adjustGammaWithThreshold(double**& img, int height, int width,
                                                float gamma, float threshold) {
    float invGamma = 1.0f / gamma;
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    // Process image in parallel chunks
    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;

        threads.emplace_back([=, &img]() {
            for (int y = startY; y < endY; ++y) {
                for (int x = 0; x < width; ++x) {
                    // Only apply gamma correction if pixel value is below threshold
                    if (img[y][x] <= threshold) {
                        float normalized = img[y][x] / 65535.0f;
                        float corrected = std::pow(normalized, invGamma);
                        img[y][x] = clamp(corrected * 65535.0f, 0.0, 65535.0);
                    }
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void ImageAdjustments::sharpenWithThreshold(double**& img, int height, int width,
                                            float sharpenStrength, float threshold) {
    // Create temporary image for unmodified pixel values
    double** tempImg = nullptr;
    malloc2D(tempImg, height, width);

    // Copy original image to temp
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tempImg[y][x] = img[y][x];
        }
    }

    float kernel[3][3] = {
        { 0, -1 * sharpenStrength,  0 },
        { -1 * sharpenStrength, 1 + 4 * sharpenStrength, -1 * sharpenStrength },
        { 0, -1 * sharpenStrength,  0 }
    };

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;

        threads.emplace_back([=, &img, &tempImg]() {
            for (int y = startY; y < endY; ++y) {
                for (int x = 1; x < width - 1; ++x) {
                    if (tempImg[y][x] <= threshold) {
                        if (y > 0 && y < height - 1) {
                            double newPixelValue = 0;
                            for (int ky = -1; ky <= 1; ++ky) {
                                for (int kx = -1; kx <= 1; ++kx) {
                                    newPixelValue += kernel[ky + 1][kx + 1] * tempImg[y + ky][x + kx];
                                }
                            }
                            img[y][x] = std::clamp(newPixelValue, 0.0, 65535.0);
                        }
                    }
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Clean up temporary image
    for (int i = 0; i < height; ++i) {
        free(tempImg[i]);
    }
    free(tempImg);
}

void ImageAdjustments::adjustContrastWithThreshold(double**& img, int height, int width,
                                                   float contrastFactor, float threshold) {
    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int chunkHeight = height / numThreads;

    const double midGray = 32767.5;

    for (int i = 0; i < numThreads; ++i) {
        int startY = i * chunkHeight;
        int endY = (i == numThreads - 1) ? height : startY + chunkHeight;

        threads.emplace_back([=, &img]() {
            for (int y = startY; y < endY; ++y) {
                for (int x = 0; x < width; ++x) {
                    if (img[y][x] <= threshold) {
                        double pixel = img[y][x];
                        img[y][x] = clamp((pixel - midGray) * contrastFactor + midGray, 0.0, 65535.0);
                    }
                }
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}
