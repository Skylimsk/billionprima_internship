#ifndef ADJUSTMENTS_H
#define ADJUSTMENTS_H

#include <vector>
#include <functional>
#include <QRect>

class ImageAdjustments {
public:
    // Overall adjustments
    static void adjustContrast(std::vector<std::vector<uint16_t>>& img, float contrastFactor);
    static void adjustGammaOverall(std::vector<std::vector<uint16_t>>& img, float gamma);
    static void sharpenImage(std::vector<std::vector<uint16_t>>& img, float sharpenStrength);

    // Regional adjustments
    static void adjustGammaForSelectedRegion(std::vector<std::vector<uint16_t>>& img, float gamma, const QRect& region);
    static void applySharpenToRegion(std::vector<std::vector<uint16_t>>& img, float sharpenStrength, const QRect& region);
    static void applyContrastToRegion(std::vector<std::vector<uint16_t>>& img, float contrastFactor, const QRect& region);

private:
    // Helper functions for threaded processing
    static void adjustGammaForRegion(std::vector<std::vector<uint16_t>>& img, float gamma,
                                     int startY, int endY, int startX, int endX, int threadId);
    static void processSharpenChunk(int startY, int endY, int width,
                                    std::vector<std::vector<uint16_t>>& img,
                                    const std::vector<std::vector<uint16_t>>& tempImg,
                                    float sharpenStrength, int threadId);
    static void processSharpenRegionChunk(int startY, int endY, int left, int right,
                                          std::vector<std::vector<uint16_t>>& img,
                                          float sharpenStrength, int threadId);
    static void processContrastChunk(int top, int bottom, int left, int right,
                                     float contrastFactor, std::vector<std::vector<uint16_t>>& img,
                                     int threadId);

    static void processRegion(std::vector<std::vector<uint16_t>>& img, const QRect& region,
                              std::function<void(int, int, int)> operation);
};

#endif // ADJUSTMENTS_H
