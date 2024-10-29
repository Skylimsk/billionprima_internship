#ifndef IMAGE_PROCESSING_PARAMS_H
#define IMAGE_PROCESSING_PARAMS_H

#include <cstdint>

struct ImageProcessingParams {
    float yStretchFactor = 1.0f;          // Stretch factor in Y-axis (default: 1.0)
    float contrastFactor = 1.09f;         // Contrast factor (default: 1.09)
    float sharpenStrength = 1.09f;        // Sharpening strength (default: 1.09)
    float gammaValue = 1.2f;              // Gamma correction value (default: 1.2)
    int linesToProcessX = 100;            // Number of lines to process (default: 200)
    float darkPixelThreshold = 0.5f;      // Threshold for dark pixels (default: 0.5)
    uint16_t darkThreshold = 2048;        // Threshold for dark regions (default: 2048)
    float regionGamma = 1.0f;             // Gamma for selected region (default: 1.0)
    float regionSharpen = 1.0f;           // Sharpening for selected region (default: 1.0)
    float regionContrast = 1.0f;          // Contrast for selected region (default: 1.0)
    float distortionFactor = 1.0f;        // Distortion factor (default: 1.0)
    bool useSineDistortion = false;       // Toggle sine distortion effect
    int paddingSize = 1;                  // Padding size for image processing (default: 1)
    float scalingFactor = 1.06f;
    float claheClipLimit = 8.0;           // CLAHE clip limit (default: 8.0)
    int linesToProcessY = 100;
};

#endif // IMAGE_PROCESSING_PARAMS_H
