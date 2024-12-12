#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <QObject>
#include <QLabel>
#include <QRect>
#include <QString>
#include <QRegularExpression>
#include <stack>
#include <mutex>
#include <opencv2/core.hpp>
#include <vector>
#include <utility>
#include "image_processing_params.h"
#include "CLAHE.h"
#include "darkline_pointer.h"
#include "adjustments.h"
#include "interlace.h"
#include "zoom.h"
#include "pch.h"

class ImageProcessor : public QObject {

public:
    struct ActionRecord {
        QString action;
        QString parameters;

        QString toString() const {
            return parameters.isEmpty() ? action : action + " - " + parameters;
        }
    };

    struct ImageState {
        double** image;
        int height;
        int width;
        DarkLineArray* darkLines;

        ImageState() : image(nullptr), height(0), width(0), darkLines(nullptr) {}

        ~ImageState() {
            if (image) {
                for (int i = 0; i < height; i++) {
                    if (image[i]) {  // Add null check for each row
                        delete[] image[i];
                    }
                }
                delete[] image;
                image = nullptr;  // Set to nullptr after deletion
            }
            if (darkLines) {
                DarkLinePointerProcessor::destroyDarkLineArray(darkLines);
                darkLines = nullptr;  // Set to nullptr after deletion
            }
        }

        // Add copy constructor for proper deep copying
        ImageState(const ImageState& other)
            : height(other.height), width(other.width), darkLines(nullptr) {
            if (other.image) {
                image = new double*[height];
                for (int i = 0; i < height; i++) {
                    image[i] = new double[width];
                    std::memcpy(image[i], other.image[i], width * sizeof(double));
                }
            } else {
                image = nullptr;
            }

            if (other.darkLines) {
                darkLines = new DarkLineArray();
                DarkLinePointerProcessor::copyDarkLineArray(other.darkLines, darkLines);
            }
        }

        // Delete assignment operator to prevent accidental shallow copies
        ImageState& operator=(const ImageState&) = delete;
    };

    ImageProcessor(QLabel* imageLabel);
    ~ImageProcessor();

    std::mutex m_mutex;

    // Basic Operations
    void loadImage(const std::string& filePath);
    static bool isImageFile(const std::string& filePath);
    void processImage();
    QString revertImage();

    // Image Processing Functions
    void processYXAxis(double**& image, int height, int width, int linesToAvgY, int linesToAvgX);
    void applyMedianFilter(double**& image, int height, int width, int filterKernelSize);
    void applyHighPassFilter(double**& image, int height, int width);
    void applyEdgeEnhancement(double**& image, int height, int width, float strength);

    // Transformation Functions
    void rotateImage(int angle);
    void stretchImageY(double**& image, int& height, int width, float yStretchFactor);
    void stretchImageX(double**& image, int height, int& width, float xStretchFactor);
    void distortImage(double**& image, int height, int width, float distortionFactor, const std::string& direction);
    void addPadding(double**& image, int& height, int& width, int paddingSize);

    bool saveImage(const QString& filePath);

    void resetToOriginal();

    // Interlace Processing
    InterlaceProcessor::InterlacedResult processEnhancedInterlacedSections(
        InterlaceProcessor::StartPoint lowEnergyStart,
        InterlaceProcessor::StartPoint highEnergyStart,
        InterlaceProcessor::MergeMethod mergeMethod);

    InterlaceProcessor::InterlacedResult processEnhancedInterlacedSections(
        InterlaceProcessor::StartPoint lowEnergyStart,
        InterlaceProcessor::StartPoint highEnergyStart,
        const InterlaceProcessor::MergeParams& mergeParams);

    // Dark Line Operations
    void removeDarkLines(const DarkLineArray* lines, bool removeInObject, bool removeIsolated,
                         DarkLinePointerProcessor::RemovalMethod method = DarkLinePointerProcessor::RemovalMethod::NEIGHBOR_VALUES);
    void removeDarkLinesSequential(const DarkLineArray* lines, DarkLine** selectedLines, int selectedCount,
                                   bool removeInObject, bool removeIsolated,
                                   DarkLinePointerProcessor::RemovalMethod method = DarkLinePointerProcessor::RemovalMethod::NEIGHBOR_VALUES);
    void clearDetectedLines();

    // Format Conversion
    static double** matToDoublePtr(const cv::Mat& mat, int& height, int& width);
    CGData cropRegion(double** inputImage, int inputHeight, int inputWidth,
                      int left, int top, int right, int bottom);

    // Performance Metrics
    CLAHEProcessor::PerformanceMetrics getLastPerformanceMetrics() const;

    // State Management
    void saveCurrentState();
    void setLastAction(const QString& action, const QString& parameters = QString());
    QString getCurrentAction() const;
    ActionRecord getLastActionRecord() const;
    const DarkLineArray* getCurrentDarkLines() const { return m_currentDarkLines; }

    // Getters and Setters
    double** getFinalImage() const { return m_finalImage; }
    int getFinalImageHeight() const { return m_height; }
    int getFinalImageWidth() const { return m_width; }
    void updateAndSaveFinalImage(double** newImage, int height, int width, bool saveCurrentStateFlag = true);
    ZoomManager& getZoomManager() { return m_zoomManager; }

    bool validateState() const;

    // Memory Management
    static double** allocateImage(int height, int width);
    static void freeImage(double**& image, int height);
    static double** cloneImage(double** src, int height, int width);

    int getHistorySize() const { return imageHistory.size(); }
    int getActionHistorySize() const { return actionHistory.size(); }

    bool canUndo() const {
        bool result = !imageHistory.empty();
        qDebug() << "Checking canUndo() - History size:" << imageHistory.size() << "Result:" << result;
        return result;
    }
    void undo();

    void setOriginalImage(double** image, int height, int width) {
        // Free existing original image
        freeImage(m_originalImg, m_height);

        // Store new original image
        m_originalImg = cloneImage(image, height, width);
    }

    void clearHistory();

private:
    // Image Data
    double** m_imgData;
    double** m_originalImg;
    double** m_finalImage;
    int m_height;
    int m_width;

    // Dark Line Data
    DarkLineArray* m_currentDarkLines;
    std::vector<std::pair<int, int>> m_detectedLines;

    // State Management
    std::stack<ImageState> imageHistory;
    std::stack<ActionRecord> actionHistory;
    QString lastAction;

    // UI Elements
    QLabel* imageLabel;
    QRect selectedRegion;
    bool regionSelected;
    int rotationState;
    int kernelSize;

    // Processing Components
    ImageProcessingParams params;
    CLAHEProcessor claheProcessor;
    ZoomManager m_zoomManager;

    // Constants
    static constexpr int SEGMENT_WIDTH = 100;
    static constexpr int WIDTH_THRESHOLD = 50;

    // Private Helper Functions
    void saveImageState();
    bool isValidPixel(double pixel);
    ImageData convertToImageData(double** image, int height, int width) const;
    void updateFromImageData(const ImageData& imgData);

    int m_originalImageHeight;
    int m_originalImageWidth;

};

#endif // IMAGE_PROCESSOR_H
