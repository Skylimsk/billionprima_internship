#ifndef DARK_LINE_H
#define DARK_LINE_H

#include <vector>
#include <cstdint>
#include <opencv2/core/types.hpp>

class DarkLineProcessor {
public:
    struct DarkLine {
        cv::Point start;
        cv::Point end;
        int thickness;
    };

    static std::vector<DarkLine> detectDarkLines(
        const std::vector<std::vector<uint16_t>>& image,
        uint16_t brightThreshold = 60000,
        uint16_t darkThreshold = 5000,
        int minLineLength = 20);

    static void removeDarkLines(
        std::vector<std::vector<uint16_t>>& image,
        const std::vector<DarkLine>& lines);

    static void removeFromZeroX(
        std::vector<std::vector<uint16_t>>& image,
        const std::vector<DarkLine>& lines);

private:
    static bool isInBrightRegion(
        const std::vector<std::vector<uint16_t>>& image,
        int x, int y,
        uint16_t brightThreshold);

    static std::vector<std::vector<bool>> findBrightRegions(
        const std::vector<std::vector<uint16_t>>& image,
        uint16_t brightThreshold);

    static void refineDarkLineDetection(std::vector<DarkLine>& lines);

    static uint16_t interpolateValue(
        const std::vector<std::vector<uint16_t>>& image,
        int x, int y,
        int margin = 5);

    static int calculateLineThickness(
        const std::vector<std::pair<int, int>>& linePixels,
        int minX, int maxX,
        int minY, int maxY);
};

#endif // DARK_LINE_H
