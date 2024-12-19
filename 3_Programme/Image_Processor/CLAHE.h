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
#include <vector>
#include <cstdint>
#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>
#include <string>

// Qt includes
#include <QDebug>

struct DarkPixelInfo {
    double value;
    int x;
    int y;
    bool isUsed;
};

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

    CLAHEProcessor();

    // Main processing functions
    void applyCLAHE(double** outputImage, double** inputImage, int height, int width,
                    double clipLimit, const cv::Size& tileSize);
    void applyCLAHE_CPU(double** outputImage, double** inputImage, int height, int width,
                        double clipLimit, const cv::Size& tileSize);

    // Modified main functions to use double**
    void applyThresholdCLAHE(double** image, int height, int width,
                             uint16_t threshold, double clipLimit,
                             const cv::Size& tileSize, bool afterNormalCLAHE = false);

    void applyThresholdCLAHE_CPU(double** finalImage, int height, int width,
                                 uint16_t threshold, double clipLimit,
                                 const cv::Size& tileSize, bool afterNormalCLAHE = false);

    void applyCombinedCLAHE(double** outputImage, double** inputImage,
                            int height, int width, double clipLimit,
                            const cv::Size& tileSize);

    void applyCombinedCLAHE_CPU(double** outputImage, double** inputImage,
                                int height, int width, double clipLimit,
                                const cv::Size& tileSize);

    // Memory management helpers
    static double** allocateImageBuffer(int height, int width);
    static void deallocateImageBuffer(double** buffer, int height);

    // Conversion utilities
    cv::Mat doubleToMat(double** image, int height, int width);
    void matToDouble(const cv::Mat& mat, double** image);

    // Configuration and metrics
    void setThreadConfig(const ThreadConfig& config) { threadConfig = config; }
    ThreadConfig getThreadConfig() const { return threadConfig; }
    PerformanceMetrics getLastPerformanceMetrics() const { return metrics; }

    // Logging utilities
    static void logMessage(const std::string& message, int threadId);
    void logThreadStart(int threadId, const std::string& function, int startRow, int endRow);
    void logThreadComplete(int threadId, const std::string& function);

protected:
    // Member variables
    PerformanceMetrics metrics;
    ThreadConfig threadConfig;
    ThreadProgress threadProgress;
    cv::Ptr<cv::CLAHE> clahe;
    std::mutex processingMutex;
    static std::mutex logMutex;

};

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
