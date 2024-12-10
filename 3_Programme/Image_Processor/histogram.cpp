#include "histogram.h"
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QMessageBox>
#include <QApplication>
#include <QScreen>
#include <algorithm>
#include <execution>

Histogram::Histogram(QWidget* parent)
    : QWidget(parent)
    , m_histogramPlot(nullptr)
    , m_histogramDialog(nullptr)
    , m_largeHistogramPlot(nullptr)
    , m_tooltip(nullptr)
    , m_isCollapsed(false)
    , m_expandedHeight(PlotSettings::PLOT_MIN_HEIGHT)
    , m_updatePending(false)
    , m_imagePtr(nullptr)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    initializeBuffers();
    setupHistogramPlot();
    setCollapsed(false);  // Start expanded by default

    // 初始化更新定时器
    m_updateTimer = new QTimer(this);
    m_updateTimer->setSingleShot(true);
    m_updateTimer->setInterval(UPDATE_INTERVAL_MS);
    connect(m_updateTimer, &QTimer::timeout, this, &Histogram::performDelayedUpdate);

    // 初始化时间追踪器
    m_lastUpdate.start();
}

Histogram::~Histogram()
{
    if (m_histogramDialog) {
        m_histogramDialog->hide();
        m_histogramDialog->deleteLater();
    }
    if (m_tooltip) {
        delete m_tooltip;
    }
}

void Histogram::initializeBuffers()
{
    // 预分配所有缓冲区
    m_histogramBuffer.resize(HISTOGRAM_BUFFER_SIZE, 0.0);
    m_xData.resize(DEFAULT_BIN_COUNT);
    m_yData.resize(DEFAULT_BIN_COUNT);

    // 预计算 X 轴数据
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        m_xData[i] = i * (65536.0 / DEFAULT_BIN_COUNT);
    }

    // 初始化缓存
    m_cache.clear();
}

void Histogram::setupHistogramPlot() {
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_histogramPlot = new QCustomPlot(this);
    m_histogramPlot->setMinimumSize(PlotSettings::PLOT_MIN_WIDTH, PlotSettings::PLOT_MIN_HEIGHT);
    layout->addWidget(m_histogramPlot);

    // Set up axes
    m_histogramPlot->xAxis->setLabel("Pixel Value");
    m_histogramPlot->yAxis->setLabel("Relative Frequency");
    m_histogramPlot->setBackground(Qt::white);

    // Original histogram (blue)
    m_histogramPlot->addGraph();
    QPen originalPen;
    originalPen.setColor(QColor(65, 105, 225));  // Royal Blue
    originalPen.setWidth(2);
    m_histogramPlot->graph(0)->setPen(originalPen);
    m_histogramPlot->graph(0)->setBrush(QBrush(QColor(65, 105, 225, 50)));

    // CLAHE histogram (green)
    m_histogramPlot->addGraph();
    QPen clahePen;
    clahePen.setColor(QColor(46, 139, 87));  // Sea Green
    clahePen.setWidth(2);
    m_histogramPlot->graph(1)->setPen(clahePen);
    m_histogramPlot->graph(1)->setBrush(QBrush(QColor(46, 139, 87, 30)));

    // Clip limit line (red dashed)
    m_histogramPlot->addGraph();
    QPen clipLimitPen;
    clipLimitPen.setColor(Qt::red);
    clipLimitPen.setWidth(2);
    clipLimitPen.setStyle(Qt::DashLine);
    m_histogramPlot->graph(2)->setPen(clipLimitPen);

    // Enable antialiasing and interactions
    m_histogramPlot->setAntialiasedElements(QCP::aeAll);
    m_histogramPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Font settings
    QFont tickFont = QApplication::font();
    tickFont.setPointSize(PlotSettings::FONT_SIZE_SMALL);
    m_histogramPlot->xAxis->setTickLabelFont(tickFont);
    m_histogramPlot->yAxis->setTickLabelFont(tickFont);

    QFont labelFont = QApplication::font();
    labelFont.setPointSize(PlotSettings::FONT_SIZE_MEDIUM);
    m_histogramPlot->xAxis->setLabelFont(labelFont);
    m_histogramPlot->yAxis->setLabelFont(labelFont);

    // Add tooltip support
    m_histogramPlot->setMouseTracking(true);
    connect(m_histogramPlot, &QCustomPlot::mouseMove, this, &Histogram::showTooltip);
}


