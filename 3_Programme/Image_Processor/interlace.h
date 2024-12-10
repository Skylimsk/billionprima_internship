#ifndef INTERLACE_H
#define INTERLACE_H

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
        STATIC,
        INTENSITY_BASED,
        GRADIENT_BASED,
        VARIANCE_BASED
    };

    struct MergeParams {
        MergeMethod method;
        WeightingMethod weightMethod;
        float baseWeight;
        int windowSize;

        MergeParams(
            MergeMethod m = MergeMethod::WEIGHTED_AVERAGE,
            WeightingMethod w = WeightingMethod::STATIC,
            float weight = 0.5f,
            int window = 3
            );
    };

    struct InterlacedResult {
        double** lowEnergyImage;
        double** highEnergyImage;
        double** combinedImage;
        int height;
        int width;

        InterlacedResult(int h, int w);
        ~InterlacedResult();
    };

    static InterlacedResult* processEnhancedInterlacedSections(
        double** inputImage,
        int height, int width,
        StartPoint lowEnergyStart,
        StartPoint highEnergyStart,
        const MergeParams& userParams);

    // Utility methods for image allocation and deallocation
    static double** allocateImage(int height, int width);
    static void deallocateImage(double** image, int height);

    // Calibration methods
    static void setCalibrationParams(int linesToProcessY, int linesToProcessX);
    static void resetCalibrationParams() { calibrationParams = CalibrationParams(); }
    static bool hasCalibrationParams() { return calibrationParams.isInitialized; }

    static QString getCalibrationParamsString() {
        if (!calibrationParams.isInitialized) return QString();
        return QString("Y:%1, X:%2")
            .arg(calibrationParams.linesToProcessY)
            .arg(calibrationParams.linesToProcessX);
    }

    static QString getWeightingMethodString(const MergeParams& params);

    static int getStoredYParam() { return calibrationParams.linesToProcessY; }
    static int getStoredXParam() { return calibrationParams.linesToProcessX; }

    static MergeParams determineBestWeightingMethod(
        double** lowEnergySection,
        double** highEnergySection,
        int height, int width);

    private:
    static void performInterlacing(
        double** firstSection,
        double** secondSection,
        int height, int width,
        double** result);

    static float calculateLocalVariance(
        double** section,
        int height, int width,
        int centerY, int centerX,
        int windowSize);

    static float calculateDynamicWeight(
        double** lowSection,
        double** highSection,
        int height, int width,
        int y, int x,
        const MergeParams& params);

    static void mergeInterlacedSections(
        double** lowEnergySection,
        double** highEnergySection,
        int height, int width,
        const MergeParams& mergeParams,
        double** result);

    static void applyCalibration(double** image, int height, int width);

    // Calibration parameters structure
    static struct CalibrationParams {
        int linesToProcessY;
        int linesToProcessX;
        bool isInitialized;

        CalibrationParams() : linesToProcessY(10), linesToProcessX(10), isInitialized(false) {}
    } calibrationParams;
};

#endif // INTERLACE_H
