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
    cv::Mat applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    cv::Mat applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_GPU(std::vector<std::vector<uint16_t>>& finalImage,
                                 uint16_t threshold, double clipLimit,
                                 const cv::Size& tileSize);
    void applyThresholdCLAHE_CPU(std::vector<std::vector<uint16_t>>& finalImage,
                                 uint16_t threshold, double clipLimit,
                                 const cv::Size& tileSize,
                                 const ThreadConfig& threadConfig = ThreadConfig());

    // Configuration and metrics
    void setThreadConfig(const ThreadConfig& config) { threadConfig = config; }
    ThreadConfig getThreadConfig() const { return threadConfig; }
    PerformanceMetrics getLastPerformanceMetrics() const { return metrics; }

    // Conversion utilities
    cv::Mat vectorToMat(const std::vector<std::vector<uint16_t>>& image);
    std::vector<std::vector<uint16_t>> matToVector(const cv::Mat& mat);

    // Logging utilities
    static void logMessage(const std::string& message, int threadId);

    void logThreadStart(int threadId, const std::string& function,
                        int startRow, int endRow);
    void logThreadComplete(int threadId, const std::string& function);

protected:
    // Helper methods for image processing
    void processImageChunk(cv::Mat& result, const cv::Mat& original,
                           const cv::Mat& claheResult, const cv::Mat& darkMask,
                           uint16_t threshold, int startRow, int endRow,
                           int threadId);

    void createDarkMask(const cv::Mat& input, cv::Mat& darkMask,
                        uint16_t threshold, float transitionRange,
                        int startRow, int endRow,
                        int threadId);

    // Conversion helpers
    cv::Mat convertTo8Bit(const cv::Mat& input);
    cv::Mat convertTo16Bit(const cv::Mat& input);
    void preserveValueRange(const cv::Mat& original, cv::Mat& processed);

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