void Histogram::setCollapsed(bool collapsed)
{
    if (m_isCollapsed == collapsed) return;

    m_isCollapsed = collapsed;
    if (m_isCollapsed) {
        m_expandedHeight = height();
        setFixedHeight(0);
        m_histogramPlot->hide();
    } else {
        setFixedHeight(m_expandedHeight);
        m_histogramPlot->show();
        m_histogramPlot->replot(QCustomPlot::rpQueuedReplot);
    }

    updateGeometry();
    if (parentWidget()) {
        parentWidget()->adjustSize();
    }
}

void Histogram::scheduleUpdate()
{
    if (!m_updatePending && m_lastUpdate.elapsed() > MIN_UPDATE_INTERVAL) {
        m_updatePending = true;
        QTimer::singleShot(0, this, &Histogram::executeUpdate);
    }
}

void Histogram::executeUpdate()
{
    m_updatePending = false;
    m_lastUpdate.restart();
    performDelayedUpdate();
}

bool Histogram::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_largeHistogramPlot && event->type() == QEvent::Leave) {
        if (m_tooltip) {
            m_tooltip->hide();
        }
        return true;
    }
    return QWidget::eventFilter(obj, event);
}

void Histogram::updateHistogram(const std::vector<std::vector<uint16_t>>& image, int height, int width) {
    if (m_isCollapsed || image.empty() || height <= 0 || width <= 0) return;

    // Initialize histogram bins
    std::vector<std::atomic<int>> histogram(HISTOGRAM_BUFFER_SIZE);
    for (auto& bin : histogram) {
        bin.store(0, std::memory_order_relaxed);
    }

    // Calculate histogram in parallel
#pragma omp parallel for collapse(2)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            uint16_t val = image[y][x];
            histogram[val].fetch_add(1, std::memory_order_relaxed);
        }
    }

    // Find maximum count for normalization
    double maxCount = 0;
    for (const auto& bin : histogram) {
        maxCount = std::max(maxCount, static_cast<double>(bin.load(std::memory_order_relaxed)));
    }

    // Prepare data for plotting
    m_currentHistogram.resize(DEFAULT_BIN_COUNT);
    m_xData.resize(DEFAULT_BIN_COUNT);
    double binWidth = HISTOGRAM_BUFFER_SIZE / static_cast<double>(DEFAULT_BIN_COUNT);

    // Apply smoothing
    const int smoothingWindow = 5;
    std::vector<double> rawValues(DEFAULT_BIN_COUNT);

    // First pass: collect raw values
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double binSum = 0;
        int startBin = static_cast<int>(i * binWidth);
        int endBin = static_cast<int>((i + 1) * binWidth);

        for (int j = startBin; j < endBin; ++j) {
            binSum += histogram[j].load(std::memory_order_relaxed);
        }
        rawValues[i] = binSum / maxCount;
        m_xData[i] = i * binWidth;
    }

    // Second pass: apply Gaussian smoothing
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double smoothedValue = 0;
        double weightSum = 0;

        for (int j = -smoothingWindow; j <= smoothingWindow; ++j) {
            int idx = i + j;
            if (idx >= 0 && idx < DEFAULT_BIN_COUNT) {
                double weight = std::exp(-0.5 * (j * j) / (smoothingWindow * smoothingWindow));
                smoothedValue += rawValues[idx] * weight;
                weightSum += weight;
            }
        }

        m_currentHistogram[i] = smoothedValue / weightSum;
    }

    // Update display
    updateHistogramDisplay(histogram);

    // Update cache
    updateHistogramCache(histogram);
}

