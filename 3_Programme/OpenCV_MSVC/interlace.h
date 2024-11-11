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

    struct InterlacedResult {
        std::vector<std::vector<uint16_t>> lowEnergyImage;
        std::vector<std::vector<uint16_t>> highEnergyImage;
        std::vector<std::vector<uint16_t>> combinedImage;
    };

    static InterlacedResult processEnhancedInterlacedSections(
        const std::vector<std::vector<uint16_t>>& inputImage,
        StartPoint lowEnergyStart,
        StartPoint highEnergyStart,
        MergeMethod mergeMethod
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

private:
    static std::vector<std::vector<uint16_t>> performInterlacing(
        const std::vector<std::vector<uint16_t>>& firstSection,
        const std::vector<std::vector<uint16_t>>& secondSection
        );

    static std::vector<std::vector<uint16_t>> mergeInterlacedSections(
        const std::vector<std::vector<uint16_t>>& lowEnergySection,
        const std::vector<std::vector<uint16_t>>& highEnergySection,
        MergeMethod method
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
