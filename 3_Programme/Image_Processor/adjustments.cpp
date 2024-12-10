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
    QRect normalizedRegion = region.normalized();
    int left = std::max(0, normalizedRegion.left());
    int top = std::max(0, normalizedRegion.top());
    int right = std::min(width, normalizedRegion.right() + 1);
    int bottom = std::min(height, normalizedRegion.bottom() + 1);

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int heightPerThread = (bottom - top) / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        int threadStartY = top + (i * heightPerThread);
        int threadEndY = (i == numThreads - 1) ? bottom : threadStartY + heightPerThread;

        threads.emplace_back([=, &img]() {
            for (int y = threadStartY; y < threadEndY; ++y) {
                for (int x = left; x < right; ++x) {
                    operation(x, y, i);
                }
            }
        });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

// Region-specific operations
void ImageAdjustments::adjustGammaForSelectedRegion(double**& img, int height, int width, float gamma,
                                                    const QRect& region) {
    float invGamma = 1.0f / gamma;
    processRegion(img, height, width, region, [&img, invGamma](int x, int y, int threadId) {
        float normalized = img[y][x] / 65535.0f;
        float corrected = std::pow(normalized, invGamma);
        img[y][x] = clamp(corrected * 65535.0f, 0.0, 65535.0);
    });
}

void ImageAdjustments::applySharpenToRegion(double**& img, int height, int width, float sharpenStrength,
                                            const QRect& region) {
    // Create temporary image
    double** tempImage = nullptr;
    malloc2D(tempImage, height, width);

    // Copy original image to temp
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            tempImage[y][x] = img[y][x];
        }
    }

    processRegion(img, height, width, region, [&img, &tempImage, sharpenStrength, width, height](int x, int y, int threadId) {
        if (x > 0 && x < width - 1 && y > 0 && y < height - 1) {
            double sum = 5 * tempImage[y][x] - tempImage[y-1][x] - tempImage[y+1][x] - tempImage[y][x-1] - tempImage[y][x+1];
            img[y][x] = std::clamp(sum * sharpenStrength + tempImage[y][x], 0.0, 65535.0);
        }
    });

    // Clean up temporary image
    for (int i = 0; i < height; ++i) {
        free(tempImage[i]);
    }
    free(tempImage);
}

void ImageAdjustments::applyContrastToRegion(double**& img, int height, int width, float contrastFactor,
                                             const QRect& region) {
    double midGray = 32767.5;
    processRegion(img, height, width, region, [&img, contrastFactor, midGray](int x, int y, int threadId) {
        double pixel = img[y][x];
        img[y][x] = std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0, 65535.0);
    });
}