void Histogram::updateHistogram(double** image, int height, int width) {
    if (m_isCollapsed || !image || height <= 0 || width <= 0) return;

    // Initialize histogram bins
    std::vector<std::atomic<int>> histogram(HISTOGRAM_BUFFER_SIZE);
    for (auto& bin : histogram) {
        bin.store(0, std::memory_order_relaxed);
    }

    // Calculate histogram in parallel
#pragma omp parallel for collapse(2)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int val = static_cast<int>(std::clamp(image[y][x], 0.0, 65535.0));
            histogram[val].fetch_add(1, std::memory_order_relaxed);
        }
    }

    // Find maximum count for normalization
    double maxCount = 0;
    for (const auto& bin : histogram) {
        maxCount = std::max(maxCount, static_cast<double>(bin.load(std::memory_order_relaxed)));
    }

    // Prepare data for plotting
    m_currentHistogram.resize(DEFAULT_BIN_COUNT);
    m_xData.resize(DEFAULT_BIN_COUNT);
    double binWidth = HISTOGRAM_BUFFER_SIZE / static_cast<double>(DEFAULT_BIN_COUNT);

    // Apply smoothing
    const int smoothingWindow = 5;
    std::vector<double> rawValues(DEFAULT_BIN_COUNT);

    // First pass: collect raw values
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double binSum = 0;
        int startBin = static_cast<int>(i * binWidth);
        int endBin = static_cast<int>((i + 1) * binWidth);

        for (int j = startBin; j < endBin; ++j) {
            binSum += histogram[j].load(std::memory_order_relaxed);
        }
        rawValues[i] = binSum / maxCount;
        m_xData[i] = i * binWidth;
    }

    // Second pass: apply Gaussian smoothing
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double smoothedValue = 0;
        double weightSum = 0;

        for (int j = -smoothingWindow; j <= smoothingWindow; ++j) {
            int idx = i + j;
            if (idx >= 0 && idx < DEFAULT_BIN_COUNT) {
                double weight = std::exp(-0.5 * (j * j) / (smoothingWindow * smoothingWindow));
                smoothedValue += rawValues[idx] * weight;
                weightSum += weight;
            }
        }

        m_currentHistogram[i] = smoothedValue / weightSum;
    }

    // Update display
    updateHistogramDisplay(histogram);

    // Update cache
    updateHistogramCache(histogram);
}

void Histogram::performDelayedUpdate()
{
    if (!m_imagePtr || m_imagePtr->empty()) return;

    calculateFullHistogram();
}

void Histogram::calculateFullHistogram()
{
    if (!m_imagePtr || m_imagePtr->empty()) return;

    const auto& image = *m_imagePtr;
    int rows = image.size();
    int cols = image[0].size();

    // Create atomic counter array
    std::vector<std::atomic<int>> histogram(HISTOGRAM_BUFFER_SIZE);
    for (auto& bin : histogram) {
        bin.store(0, std::memory_order_relaxed);
    }

    // Calculate histogram
#pragma omp parallel for collapse(2)
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            histogram[image[y][x]].fetch_add(1, std::memory_order_relaxed);
        }
    }

    // Find maximum count
    double maxCount = 0;
    for (const auto& bin : histogram) {
        maxCount = std::max(maxCount, static_cast<double>(bin.load(std::memory_order_relaxed)));
    }

    // Apply smoothing and create display data
    const int smoothingWindow = 5;  // Adjust this value to control smoothing amount
    m_currentHistogram.resize(DEFAULT_BIN_COUNT);
    m_xData.resize(DEFAULT_BIN_COUNT);
    double binWidth = HISTOGRAM_BUFFER_SIZE / static_cast<double>(DEFAULT_BIN_COUNT);

    // First pass: collect raw values
    std::vector<double> rawValues(DEFAULT_BIN_COUNT);
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double binSum = 0;
        int startBin = static_cast<int>(i * binWidth);
        int endBin = static_cast<int>((i + 1) * binWidth);

        for (int j = startBin; j < endBin; ++j) {
            binSum += histogram[j].load(std::memory_order_relaxed);
        }
        rawValues[i] = binSum / maxCount;
        m_xData[i] = i * binWidth;  // Store x coordinates
    }

    // Second pass: apply Gaussian smoothing
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double smoothedValue = 0;
        double weightSum = 0;

        for (int j = -smoothingWindow; j <= smoothingWindow; ++j) {
            int idx = i + j;
            if (idx >= 0 && idx < DEFAULT_BIN_COUNT) {
                // Gaussian weight
                double weight = std::exp(-0.5 * (j * j) / (smoothingWindow * smoothingWindow));
                smoothedValue += rawValues[idx] * weight;
                weightSum += weight;
            }
        }

        m_currentHistogram[i] = smoothedValue / weightSum;
    }

    // Update cache and display
    updateHistogramCache(histogram);
    updateHistogramDisplay(histogram);
}

