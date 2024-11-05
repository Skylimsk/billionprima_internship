#ifndef DARK_LINE_H
#define DARK_LINE_H

#include <vector>
#include <cstdint>
#include <mutex>
#include <thread>

class DarkLineProcessor {
public:
    struct DarkLine {
        int x;           // x coordinate of the dark line
        int y;           // y coordinate for horizontal lines
        int startY;      // start Y coordinate for vertical lines
        int endY;        // end Y coordinate for vertical lines
        int startX;      // start X coordinate for horizontal lines
        int endX;        // end X coordinate for horizontal lines
        int width;       // width of the line
        bool isVertical; // true for vertical lines, false for horizontal
        bool inObject;   // true if line is within a dark object
    };

    // Main functionality
    static std::vector<DarkLine> detectDarkLines(const std::vector<std::vector<uint16_t>>& image);
    static void removeDarkLines(std::vector<std::vector<uint16_t>>& image, const std::vector<DarkLine>& lines);

    static void removeDarkLinesSelective(std::vector<std::vector<uint16_t>>& image,
                                         const std::vector<DarkLine>& lines,
                                         bool removeInObject,
                                         bool removeIsolated);

private:
    // Constants for detection and removal
    static constexpr uint16_t BLACK_THRESHOLD = 1000;
    static constexpr float NOISE_TOLERANCE = 0.1f;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr int VERTICAL_CHECK_RANGE = 3;
    static constexpr int HORIZONTAL_CHECK_RANGE = 2;
    static constexpr uint16_t WHITE_THRESHOLD = 55000;
    static constexpr int SEARCH_RADIUS = 20;
    static constexpr uint16_t MIN_BRIGHTNESS = 1000;

    // Helper functions
    static bool isInObject(const std::vector<std::vector<uint16_t>>& image, int pos, int lineWidth, bool isVertical, int threadId);
    static uint16_t findReplacementValue(const std::vector<std::vector<uint16_t>>& image, int x, int y, bool isVertical);
};

#endif // DARK_LINE_H
