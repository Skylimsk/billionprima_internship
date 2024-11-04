#include "adjustments.h"
#include <thread>
#include <iostream>
#include <cmath>
#include <algorithm>

void ImageAdjustments::adjustGammaForRegion(std::vector<std::vector<uint16_t>>& img, float gamma,
                                            int startY, int endY, int startX, int endX, int threadId) {
    float invGamma = 1.0f / gamma;
    std::cout << "Thread " << threadId << " processing gamma correction for rows " << startY << " to " << endY << std::endl;
    for (int y = startY; y < endY; ++y) {
        for (int x = startX; x < endX; ++x) {
            float normalized = img[y][x] / 65535.0f;
            float corrected = std::pow(normalized, invGamma);
            img[y][x] = static_cast<uint16_t>(corrected * 65535.0f);
        }
    }
    std::cout << "Thread " << threadId << " completed gamma correction" << std::endl;
}

void ImageAdjustments::processContrastChunk(int top, int bottom, int left, int right,
                                            float contrastFactor, std::vector<std::vector<uint16_t>>& img,
                                            int threadId) {
    std::cout << "Thread " << threadId << " processing contrast for rows " << top << " to " << bottom << std::endl;
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;
    for (int y = top; y < bottom; ++y) {
        for (int x = left; x < right; ++x) {
            float pixel = static_cast<float>(img[y][x]);
            img[y][x] = static_cast<uint16_t>(std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0f, MAX_PIXEL_VALUE));
        }
    }
    std::cout << "Thread " << threadId << " completed contrast adjustment" << std::endl;
}

void ImageAdjustments::processSharpenChunk(int startY, int endY, int width,
                                           std::vector<std::vector<uint16_t>>& img,
                                           const std::vector<std::vector<uint16_t>>& tempImg,
                                           float sharpenStrength, int threadId) {
    std::cout << "Thread " << threadId << " processing advanced sharpen for rows " << startY << " to " << endY << std::endl;
    float kernel[3][3] = {
        { 0, -1 * sharpenStrength,  0 },
        { -1 * sharpenStrength, 1 + 4 * sharpenStrength, -1 * sharpenStrength },
        { 0, -1 * sharpenStrength,  0 }
    };

    for (std::vector<std::vector<uint16_t>>::size_type y = startY; y < static_cast<std::vector<std::vector<uint16_t>>::size_type>(endY); ++y) {
        for (std::vector<uint16_t>::size_type x = 1; x < static_cast<std::vector<uint16_t>::size_type>(width) - 1; ++x) {
            if (y > 0 && y < tempImg.size() - 1 && x > 0 && x < tempImg[0].size() - 1) {
                float newPixelValue = 0;
                for (int ky = -1; ky <= 1; ++ky) {
                    for (int kx = -1; kx <= 1; ++kx) {
                        newPixelValue += kernel[ky + 1][kx + 1] * tempImg[y + ky][x + kx];
                    }
                }
                img[y][x] = static_cast<uint16_t>(std::clamp(newPixelValue, 0.0f, 65535.0f));
            }
        }
    }
    std::cout << "Thread " << threadId << " completed advanced sharpen" << std::endl;
}

void ImageAdjustments::adjustContrast(std::vector<std::vector<uint16_t>>& img, float contrastFactor) {
    const float MAX_PIXEL_VALUE = 65535.0f;
    const float midGray = MAX_PIXEL_VALUE / 2.0f;
    int height = img.size();
    int width = img[0].size();

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

void ImageAdjustments::adjustGammaOverall(std::vector<std::vector<uint16_t>>& img, float gamma) {
    int height = img.size();
    int width = img[0].size();

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

void ImageAdjustments::sharpenImage(std::vector<std::vector<uint16_t>>& img, float sharpenStrength) {
    int height = img.size();
    int width = img[0].size();
    std::vector<std::vector<uint16_t>> tempImg = img;

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
    std::cout << "All threads completed image sharpening" << std::endl;
}

void ImageAdjustments::processRegion(std::vector<std::vector<uint16_t>>& img, const QRect& region,
                                     std::function<void(int, int, int)> operation) {
    QRect normalizedRegion = region.normalized();
    int left = std::max(0, normalizedRegion.left());
    int top = std::max(0, normalizedRegion.top());
    int right = std::min(static_cast<int>(img[0].size()), normalizedRegion.right() + 1);
    int bottom = std::min(static_cast<int>(img.size()), normalizedRegion.bottom() + 1);

    const int numThreads = std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    int heightPerThread = (bottom - top) / numThreads;

    std::cout << "Starting region processing with " << numThreads << " threads" << std::endl;

    for (int i = 0; i < numThreads; ++i) {
        int threadStartY = top + (i * heightPerThread);
        int threadEndY = (i == numThreads - 1) ? bottom : threadStartY + heightPerThread;

        threads.emplace_back([=, &img]() {
            std::cout << "Thread " << i << " processing region from row " << threadStartY << " to " << threadEndY << std::endl;
            for (int y = threadStartY; y < threadEndY; ++y) {
                for (int x = left; x < right; ++x) {
                    operation(x, y, i);
                }
            }
            std::cout << "Thread " << i << " completed region processing" << std::endl;
        });
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    std::cout << "All threads completed region processing" << std::endl;
}

void ImageAdjustments::adjustGammaForSelectedRegion(std::vector<std::vector<uint16_t>>& img, float gamma,
                                                    const QRect& region) {
    float invGamma = 1.0f / gamma;
    processRegion(img, region, [&img, invGamma](int x, int y, int threadId) {
        float normalized = img[y][x] / 65535.0f;
        float corrected = std::pow(normalized, invGamma);
        img[y][x] = static_cast<uint16_t>(corrected * 65535.0f);
    });
}

void ImageAdjustments::applySharpenToRegion(std::vector<std::vector<uint16_t>>& img, float sharpenStrength,
                                            const QRect& region) {
    std::vector<std::vector<uint16_t>> tempImage = img;
    processRegion(img, region, [&img, &tempImage, sharpenStrength](int x, int y, int threadId) {
        auto width = static_cast<int>(img[0].size());
        auto height = static_cast<int>(img.size());
        if (x > 0 && x < width - 1 && y > 0 && y < height - 1) {
            int sum = 5 * tempImage[y][x] - tempImage[y-1][x] - tempImage[y+1][x] - tempImage[y][x-1] - tempImage[y][x+1];
            img[y][x] = static_cast<uint16_t>(std::clamp(static_cast<float>(sum) * sharpenStrength + static_cast<float>(tempImage[y][x]), 0.0f, 65535.0f));
        }
    });
}

void ImageAdjustments::applyContrastToRegion(std::vector<std::vector<uint16_t>>& img, float contrastFactor,
                                             const QRect& region) {
    float midGray = 32767.5f;
    processRegion(img, region, [&img, contrastFactor, midGray](int x, int y, int threadId) {
        float pixel = static_cast<float>(img[y][x]);
        img[y][x] = static_cast<uint16_t>(std::clamp((pixel - midGray) * contrastFactor + midGray, 0.0f, 65535.0f));
    });
}
