#ifndef DARKLINE_POINTER_H
#define DARKLINE_POINTER_H

#include <mutex>
#include <thread>
#include <memory>
#include <sstream>
#include <iomanip>
#include <iostream>

// Image data structure using double 2D pointer
class ImageData {
public:
    double** data;    // 2D pointer for image data
    int rows;         // Number of rows in the image
    int cols;         // Number of columns in the image

    // Constructor
    ImageData() : data(nullptr), rows(0), cols(0) {}

    // Copy constructor
    ImageData(const ImageData& other);

    // Assignment operator
    ImageData& operator=(const ImageData& other);

    // Move constructor
    ImageData(ImageData&& other) noexcept;

    // Move assignment operator
    ImageData& operator=(ImageData&& other) noexcept;

    // Destructor
    ~ImageData() { cleanup(); }

private:
    void cleanup() {
        if (data) {
            for (int i = 0; i < rows; i++) {
                delete[] data[i];
            }
            delete[] data;
            data = nullptr;
        }
        rows = 0;
        cols = 0;
    }
};

// Dark line structure
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

    // Constructor
    DarkLine() : x(0), y(0), startY(0), endY(0), startX(0), endX(0),
        width(0), isVertical(false), inObject(false) {}
};

// DarkLine Array structure using double pointer
struct DarkLineArray {
    DarkLine** lines;  // 2D pointer for storing dark lines
    int rows;         // Number of rows in the array
    int cols;         // Number of columns in the array

    // Constructor
    DarkLineArray() : lines(nullptr), rows(0), cols(0) {}

    // Destructor
    ~DarkLineArray() {
        cleanup();
    }

    // Copy constructor
    DarkLineArray(const DarkLineArray& other) {
        copyFrom(other);
    }

    // Assignment operator
    DarkLineArray& operator=(const DarkLineArray& other) {
        if (this != &other) {
            cleanup();
            copyFrom(other);
        }
        return *this;
    }

    // Move constructor
    DarkLineArray(DarkLineArray&& other) noexcept
        : lines(other.lines), rows(other.rows), cols(other.cols) {
        other.lines = nullptr;
        other.rows = 0;
        other.cols = 0;
    }

    // Move assignment operator
    DarkLineArray& operator=(DarkLineArray&& other) noexcept {
        if (this != &other) {
            cleanup();
            lines = other.lines;
            rows = other.rows;
            cols = other.cols;
            other.lines = nullptr;
            other.rows = 0;
            other.cols = 0;
        }
        return *this;
    }

private:
    void cleanup() {
        if (lines) {
            for (int i = 0; i < rows; i++) {
                delete[] lines[i];
            }
            delete[] lines;
            lines = nullptr;
        }
        rows = 0;
        cols = 0;
    }

    void copyFrom(const DarkLineArray& other) {
        rows = other.rows;
        cols = other.cols;
        if (other.lines) {
            lines = new DarkLine*[rows];
            for (int i = 0; i < rows; i++) {
                lines[i] = new DarkLine[cols];
                for (int j = 0; j < cols; j++) {
                    lines[i][j] = other.lines[i][j];
                }
            }
        } else {
            lines = nullptr;
        }
    }
};

class DarkLinePointerProcessor {
public:
    // Constants for detection and processing
    static constexpr double BLACK_THRESHOLD = 1000.0;
    static constexpr double WHITE_THRESHOLD = 55000.0;
    static constexpr double MIN_BRIGHTNESS = 1000.0;
    static constexpr double NOISE_TOLERANCE = 0.1;
    static constexpr int MIN_LINE_WIDTH = 1;
    static constexpr int VERTICAL_CHECK_RANGE = 3;
    static constexpr int HORIZONTAL_CHECK_RANGE = 2;
    static constexpr int MIN_SEARCH_RADIUS = 10;
    static constexpr int MAX_SEARCH_RADIUS = 200;

    enum class RemovalMethod {
        NEIGHBOR_VALUES,  // Using neighbor values to fill
        DIRECT_STITCH    // Direct stitching
    };