void Histogram::updateHistogramDisplay(const std::vector<std::atomic<int>>& histogram) {
    if (!m_histogramPlot) return;

    // Update the main curve
    m_histogramPlot->graph(0)->setData(m_xData, m_currentHistogram);

    // Find maximum value
    double maxValue = 0;
    for (const auto& value : m_currentHistogram) {
        maxValue = std::max(maxValue, value);
    }

    // Set axis ranges with padding
    m_histogramPlot->xAxis->setRange(0, 65535);
    double maxYRange = std::max(maxValue, m_clipLimit > 0 ? m_clipLimit : 0.0) * 1.1;
    m_histogramPlot->yAxis->setRange(0, maxYRange);

    // Store ranges
    m_currentXRange = m_histogramPlot->xAxis->range();
    m_currentYRange = m_histogramPlot->yAxis->range();

    // Replot with queued replot for better performance
    m_histogramPlot->replot(QCustomPlot::rpQueuedReplot);

    // Update large histogram if visible
    if (m_largeHistogramPlot && m_histogramDialog && m_histogramDialog->isVisible()) {
        updateLargeHistogram();
    }
}

bool Histogram::needsUpdate(const std::vector<std::vector<uint16_t>>& image)
{
    if (m_cache.imageWidth != image[0].size() ||
        m_cache.imageHeight != image.size()) {
        return true;
    }

    return calculateChecksum(image) != m_cache.checksum;
}

uint64_t Histogram::calculateChecksum(const std::vector<std::vector<uint16_t>>& image)
{
    uint64_t checksum = 0;
    for (const auto& row : image) {
        // 使用简单的采样来加速计算
        for (size_t i = 0; i < row.size(); i += 16) {
            checksum = (checksum * 131) + row[i];
        }
    }
    return checksum;
}

void Histogram::showLargeHistogram()
{
    if (!m_histogramPlot || !m_histogramPlot->graph(0)) return;

    if (!m_histogramDialog) {
        m_histogramDialog = new QDialog(this);
        m_histogramDialog->setWindowTitle("Detailed Histogram View");
        m_histogramDialog->resize(800, 600);
        setupLargeHistogramDialog();
    }

    updateLargeHistogram();
    m_histogramDialog->show();
    m_histogramDialog->raise();
    m_histogramDialog->activateWindow();

    // 将窗口移动到屏幕中心
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    m_histogramDialog->move(
        screenGeometry.center() - m_histogramDialog->rect().center()
        );
}

