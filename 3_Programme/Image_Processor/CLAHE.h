#pragma once

#define _USE_MATH_DEFINES

#include "ThreadLogger.h"

// OpenCV includes
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/core/cuda.hpp>

// Standard library includes
#include <cstdint>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

// Qt includes
#include <QDebug>

// Stores information about dark pixels including position and processing state
struct DarkPixelInfo {
    double value;
    int x;
    int y;
    bool isUsed;
};

class CLAHEProcessor {
public:
    // Configuration structure for managing thread parameters
    struct ThreadConfig {
        unsigned int numThreads;
        unsigned int chunkSize;

        ThreadConfig(unsigned int threads = std::thread::hardware_concurrency(),
                     unsigned int chunk = 64)
            : numThreads(threads), chunkSize(chunk) {}
    };

    // Tracks progress of multi-threaded operations using atomic counters
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

    // Stores performance metrics for CLAHE operations
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

    // Initialize CLAHE processor with default settings
    CLAHEProcessor();

    // GPU-accelerated CLAHE processing
    void applyCLAHE(double** outputImage, double** inputImage, int height, int width,
                    double clipLimit, const cv::Size& tileSize);

    // Multi-threaded CPU implementation of CLAHE
    void applyCLAHE_CPU(double** outputImage, double** inputImage, int height, int width,
                        double clipLimit, const cv::Size& tileSize);

    // GPU-accelerated threshold-based CLAHE for dark region enhancement
    void applyThresholdCLAHE(double** image, int height, int width,
                             uint16_t threshold, double clipLimit,
                             const cv::Size& tileSize, bool afterNormalCLAHE = false);

    // CPU version of threshold-based CLAHE processing
    void applyThresholdCLAHE_CPU(double** finalImage, int height, int width,
                                 uint16_t threshold, double clipLimit,
                                 const cv::Size& tileSize, bool afterNormalCLAHE = false);

    // Combined CLAHE processing with GPU acceleration
    void applyCombinedCLAHE(double** outputImage, double** inputImage,
                            int height, int width, double clipLimit,
                            const cv::Size& tileSize);

    // CPU version of combined CLAHE processing
    void applyCombinedCLAHE_CPU(double** outputImage, double** inputImage,
                                int height, int width, double clipLimit,
                                const cv::Size& tileSize);

    // Memory allocation helper for image buffers
    static double** allocateImageBuffer(int height, int width);

    // Memory deallocation helper for image buffers
    static void deallocateImageBuffer(double** buffer, int height);

    // Convert double array to OpenCV Mat format
    cv::Mat doubleToMat(double** image, int height, int width);

    // Convert OpenCV Mat to double array format
    void matToDouble(const cv::Mat& mat, double** image);

    // Thread configuration management
    void setThreadConfig(const ThreadConfig& config) { threadConfig = config; }
    ThreadConfig getThreadConfig() const { return threadConfig; }

    // Access last operation's performance metrics
    PerformanceMetrics getLastPerformanceMetrics() const { return metrics; }

    // Thread-safe logging utilities
    static void logMessage(const std::string& message, int threadId);
    void logThreadStart(int threadId, const std::string& function, int startRow, int endRow);
    void logThreadComplete(int threadId, const std::string& function);

protected:
    PerformanceMetrics metrics;
    ThreadConfig threadConfig;
    ThreadProgress threadProgress;
    cv::Ptr<cv::CLAHE> clahe;
    std::mutex processingMutex;
    static std::mutex logMutex;
};

// RAII-style timer for measuring operation duration
class ScopedTimer {
public:
    ScopedTimer(const std::string& operationName, int threadId)
        : operationName(operationName)
        , threadId(threadId)
        , startTime(std::chrono::high_resolution_clock::now()) {
        CLAHEProcessor::logMessage("Starting: " + operationName, threadId);
    }

    ~ScopedTimer() {
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            endTime - startTime).count();
        CLAHEProcessor::logMessage(
            operationName + " completed in " + std::to_string(duration) + "ms",
            threadId
            );
    }

private:
    std::string operationName;
    int threadId;
    std::chrono::high_resolution_clock::time_point startTime;
};
