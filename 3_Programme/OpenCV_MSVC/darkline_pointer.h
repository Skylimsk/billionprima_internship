#ifndef DARKLINE_POINTER_H
#define DARKLINE_POINTER_H

#include <vector>
#include <cstdint>
#include <mutex>
#include <thread>

// 图像数据结构，使用 double 类型的 2D pointer
struct ImageData {
    double** data;    // 2D pointer for image data
    int rows;         // Number of rows in the image
    int cols;         // Number of columns in the image

    // Constructor
    ImageData() : data(nullptr), rows(0), cols(0) {}

    // Destructor
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
    // 暗线结构体
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
        NEIGHBOR_VALUES,  // Using neighbor values to fill
        DIRECT_STITCH    // Direct stitching
    };

    // Main functionality
    static std::vector<DarkLine> detectDarkLines(const ImageData& image);
    static void removeDarkLines(ImageData& image, const std::vector<DarkLine>& lines);

    // Specialized removal functions
    static void removeAllDarkLines(ImageData& image, std::vector<DarkLine>& detectedLines);
    static void removeInObjectDarkLines(ImageData& image, std::vector<DarkLine>& detectedLines);
    static void removeIsolatedDarkLines(ImageData& image, std::vector<DarkLine>& detectedLines);

    static void removeDarkLinesSelective(
        ImageData& image,
        const std::vector<DarkLine>& lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES);

    // Helper functions for pixel value replacement
    static double findReplacementValue(
        const ImageData& image,
        int x, int y,
        bool isVertical,
        int lineWidth);

    static double findStitchValue(
        const ImageData& image,
        const DarkLine& line,
        int x, int y);

    // Sequential line removal
    static void removeDarkLinesSequential(
        ImageData& image,
        const std::vector<DarkLine>& lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES);

private:
    // Constants for detection and removal
    static constexpr double BLACK_THRESHOLD = 1000.0;
    static constexpr double NOISE_TOLERANCE = 0.1;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr int VERTICAL_CHECK_RANGE = 3;
    static constexpr int HORIZONTAL_CHECK_RANGE = 2;
    static constexpr double WHITE_THRESHOLD = 55000.0;
    static constexpr double MIN_BRIGHTNESS = 1000.0;

    // Helper functions
    static int calculateSearchRadius(int lineWidth);
    static bool isInObject(const ImageData& image, int pos, int lineWidth, bool isVertical, int threadId);
};

#endif // DARKLINE_POINTER_H