void Histogram::setupLargeHistogramDialog()
{
    QVBoxLayout* dialogLayout = new QVBoxLayout(m_histogramDialog);
    dialogLayout->setSpacing(8);

    // 创建大直方图绘图窗口
    m_largeHistogramPlot = new QCustomPlot(m_histogramDialog);
    m_largeHistogramPlot->setMinimumSize(700, 500);

    // 配置绘图样式
    m_largeHistogramPlot->xAxis->setLabel("Pixel Value");
    m_largeHistogramPlot->yAxis->setLabel("Frequency");
    m_largeHistogramPlot->setBackground(Qt::white);

    // 配置图表样式
    m_largeHistogramPlot->addGraph();
    QPen graphPen;
    graphPen.setColor(QColor(65, 105, 225));  // Royal Blue
    graphPen.setWidth(2);
    m_largeHistogramPlot->graph(0)->setPen(graphPen);
    m_largeHistogramPlot->graph(0)->setBrush(QBrush(QColor(65, 105, 225, 50)));

    // 启用鼠标交互
    m_largeHistogramPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_largeHistogramPlot->setMouseTracking(true);
    m_largeHistogramPlot->installEventFilter(this);

    // 创建工具提示标签
    m_tooltip = new QLabel(m_largeHistogramPlot);
    m_tooltip->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(255, 255, 255, 0.95);"
        "   border: 1px solid #999;"
        "   border-radius: 4px;"
        "   padding: 4px 8px;"
        "   font-size: 10pt;"
        "   color: #333;"
        "}"
        );
    m_tooltip->hide();

    // 创建统计信息面板
    QFrame* statsFrame = new QFrame(m_histogramDialog);
    statsFrame->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
    QHBoxLayout* statsLayout = new QHBoxLayout(statsFrame);

    m_statsLabels.mean = new QLabel(statsFrame);
    m_statsLabels.median = new QLabel(statsFrame);
    m_statsLabels.stdDev = new QLabel(statsFrame);
    m_statsLabels.mode = new QLabel(statsFrame);

    statsLayout->addWidget(m_statsLabels.mean);
    statsLayout->addWidget(m_statsLabels.median);
    statsLayout->addWidget(m_statsLabels.stdDev);
    statsLayout->addWidget(m_statsLabels.mode);

    // 创建按钮面板
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // 缩放控制按钮
    QPushButton* resetZoomButton = new QPushButton("Reset Zoom", m_histogramDialog);
    resetZoomButton->setFixedWidth(100);
    connect(resetZoomButton, &QPushButton::clicked, this, &Histogram::resetZoom);

    // 导出按钮
    QPushButton* exportButton = new QPushButton("Export Data", m_histogramDialog);
    exportButton->setFixedWidth(100);
    connect(exportButton, &QPushButton::clicked, this, &Histogram::exportHistogramData);

    // 关闭按钮
    QPushButton* closeButton = new QPushButton("Close", m_histogramDialog);
    closeButton->setFixedWidth(100);
    connect(closeButton, &QPushButton::clicked, m_histogramDialog, &QDialog::close);

    buttonLayout->addStretch();
    buttonLayout->addWidget(resetZoomButton);
    buttonLayout->addWidget(exportButton);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    // 添加所有组件到对话框布局
    dialogLayout->addWidget(m_largeHistogramPlot);
    dialogLayout->addWidget(statsFrame);
    dialogLayout->addLayout(buttonLayout);

    // 连接鼠标移动信号
    connect(m_largeHistogramPlot, &QCustomPlot::mouseMove, this, &Histogram::showTooltip);
}