    // Constructor and Destructor
    DarkLinePointerProcessor() : selectedLines(nullptr), selectedLinesCount(0), selectedLinesCapacity(0) {}

    ~DarkLinePointerProcessor() {
        delete[] selectedLines;
    }

    // Core detection and removal functions
    static DarkLineArray* detectDarkLines(const ImageData& image);
    static void removeDarkLines(ImageData& image, const DarkLineArray* lines);

    // Specialized removal functions
    static void removeAllDarkLines(ImageData& image, DarkLineArray* lines);
    static void removeInObjectDarkLines(ImageData& image, DarkLineArray* lines);
    static void removeIsolatedDarkLines(ImageData& image, DarkLineArray* lines);

    // Line management functions
    void addSelectedLine(const DarkLine& line) {
        if (selectedLinesCount >= selectedLinesCapacity) {
            int newCapacity = (selectedLinesCapacity == 0) ? 1 : selectedLinesCapacity * 2;
            DarkLine* newArray = new DarkLine[newCapacity];

            for (int i = 0; i < selectedLinesCount; i++) {
                newArray[i] = selectedLines[i];
            }

            delete[] selectedLines;
            selectedLines = newArray;
            selectedLinesCapacity = newCapacity;
        }

        selectedLines[selectedLinesCount++] = line;
    }

    void clearSelectedLines() {
        delete[] selectedLines;
        selectedLines = nullptr;
        selectedLinesCount = 0;
        selectedLinesCapacity = 0;
    }

    int getSelectedLinesCount() const {
        return selectedLinesCount;
    }

    const DarkLine* getSelectedLines() const {
        return selectedLines;
    }

    // Enhanced removal functions
    static void removeDarkLinesSelective(
        ImageData& image,
        const DarkLineArray* lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES
        );

    // Sequential processing functions
    static void removeDarkLinesSequential(
        ImageData& image,
        DarkLineArray* lines,
        DarkLine** selectedLines,
        int selectedCount,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES
        );

    // Helper functions made public
    static DarkLineArray* createDarkLineArray(int rows, int cols);
    static void destroyDarkLineArray(DarkLineArray* array);
    static void copyDarkLineArray(const DarkLineArray* source, DarkLineArray* destination);

    // 修改这三个函数的声明
    static void validateNewDimensions(
        const ImageData& image,
        const std::pair<DarkLine, int>** lines,  // 改为 2D pointer
        int linesCount,                          // 添加数量参数
        bool isVertical,
        int& newSize
        );

    static bool prepareImageBuffer(
        const ImageData& image,
        const std::pair<DarkLine, int>** lines,  // 改为 2D pointer
        int linesCount,                          // 添加数量参数
        ImageData& buffer
        );

private:
    // Member variables
    DarkLine* selectedLines;
    int selectedLinesCount;
    int selectedLinesCapacity;

    // Helper functions
    static int calculateSearchRadius(int lineWidth);
    static bool isInObject(const ImageData& image, int pos, int lineWidth, bool isVertical, int threadId);

    static double findReplacementValue(
        const ImageData& image,
        int x, int y,
        bool isVertical,
        int lineWidth
        );

    static double findStitchValue(
        const ImageData& image,
        const DarkLine& line,
        int x, int y
        );

    // Image manipulation helpers
    static std::unique_ptr<ImageData> createImageCopy(const ImageData& source);
    static void copyImageData(const ImageData& source, ImageData& destination);
    static void resizeImageData(ImageData& image, int newRows, int newCols);

    // 修改 adjustLineCoordinates 函数声明
    static void adjustLineCoordinates(
        DarkLine& line,
        const std::pair<int, int>** verticalSections,   // 改为 2D pointer
        int verticalCount,                              // 添加数量参数
        const std::pair<int, int>** horizontalSections, // 改为 2D pointer
        int horizontalCount,                            // 添加数量参数
        int newWidth,
        int newHeight
        );

    static bool validateLineArray(const DarkLineArray* lines, const ImageData& image);
    static DarkLineArray* createSafeDarkLineArray(int rows, int cols);
};

#endif // DARKLINE_POINTER_H
