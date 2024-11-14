#ifndef DARKLINE_POINTER_H
#define DARKLINE_POINTER_H

#include <vector>
#include <cstdint>
#include <mutex>
#include <thread>

// Structure to hold image data with double pointer
struct ImageData {
    uint16_t** data;  // 2D double pointer for image data
    int rows;         // Number of rows in the image
    int cols;         // Number of columns in the image

    // Constructor
    ImageData() : data(nullptr), rows(0), cols(0) {}

    // Destructor to clean up memory
    ~ImageData() {
        if (data) {
            for (int i = 0; i < rows; i++) {
                delete[] data[i];
            }
            delete[] data;
        }
    }
};

class DarkLinePointerProcessor {
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

    enum class RemovalMethod {
        NEIGHBOR_VALUES,  // Using neighboring values to fill
        DIRECT_STITCH    // Direct stitching
    };

    // Main functionality modified to use ImageData
    static std::vector<DarkLine> detectDarkLines(const ImageData& image);
    static void removeDarkLines(ImageData& image, const std::vector<DarkLine>& lines);

    static void removeAllDarkLines(ImageData& image,
                                   std::vector<DarkLine>& detectedLines);

    static void removeInObjectDarkLines(ImageData& image,
                                        std::vector<DarkLine>& detectedLines);

    static void removeIsolatedDarkLines(ImageData& image,
                                        std::vector<DarkLine>& detectedLines);

    static void removeDarkLinesSelective(
        ImageData& image,
        const std::vector<DarkLine>& lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES);

    static uint16_t findReplacementValue(
        const ImageData& image,
        int x, int y,
        bool isVertical,
        int lineWidth);

    static uint16_t findStitchValue(
        const ImageData& image,
        const DarkLine& line,
        int x, int y);

    static void removeDarkLinesSequential(
        ImageData& image,
        const std::vector<DarkLine>& lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES);

private:
    // Constants for detection and removal
    static constexpr uint16_t BLACK_THRESHOLD = 1000;
    static constexpr float NOISE_TOLERANCE = 0.1f;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr int VERTICAL_CHECK_RANGE = 3;
    static constexpr int HORIZONTAL_CHECK_RANGE = 2;
    static constexpr uint16_t WHITE_THRESHOLD = 55000;
    static constexpr uint16_t MIN_BRIGHTNESS = 1000;

    static int calculateSearchRadius(int lineWidth);
    static bool isInObject(const ImageData& image, int pos, int lineWidth, bool isVertical, int threadId);
};

#endif // DARKLINE_POINTER_H