void Histogram::showTooltip(QMouseEvent* event)
{
    if (!m_largeHistogramPlot || !m_tooltip) return;

    double x = m_largeHistogramPlot->xAxis->pixelToCoord(event->pos().x());
    double y = m_largeHistogramPlot->yAxis->pixelToCoord(event->pos().y());

    // Find nearest data point
    int index = findNearestDataPoint(x);
    if (index >= 0 && index < m_currentHistogram.size()) {
        // Get pixel value and relative height
        double pixelValue = m_xData[index];
        double relativeHeight = m_currentHistogram[index];

        // Format tooltip text showing relative to clip limit
        QString tooltipText = QString(
                                  "Pixel Value: %1\n"
                                  "Relative Height: %2x"
                                  ).arg(qRound(pixelValue))
                                  .arg(relativeHeight, 0, 'f', 3);  // Show as multiplier of clip limit

        // Calculate tooltip position
        QPoint pos = event->pos() + QPoint(10, 10);

        // Ensure tooltip stays within plot area
        if (pos.x() + m_tooltip->width() > m_largeHistogramPlot->width()) {
            pos.setX(event->pos().x() - m_tooltip->width() - 10);
        }
        if (pos.y() + m_tooltip->height() > m_largeHistogramPlot->height()) {
            pos.setY(event->pos().y() - m_tooltip->height() - 10);
        }

        m_tooltip->setText(tooltipText);
        m_tooltip->adjustSize();
        m_tooltip->move(pos);
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
}


void Histogram::resetZoom()
{
    if (m_largeHistogramPlot) {
        m_largeHistogramPlot->xAxis->setRange(m_currentXRange);
        m_largeHistogramPlot->yAxis->setRange(m_currentYRange);
        m_largeHistogramPlot->replot();
    }
}

int Histogram::findNearestDataPoint(double x)
{
    if (m_xData.isEmpty()) return -1;

    return std::lower_bound(m_xData.begin(), m_xData.end(), x) - m_xData.begin();
}

void Histogram::exportHistogramData()
{
    QString fileName = QFileDialog::getSaveFileName(
        m_histogramDialog,
        "Export Histogram Data",
        QString(),
        "CSV files (*.csv);;All files (*.*)"
        );

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(
            m_histogramDialog,
            "Export Error",
            "Failed to open file for writing."
            );
        return;
    }

    QTextStream out(&file);
    out << "Pixel Value,Frequency\n";

    for (int i = 0; i < m_xData.size(); ++i) {
        out << QString("%1,%2\n")
        .arg(m_xData[i])
            .arg(m_currentHistogram[i], 0, 'f', 6);
    }

    file.close();
}

void Histogram::updateLargeHistogram()
{
    if (!m_largeHistogramPlot || !m_histogramPlot->graph(0)) return;

    // 复制数据到大直方图
    m_largeHistogramPlot->graph(0)->setData(m_xData, m_currentHistogram);

    // 设置坐标轴范围
    m_largeHistogramPlot->xAxis->setRange(m_currentXRange);
    m_largeHistogramPlot->yAxis->setRange(m_currentYRange);

    // 使用队列重绘以提高性能
    m_largeHistogramPlot->replot(QCustomPlot::rpQueuedReplot);
}

void Histogram::setupClaheGraph() {
    if (!m_histogramPlot) return;

    // Add a second graph for CLAHE histogram
    m_histogramPlot->addGraph();
    QPen clahePen;
    clahePen.setColor(QColor(46, 139, 87));  // Sea Green
    clahePen.setWidth(1);
    m_histogramPlot->graph(1)->setPen(clahePen);
    m_histogramPlot->graph(1)->setVisible(false);

    // Style for clip limit line
    m_histogramPlot->addGraph();
    QPen clipLimitPen;
    clipLimitPen.setColor(Qt::red);
    clipLimitPen.setWidth(2);
    clipLimitPen.setStyle(Qt::DashLine);
    m_histogramPlot->graph(2)->setPen(clipLimitPen);
    m_histogramPlot->graph(2)->setVisible(false);
}

