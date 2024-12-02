#ifndef INTERLACE_H
#define INTERLACE_H

#include <vector>
#include <QString>

class InterlaceProcessor {
public:
    enum class StartPoint {
        LEFT_LEFT,
        LEFT_RIGHT,
        RIGHT_LEFT,
        RIGHT_RIGHT
    };

    enum class MergeMethod {
        WEIGHTED_AVERAGE,
        MINIMUM_VALUE
    };

    enum class WeightingMethod {
        STATIC,             // Use fixed weights
        INTENSITY_BASED,    // Weight based on pixel intensity
        GRADIENT_BASED,     // Weight based on local gradients
        VARIANCE_BASED      // Weight based on local variance
    };

    struct MergeParams {
        MergeMethod method;
        WeightingMethod weightMethod;
        float baseWeight;  // Base weight for static method or minimum weight for dynamic
        int windowSize;    // Size of window for local calculations (odd number)

        MergeParams(
            MergeMethod m = MergeMethod::WEIGHTED_AVERAGE,
            WeightingMethod w = WeightingMethod::STATIC,
            float weight = 0.5f,
            int window = 3
            ) : method(m), weightMethod(w), baseWeight(weight), windowSize(window) {}
    };

    struct InterlacedResult {
        std::vector<std::vector<uint16_t>> lowEnergyImage;
        std::vector<std::vector<uint16_t>> highEnergyImage;
        std::vector<std::vector<uint16_t>> combinedImage;
    };

    static InterlacedResult processEnhancedInterlacedSections(  // Add 'static' here
        const std::vector<std::vector<uint16_t>>& inputImage,
        StartPoint lowEnergyStart,
        StartPoint highEnergyStart,
        const MergeParams& mergeParams
        );


    // Calibration related methods
    static void setCalibrationParams(int linesToProcessY, int linesToProcessX);
    static void applyCalibration(std::vector<std::vector<uint16_t>>& image);
    static bool hasCalibrationParams() { return calibrationParams.isInitialized; }
    static void resetCalibrationParams() { calibrationParams = CalibrationParams(); }

    static QString getCalibrationParamsString() {
        if (!calibrationParams.isInitialized) return QString();
        return QString("Y:%1, X:%2")
            .arg(calibrationParams.linesToProcessY)
            .arg(calibrationParams.linesToProcessX);
    }

    static QString getWeightingMethodString(const MergeParams& params) {
        QString methodStr;

        // Merge method
        methodStr += "Merge: ";
        methodStr += (params.method == MergeMethod::WEIGHTED_AVERAGE) ?
                         "Weighted Average" : "Minimum Value";

        // If using weighted average, add weighting details
        if (params.method == MergeMethod::WEIGHTED_AVERAGE) {
            methodStr += "\nWeighting: ";
            switch (params.weightMethod) {
            case WeightingMethod::STATIC:
                methodStr += QString("Static (Low:%1, High:%2)")
                                 .arg(params.baseWeight)
                                 .arg(1.0f - params.baseWeight);
                break;

            case WeightingMethod::INTENSITY_BASED:
                methodStr += QString("Intensity Based (Base:%1)")
                                 .arg(params.baseWeight);
                break;

            case WeightingMethod::GRADIENT_BASED:
                methodStr += QString("Gradient Based (Base:%1)")
                                 .arg(params.baseWeight);
                break;

            case WeightingMethod::VARIANCE_BASED:
                methodStr += QString("Variance Based (Base:%1, Window:%2)")
                                 .arg(params.baseWeight)
                                 .arg(params.windowSize);
                break;
            }
        }

        return methodStr;
    }

