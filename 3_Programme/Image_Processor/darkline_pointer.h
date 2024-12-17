#ifndef DARKLINE_POINTER_H
#define DARKLINE_POINTER_H

#include <mutex>
#include <thread>
#include <memory>
#include <sstream>
#include <iomanip>
#include <iostream>


class ImageData {
public:
    double** data;    // 2D array storing pixel values
    int rows;         // Number of rows in the image
    int cols;         // Number of columns in the image

    // Default constructor
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

struct DarkLine {
    int x;           // X coordinate of the line origin
    int y;           // Y coordinate for horizontal lines
    int startY;      // Start Y coordinate for vertical lines
    int endY;        // End Y coordinate for vertical lines
    int startX;      // Start X coordinate for horizontal lines
    int endX;        // End X coordinate for horizontal lines
    int width;       // Width of the line in pixels
    bool isVertical; // Line orientation flag
    bool inObject;   // Flag indicating if line is within a dark object

    // Constructor with default initialization
    DarkLine() : x(0), y(0), startY(0), endY(0), startX(0), endX(0),
        width(0), isVertical(false), inObject(false) {}
};

struct DarkLineArray {
    DarkLine** lines;  // 2D array of dark lines
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
    // Detection thresholds and parameters
    static constexpr double BLACK_THRESHOLD = 1000.0;    // Threshold for identifying black pixels
    static constexpr double WHITE_THRESHOLD = 55000.0;   // Threshold for identifying white pixels
    static constexpr double MIN_BRIGHTNESS = 1000.0;     // Minimum brightness for non-dark pixels
    static constexpr double NOISE_TOLERANCE = 0.1;       // Tolerance for noise in line detection
    static constexpr int MIN_LINE_WIDTH = 1;            // Minimum width for valid lines
    static constexpr int VERTICAL_CHECK_RANGE = 3;      // Range for checking vertical line context
    static constexpr int HORIZONTAL_CHECK_RANGE = 2;    // Range for checking horizontal line context
    static constexpr int MIN_SEARCH_RADIUS = 10;        // Minimum radius for neighbor search
    static constexpr int MAX_SEARCH_RADIUS = 200;       // Maximum radius for neighbor search


    enum class RemovalMethod {
        NEIGHBOR_VALUES,  // Fill removed lines using neighboring pixel values
        DIRECT_STITCH    // Directly stitch image parts after line removal
    };

    // Detect dark lines in the image
    static DarkLineArray* detectDarkLines(const ImageData& image);
    static DarkLineArray* detectVerticalLines(const ImageData& image);
    static DarkLineArray* detectHorizontalLines(const ImageData& image);

    // Check for lines in specific directions
    static bool checkforHorizontal(const ImageData& image, DarkLineArray*& outLines);
    static bool checkforVertical(const ImageData& image, DarkLineArray*& outLines);
    static bool checkforBoth(const ImageData& image, DarkLineArray*& outLines);

    // Manage DarkLineArray
    static DarkLineArray* createDarkLineArray(int rows, int cols);
    static void destroyDarkLineArray(DarkLineArray* array);
    static void copyDarkLineArray(const DarkLineArray* source, DarkLineArray* destination);
    static DarkLineArray* createSafeDarkLineArray(int rows, int cols);

    static void removeDarkLinesSelective(
        ImageData& image,
        const DarkLineArray* lines,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES
        );


    static void removeDarkLinesSequential(
        ImageData& image,
        DarkLineArray* lines,
        DarkLine** selectedLines,
        int selectedCount,
        bool removeInObject,
        bool removeIsolated,
        RemovalMethod method = RemovalMethod::NEIGHBOR_VALUES
        );

private:

    static int calculateSearchRadius(int lineWidth);

    // Check if a position is within an object in the image
    static bool isInObject(
        const ImageData& image,
        int pos,
        int lineWidth,
        bool isVertical,
        int threadId
        );

    // Image manipulation helpers
    static std::unique_ptr<ImageData> createImageCopy(const ImageData& source);
    static void copyImageData(const ImageData& source, ImageData& destination);

    static double findReplacementValue(
        const ImageData& image,
        int x,
        int y,
        bool isVertical,
        int lineWidth
        );


};


#endif // DARKLINE_POINTER_H
