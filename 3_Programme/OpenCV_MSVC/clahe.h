#pragma once

#define _USE_MATH_DEFINES

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/core/cuda.hpp>
#include <vector>
#include <cstdint>
#include <QDebug>
#include <chrono>

class CLAHEProcessor {
public:
    CLAHEProcessor();

    cv::Mat applyCLAHE(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    cv::Mat applyCLAHE_CPU(const cv::Mat& inputImage, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_GPU(std::vector<std::vector<uint16_t>>& finalImage, uint16_t threshold, double clipLimit, const cv::Size& tileSize);
    void applyThresholdCLAHE_CPU(std::vector<std::vector<uint16_t>>& finalImage, uint16_t threshold, double clipLimit, const cv::Size& tileSize);

    struct PerformanceMetrics {
        double processingTime;
        double totalTime;
        bool isCLAHE;
        void reset() {
            processingTime = totalTime = 0.0;
            isCLAHE = false;
        }
    };

    PerformanceMetrics getLastPerformanceMetrics() const { return metrics; }

    cv::Mat vectorToMat(const std::vector<std::vector<uint16_t>>& image);
    std::vector<std::vector<uint16_t>> matToVector(const cv::Mat& mat);

private:
    cv::Mat convertTo8Bit(const cv::Mat& input);
    cv::Mat convertTo16Bit(const cv::Mat& input);
    void preserveValueRange(const cv::Mat& original, cv::Mat& processed);

    PerformanceMetrics metrics;
    cv::Ptr<cv::CLAHE> clahe;
};