    static MergeParams determineBestWeightingMethod(
        const std::vector<std::vector<uint16_t>>& lowEnergySection,
        const std::vector<std::vector<uint16_t>>& highEnergySection
        ) {
        int height = lowEnergySection.size();
        int width = lowEnergySection[0].size();

        // Calculate image characteristics
        float avgGradient = 0.0f;
        float avgIntensity = 0.0f;
        float avgVariance = 0.0f;
        int samplePoints = 0;

        // Sample points throughout the image (every 10th pixel)
        for (int y = 1; y < height - 1; y += 10) {
            for (int x = 1; x < width - 1; x += 10) {
                // Calculate gradient
                float lowGradX = std::abs(static_cast<float>(lowEnergySection[y][x+1]) -
                                          static_cast<float>(lowEnergySection[y][x-1])) / 2.0f;
                float lowGradY = std::abs(static_cast<float>(lowEnergySection[y+1][x]) -
                                          static_cast<float>(lowEnergySection[y-1][x])) / 2.0f;
                float gradMag = std::sqrt(lowGradX * lowGradX + lowGradY * lowGradY);
                avgGradient += gradMag;

                // Calculate intensity
                float intensity = static_cast<float>(lowEnergySection[y][x]);
                avgIntensity += intensity;

                // Calculate local variance with a 3x3 window
                float variance = calculateLocalVariance(lowEnergySection, y, x, 3);
                avgVariance += variance;

                samplePoints++;
            }
        }

        avgGradient /= samplePoints;
        avgIntensity /= samplePoints;
        avgVariance /= samplePoints;

        // Normalize metrics to compare them
        float normalizedGradient = avgGradient / 65535.0f;  // Normalize to 16-bit range
        float normalizedIntensity = avgIntensity / 65535.0f;
        float normalizedVariance = std::min(avgVariance / (65535.0f * 65535.0f), 1.0f);

        // Decision logic
        WeightingMethod selectedMethod;
        float baseWeight = 0.5f;  // Default weight
        int windowSize = 3;       // Default window size

        if (normalizedGradient > 0.1f) {
            // High gradient areas - use gradient-based weighting
            selectedMethod = WeightingMethod::GRADIENT_BASED;
            baseWeight = 0.6f;  // Slight bias towards low energy section
        }
        else if (normalizedVariance > 0.05f) {
            // High variance areas - use variance-based weighting
            selectedMethod = WeightingMethod::VARIANCE_BASED;
            baseWeight = 0.55f;
            windowSize = 5;  // Larger window for better variance estimation
        }
        else {
            // Relatively uniform areas - use intensity-based weighting
            selectedMethod = WeightingMethod::INTENSITY_BASED;
            baseWeight = 0.5f;  // Equal weighting
        }

        return MergeParams(
            MergeMethod::WEIGHTED_AVERAGE,
            selectedMethod,
            baseWeight,
            windowSize
            );
    }

    static int getStoredYParam() { return calibrationParams.linesToProcessY; }
    static int getStoredXParam() { return calibrationParams.linesToProcessX; }

private:
    static std::vector<std::vector<uint16_t>> performInterlacing(
        const std::vector<std::vector<uint16_t>>& firstSection,
        const std::vector<std::vector<uint16_t>>& secondSection
        );

    // Update this declaration to match the implementation
    static std::vector<std::vector<uint16_t>> mergeInterlacedSections(
        const std::vector<std::vector<uint16_t>>& lowEnergySection,
        const std::vector<std::vector<uint16_t>>& highEnergySection,
        const MergeParams& mergeParams
        );

    static float calculateDynamicWeight(
        const std::vector<std::vector<uint16_t>>& lowSection,
        const std::vector<std::vector<uint16_t>>& highSection,
        int y, int x,
        const MergeParams& params
        );

    static float calculateLocalVariance(
        const std::vector<std::vector<uint16_t>>& section,
        int centerY, int centerX,
        int windowSize
        );

    // Calibration parameters structure
    static struct CalibrationParams {
        int linesToProcessY;
        int linesToProcessX;
        bool isInitialized;

        CalibrationParams() : linesToProcessY(10), linesToProcessX(10), isInitialized(false) {}
    } calibrationParams;
};

#endif // INTERLACE_H
