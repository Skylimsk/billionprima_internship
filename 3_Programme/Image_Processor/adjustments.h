#ifndef ADJUSTMENTS_H
#define ADJUSTMENTS_H

#include <functional>
#include <QRect>

class ImageAdjustments {
public:
    // Overall adjustments
    static void adjustContrast(double**& img, int height, int width, float contrastFactor);
    static void adjustGammaOverall(double**& img, int height, int width, float gamma);
    static void sharpenImage(double**& img, int height, int width, float sharpenStrength);

    // Regional adjustments
    static void adjustGammaForSelectedRegion(double**& img, int height, int width, float gamma, const QRect& region);
    static void applySharpenToRegion(double**& img, int height, int width, float sharpenStrength, const QRect& region);
    static void applyContrastToRegion(double**& img, int height, int width, float contrastFactor, const QRect& region);

private:
    // Helper functions for threaded processing
    static void adjustGammaForRegion(double**& img, float gamma,
                                     int startY, int endY, int startX, int endX, int threadId);
    static void processSharpenChunk(int startY, int endY, int width,
                                    double**& img, double** const& tempImg,
                                    float sharpenStrength, int threadId);
    static void processContrastChunk(int top, int bottom, int left, int right,
                                     float contrastFactor, double**& img, int threadId);

    static void processRegion(double**& img, int height, int width, const QRect& region,
                              std::function<void(int, int, int)> operation);

    static double clamp(double value, double min, double max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
};

#endif // ADJUSTMENTS_H