void Histogram::updateClaheHistogram(double** image, int height, int width, double clipLimit) {
    if (!image || height <= 0 || width <= 0 || m_isCollapsed) return;

    m_clipLimit = clipLimit;

    // Create atomic counters for thread-safe histogram calculation
    std::vector<std::atomic<int>> histogram(HISTOGRAM_BUFFER_SIZE);
    for (auto& bin : histogram) {
        bin.store(0, std::memory_order_relaxed);
    }

    // Calculate histogram in parallel
#pragma omp parallel for collapse(2)
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            auto val = static_cast<int>(std::clamp(image[y][x], 0.0, 65535.0));
            histogram[val].fetch_add(1, std::memory_order_relaxed);
        }
    }

    // Find maximum count for normalization
    double maxCount = 0;
    for (const auto& bin : histogram) {
        maxCount = std::max(maxCount, static_cast<double>(bin.load(std::memory_order_relaxed)));
    }

    // Calculate clip limit threshold
    double clipThreshold = clipLimit * maxCount;
    double redistributeCount = 0;

    // Calculate excess for redistribution
    for (const auto& bin : histogram) {
        double count = bin.load(std::memory_order_relaxed);
        if (count > clipThreshold) {
            redistributeCount += count - clipThreshold;
        }
    }

    // Redistribute excess
    double redistributePerBin = redistributeCount / DEFAULT_BIN_COUNT;

    // Prepare histogram data
    QVector<double> xData(DEFAULT_BIN_COUNT), yData(DEFAULT_BIN_COUNT), claheData(DEFAULT_BIN_COUNT);
    double binWidth = HISTOGRAM_BUFFER_SIZE / static_cast<double>(DEFAULT_BIN_COUNT);

    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        xData[i] = i * binWidth;
        double sum = 0;
        double claheSum = 0;
        int startBin = static_cast<int>(i * binWidth);
        int endBin = static_cast<int>((i + 1) * binWidth);

        for (int j = startBin; j < endBin; ++j) {
            double count = histogram[j].load(std::memory_order_relaxed);
            sum += count;
            claheSum += std::min(count, clipThreshold);
        }

        yData[i] = sum / maxCount;
        claheData[i] = (claheSum + redistributePerBin) / maxCount;
    }

    // Update plots
    m_histogramPlot->graph(0)->setData(xData, yData);
    m_histogramPlot->graph(1)->setData(xData, claheData);
    m_histogramPlot->graph(1)->setVisible(true);

    // Update clip limit line
    QVector<double> clipX = {0, 65535};
    QVector<double> clipY = {clipLimit, clipLimit};
    m_histogramPlot->graph(2)->setData(clipX, clipY);
    m_histogramPlot->graph(2)->setVisible(true);

    // Set axis ranges
    double maxY = *std::max_element(yData.begin(), yData.end());
    maxY = std::max(maxY, clipLimit) * 1.1; // Add 10% padding
    m_histogramPlot->xAxis->setRange(0, 65535);
    m_histogramPlot->yAxis->setRange(0, maxY);

    // Update stored data
    m_hasClaheData = true;
    m_claheHistogram = claheData;

    // Replot
    m_histogramPlot->replot(QCustomPlot::rpQueuedReplot);
}

void Histogram::updateClaheDisplay() {
    if (!m_histogramPlot || !m_hasClaheData) return;

    // Update CLAHE histogram
    m_histogramPlot->graph(1)->setVisible(true);
    m_histogramPlot->graph(1)->setData(m_xData, m_claheHistogram);

    // Find maximum frequency between original and CLAHE histograms
    m_maxFreq = 0.0;
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        m_maxFreq = std::max(m_maxFreq, m_currentHistogram[i]);
        if (m_hasClaheData) {
            m_maxFreq = std::max(m_maxFreq, m_claheHistogram[i]);
        }
    }

    // Update clip limit line if available
    if (m_clipLimit > 0) {
        m_histogramPlot->graph(2)->setVisible(true);
        QVector<double> clipX = {0, 65535};
        QVector<double> clipY = {m_clipLimit, m_clipLimit};
        m_histogramPlot->graph(2)->setData(clipX, clipY);
    }

    m_histogramPlot->replot(QCustomPlot::rpQueuedReplot);

    // Update large histogram if visible
    if (m_largeHistogramPlot && m_histogramDialog && m_histogramDialog->isVisible()) {
        // Add and update CLAHE data for large histogram
        if (m_largeHistogramPlot->graphCount() < 3) {
            m_largeHistogramPlot->addGraph();
            m_largeHistogramPlot->graph(1)->setPen(QPen(QColor(46, 139, 87), 2));
            m_largeHistogramPlot->addGraph();
            m_largeHistogramPlot->graph(2)->setPen(QPen(Qt::red, 2, Qt::DashLine));
        }

        m_largeHistogramPlot->graph(1)->setVisible(true);
        m_largeHistogramPlot->graph(1)->setData(m_xData, m_claheHistogram);

        if (m_clipLimit > 0) {
            m_largeHistogramPlot->graph(2)->setVisible(true);
            QVector<double> clipX = {0, 65535};
            QVector<double> clipY = {m_clipLimit, m_clipLimit};
            m_largeHistogramPlot->graph(2)->setData(clipX, clipY);
        }

        m_largeHistogramPlot->replot(QCustomPlot::rpQueuedReplot);
    }
}

