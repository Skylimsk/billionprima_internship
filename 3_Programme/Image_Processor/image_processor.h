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

    void mergeHalves(double**& image, int height, int width);

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

    // Dark Line Operations
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

    void resetToOriginal();
    void clearHistory();

    void clearImageData() {
        if (m_finalImage) {
            for (int i = 0; i < m_height; i++) {
                if (m_finalImage[i]) {
                    free(m_finalImage[i]);
                }
            }
            free(m_finalImage);
            m_finalImage = nullptr;
        }

        if (m_originalImg) {
            for (int i = 0; i < m_height; i++) {
                if (m_originalImg[i]) {
                    free(m_originalImg[i]);
                }
            }
            free(m_originalImg);
            m_originalImg = nullptr;
        }

        m_height = 0;
        m_width = 0;
    }

    double** getOriginalImg() const { return m_originalImg; }

private:
    // Image Data
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

    // Processing Components
    CLAHEProcessor claheProcessor;
    ZoomManager m_zoomManager;

    // Private Helper Functions
    void saveImageState();
    ImageData convertToImageData(double** image, int height, int width) const;
    ImageData convertToImageData(double** image, int height, int width);
    double** convertFromImageData(const ImageData& imageData);

};

#endif // IMAGE_PROCESSOR_H
