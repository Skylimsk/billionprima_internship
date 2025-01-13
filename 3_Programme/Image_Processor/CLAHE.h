#pragma once

#define _USE_MATH_DEFINES

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/core/cuda.hpp>
#include <cstdint>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>
#include <QDebug>

class CLAHEProcessor {
public:
    // Configuration structure for thread management
    struct ThreadConfig {
        unsigned int numThreads;
        unsigned int chunkSize;

        ThreadConfig(unsigned int threads = std::thread::hardware_concurrency(),
                     unsigned int chunk = 64)
            : numThreads(threads), chunkSize(chunk) {}
    };

    // Progress tracking for multi-threaded operations
    struct ThreadProgress {
        std::atomic<int> completedRows{0};
        std::atomic<int> totalRows{0};
        std::string currentOperation;

        void reset() {
            completedRows = 0;
            totalRows = 0;
            currentOperation.clear();
        }
    };

    // Performance metrics tracking
    struct PerformanceMetrics {
        double processingTime;
        double totalTime;
        bool isCLAHE;
        unsigned int threadsUsed;

        void reset() {
            processingTime = totalTime = 0.0;
            isCLAHE = false;
            threadsUsed = 1;
        }
    };

    // Constructor
    CLAHEProcessor();

    // Standard CLAHE operations
    void applyCLAHE(double** outputImage, double** inputImage,
                    int height, int width, double clipLimit, const cv::Size& tileSize);
    void applyCLAHE_CPU(double** outputImage, double** inputImage,
                        int height, int width, double clipLimit, const cv::Size& tileSize);
    void applyCLAHE(double** outputImage, const uint32_t* inputBuffer,
                    int height, int width, double clipLimit, const cv::Size& tileSize);
    void applyCLAHE_CPU(double** outputImage, const uint32_t* inputBuffer,
                        int height, int width, double clipLimit, const cv::Size& tileSize);

    // Threshold CLAHE operations
    void applyThresholdCLAHE(double** finalImage, int height, int width,
                             uint16_t threshold, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_CPU(double** finalImage, int height, int width,
                                 uint16_t threshold, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE(double** finalImage, const uint32_t* inputBuffer,
                             int height, int width, uint16_t threshold,
                             double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_CPU(double** finalImage, const uint32_t* inputBuffer,
                                 int height, int width, uint16_t threshold,
                                 double clipLimit, const cv::Size& tileSize);

    // Combined CLAHE operations
    void applyCombinedCLAHE(double** outputImage, double** inputImage,
                            int height, int width, double clipLimit, const cv::Size& tileSize);
    void applyCombinedCLAHE_CPU(double** outputImage, double** inputImage,
                                int height, int width, double clipLimit, const cv::Size& tileSize);
    void applyCombinedCLAHE(double** outputImage, const uint32_t* inputBuffer,
                            int height, int width, double clipLimit, const cv::Size& tileSize);
    void applyCombinedCLAHE_CPU(double** outputImage, const uint32_t* inputBuffer,
                                int height, int width, double clipLimit, const cv::Size& tileSize);

    // Utility functions
    static double** allocateImageBuffer(int height, int width);
    static void deallocateImageBuffer(double** buffer, int height);
    cv::Mat doubleToMat(double** image, int height, int width);
    void matToDouble(const cv::Mat& mat, double** image);

    // Configuration management
    void setThreadConfig(const ThreadConfig& config) { threadConfig = config; }
    ThreadConfig getThreadConfig() const { return threadConfig; }
    PerformanceMetrics getLastPerformanceMetrics() const { return metrics; }

protected:

    // Logging utilities
    static void logMessage(const std::string& message, int threadId = -1);
    void logThreadStart(int threadId, const std::string& function, int startRow, int endRow);
    void logThreadComplete(int threadId, const std::string& function);

private:
    PerformanceMetrics metrics;
    ThreadConfig threadConfig;
    ThreadProgress threadProgress;
    cv::Ptr<cv::CLAHE> clahe;
    std::mutex processingMutex;
    static std::mutex logMutex;
};