void Histogram::toggleClaheVisibility() {
    if (!m_histogramPlot || !m_hasClaheData) return;

    bool currentVisible = m_histogramPlot->graph(1)->visible();
    m_histogramPlot->graph(1)->setVisible(!currentVisible);
    m_histogramPlot->graph(2)->setVisible(!currentVisible);
    m_histogramPlot->replot(QCustomPlot::rpQueuedReplot);

    if (m_claheButton) {
        m_claheButton->setText(currentVisible ? "Show CLAHE" : "Hide CLAHE");
    }
}

void Histogram::clearClaheData() {
    m_hasClaheData = false;
    m_clipLimit = -1;
    if (m_histogramPlot) {
        m_histogramPlot->graph(1)->setVisible(false);
        m_histogramPlot->graph(2)->setVisible(false);
        m_histogramPlot->replot(QCustomPlot::rpQueuedReplot);
    }
    if (m_claheButton) {
        m_claheButton->setText("Show CLAHE");
    }
}

void Histogram::updateHistogramCache(const std::vector<std::atomic<int>>& histogram)
{
    m_cache.data.clear();
    m_cache.data.reserve(HISTOGRAM_BUFFER_SIZE);

    // Copy histogram data to cache
    for (const auto& bin : histogram) {
        m_cache.data.push_back(bin.load(std::memory_order_relaxed));
    }

    // Update cache dimensions if we have an image pointer
    if (m_imagePtr && !m_imagePtr->empty()) {
        m_cache.imageHeight = m_imagePtr->size();
        m_cache.imageWidth = (*m_imagePtr)[0].size();
        m_cache.checksum = calculateChecksum(*m_imagePtr);
    }
}

void Histogram::updateHistogramCommon(const std::vector<std::atomic<int>>& histogram) {
    // Find maximum count
    double maxCount = 0;
    for (const auto& bin : histogram) {
        maxCount = std::max(maxCount, static_cast<double>(bin.load(std::memory_order_relaxed)));
    }

    // Apply smoothing and create display data
    const int smoothingWindow = 5;
    m_currentHistogram.resize(DEFAULT_BIN_COUNT);
    m_xData.resize(DEFAULT_BIN_COUNT);
    double binWidth = HISTOGRAM_BUFFER_SIZE / static_cast<double>(DEFAULT_BIN_COUNT);

    // First pass: collect raw values
    std::vector<double> rawValues(DEFAULT_BIN_COUNT);
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double binSum = 0;
        int startBin = static_cast<int>(i * binWidth);
        int endBin = static_cast<int>((i + 1) * binWidth);

        for (int j = startBin; j < endBin; ++j) {
            binSum += histogram[j].load(std::memory_order_relaxed);
        }
        rawValues[i] = binSum / maxCount;
        m_xData[i] = i * binWidth;
    }

    // Second pass: apply Gaussian smoothing
    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double smoothedValue = 0;
        double weightSum = 0;

        for (int j = -smoothingWindow; j <= smoothingWindow; ++j) {
            int idx = i + j;
            if (idx >= 0 && idx < DEFAULT_BIN_COUNT) {
                double weight = std::exp(-0.5 * (j * j) / (smoothingWindow * smoothingWindow));
                smoothedValue += rawValues[idx] * weight;
                weightSum += weight;
            }
        }

        m_currentHistogram[i] = smoothedValue / weightSum;
    }

    updateHistogramCache(histogram);
    updateHistogramDisplay(histogram);
}
