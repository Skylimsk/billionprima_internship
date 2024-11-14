#ifndef DARKLINE_POINTER_H_NEW
#define DARKLINE_POINTER_H_NEW

#include <mutex>
#include <thread>

// Rename ImageData to DarkLineImageData
struct DarkLineImageData {
    double** data;     // 2D pointer for image data
    int rows;          // number of rows
    int cols;          // number of columns

    DarkLineImageData() : data(nullptr), rows(0), cols(0) {}
    ~DarkLineImageData() {
        if (data) {
            for (int i = 0; i < rows; ++i) {
                delete[] data[i];
            }
            delete[] data;
        }
    }
};

// Rename DarkLine to DarkLinePtr
struct DarkLinePtr {
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

// Rename DarkLineArray to DarkLinePtrArray
struct DarkLinePtrArray {
    DarkLinePtr* lines;    // pointer to array of dark lines
    int count;          // number of lines in the array
    int capacity;       // capacity of the array

    DarkLinePtrArray() : lines(nullptr), count(0), capacity(0) {}
    ~DarkLinePtrArray() {
        if (lines) {
            delete[] lines;
        }
    }
};

// Main class renamed to make pointer-based implementation clear
class DarkLinePointerProcessor {
public:
    enum class RemovalMethod {
        NEIGHBOR_VALUES,  // Use neighbor values for filling
        DIRECT_STITCH    // Direct stitch
    };

    // Update function signatures to use new type names
    static DarkLinePtrArray* detectDarkLines(const DarkLineImageData& image);
    static void removeDarkLines(DarkLineImageData& image, const DarkLinePtrArray* lines);

    static void removeAllDarkLines(DarkLineImageData& image, DarkLinePtrArray* detectedLines);
    static void removeInObjectDarkLines(DarkLineImageData& image, DarkLinePtrArray* detectedLines);
    static void removeIsolatedDarkLines(DarkLineImageData& image, DarkLinePtrArray* detectedLines);

    static void removeDarkLinesSelective(
        DarkLineImageData& image,
        const DarkLinePtrArray* lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES
        );

    static double findReplacementValue(
        const DarkLineImageData& image,
        int x, int y,
        bool isVertical,
        int lineWidth
        );

    static double findStitchValue(
        const DarkLineImageData& image,
        const DarkLinePtr& line,
        int x, int y
        );

    static void removeDarkLinesSequential(
        DarkLineImageData& image,
        const DarkLinePtrArray* lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES
        );

private:
    // Constants remain the same
    static constexpr double BLACK_THRESHOLD = 1000.0;
    static constexpr double NOISE_TOLERANCE = 0.1;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr int VERTICAL_CHECK_RANGE = 3;
    static constexpr int HORIZONTAL_CHECK_RANGE = 2;
    static constexpr double WHITE_THRESHOLD = 55000.0;
    static constexpr double MIN_BRIGHTNESS = 1000.0;
    static constexpr int INITIAL_CAPACITY = 100;

    static int calculateSearchRadius(int lineWidth);
    static bool isInObject(const DarkLineImageData& image, int pos, int lineWidth, bool isVertical, int threadId);

    static void addDarkLine(DarkLinePtrArray* array, const DarkLinePtr& line);
    static void expandArray(DarkLinePtrArray* array);
};

#endif // DARKLINE_POINTER_H_NEW
