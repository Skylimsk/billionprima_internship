#ifndef GRAPH_PROCESSOR_H
#define GRAPH_PROCESSOR_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include <atomic>
#include <memory>
#include <vector>
#include "third_party/qcustomplot/qcustomplot.h"
#include <opencv2/opencv.hpp>

class GraphProcessor : public QWidget {
    Q_OBJECT

public:
    explicit GraphProcessor(QWidget* parent = nullptr);
    ~GraphProcessor();

    // Main public interface with overloaded updateHistogram
    void updateHistogram(double** image, int height, int width);
    void updateHistogram(const std::vector<std::vector<uint16_t>>& image, int height, int width);
    void setCollapsed(bool collapsed);
    bool isCollapsed() const { return m_isCollapsed; }
    QCustomPlot* getHistogramPlot() const { return m_histogramPlot; }
    void updateClaheHistogram(double** image, int height, int width, double clipLimit);
    void clearClaheData();
    void toggleClaheVisibility();
    void clearHistogram();


protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void performDelayedUpdate();
    void showLargeHistogram();
    void updateLargeHistogram();
    void showTooltip(QMouseEvent* event);
    void resetZoom();

private:
    // Initialization functions
    void setupHistogramPlot();
    void setupLargeHistogramDialog();
    void initializeBuffers();

    // Histogram calculation related
    void calculateFullHistogram();
    bool needsUpdate(const std::vector<std::vector<uint16_t>>& image);
    uint64_t calculateChecksum(const std::vector<std::vector<uint16_t>>& image);
    void updateHistogramDisplay(const std::vector<std::atomic<int>>& histogram);

    // UI components
    QCustomPlot* m_histogramPlot;
    QDialog* m_histogramDialog;
    QCustomPlot* m_largeHistogramPlot;
    QLabel* m_tooltip;

    // State flags
    bool m_isCollapsed;
    int m_expandedHeight;
    bool m_updatePending;

    // Data cache
    struct HistogramCache {
        std::vector<int> data;
        size_t imageWidth;
        size_t imageHeight;
        uint64_t checksum;

        void clear() {
            data.clear();
            imageWidth = 0;
            imageHeight = 0;
            checksum = 0;
        }
    } m_cache;

    struct StatsLabels {
        QLabel* mean;
        QLabel* median;
        QLabel* stdDev;
        QLabel* mode;
    } m_statsLabels;

    // Helper function declarations
    int findNearestDataPoint(double x);
    void exportHistogramData();

    // Chart data
    QVector<double> m_currentBinCenters;
    QVector<double> m_currentHistogram;
    QCPRange m_currentXRange;
    QCPRange m_currentYRange;

    // Buffers
    std::vector<double> m_histogramBuffer;
    QVector<double> m_xData;
    QVector<double> m_yData;

    // Performance optimization related
    const std::vector<std::vector<uint16_t>>* m_imagePtr;
    QTimer* m_updateTimer;
    QElapsedTimer m_lastUpdate;
    static constexpr int UPDATE_INTERVAL_MS = 100;
    static constexpr int MIN_UPDATE_INTERVAL = 100;

    // Helper functions
    void scheduleUpdate();
    void executeUpdate();

    // Internal configuration constants
    static constexpr int DEFAULT_BIN_COUNT = 256;
    static constexpr int HISTOGRAM_BUFFER_SIZE = 65536;  // for 16-bit images
    static constexpr int MAX_TOOLTIP_DECIMALS = 2;

    // Plot related settings
    struct PlotSettings {
        static constexpr int FONT_SIZE_SMALL = 8;
        static constexpr int FONT_SIZE_MEDIUM = 9;
        static constexpr int FONT_SIZE_LARGE = 10;
        static constexpr int PLOT_MIN_WIDTH = 250;
        static constexpr int PLOT_MIN_HEIGHT = 350;
    };

    QVector<double> m_claheHistogram;  // Store CLAHE histogram data
    bool m_hasClaheData = false;       // Flag to track if CLAHE data exists
    double m_clipLimit = -1;           // Store CLAHE clip limit
    double m_maxFreq = 0.0;            // Store maximum frequency for visualization
    QPushButton* m_claheButton = nullptr;  // CLAHE button reference

    void setupClaheGraph();
    void updateClaheDisplay();

    void updateHistogramCache(const std::vector<std::atomic<int>>& histogram);
    void updateHistogramCommon(const std::vector<std::atomic<int>>& histogram);

    // Disable copy constructor and assignment operator
    GraphProcessor(const GraphProcessor&) = delete;
    GraphProcessor& operator=(const GraphProcessor&) = delete;
};

#endif // GRAPH_PROCESSOR_H
