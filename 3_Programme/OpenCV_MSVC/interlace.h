#ifndef INTERLACE_H
#define INTERLACE_H

#include <vector>

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
};

#endif // INTERLACE_H
