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

void Histogram::setupHistogramPlot()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // 创建主直方图绘图窗口
    m_histogramPlot = new QCustomPlot(this);
    m_histogramPlot->setMinimumSize(PlotSettings::PLOT_MIN_WIDTH, PlotSettings::PLOT_MIN_HEIGHT);
    layout->addWidget(m_histogramPlot);

    // 设置绘图外观
    m_histogramPlot->xAxis->setLabel("Pixel Value");
    m_histogramPlot->yAxis->setLabel("Frequency");
    m_histogramPlot->setBackground(Qt::white);

    // 创建图表并设置样式
    m_histogramPlot->addGraph();
    QPen graphPen;
    graphPen.setColor(QColor(65, 105, 225));  // Royal Blue
    graphPen.setWidth(1);
    m_histogramPlot->graph(0)->setPen(graphPen);
    m_histogramPlot->graph(0)->setBrush(QBrush(QColor(65, 105, 225, 50)));

    // 禁用默认的交互
    m_histogramPlot->setInteractions(QCP::iNone);

    // 设置字体大小
    QFont tickFont = QApplication::font();
    tickFont.setPointSize(PlotSettings::FONT_SIZE_SMALL);
    m_histogramPlot->xAxis->setTickLabelFont(tickFont);
    m_histogramPlot->yAxis->setTickLabelFont(tickFont);

    QFont labelFont = QApplication::font();
    labelFont.setPointSize(PlotSettings::FONT_SIZE_MEDIUM);
    m_histogramPlot->xAxis->setLabelFont(labelFont);
    m_histogramPlot->yAxis->setLabelFont(labelFont);

    // 设置点击事件
    m_histogramPlot->setCursor(Qt::PointingHandCursor);
    connect(m_histogramPlot, &QCustomPlot::mousePress, this, &Histogram::showLargeHistogram);

    // 优化渲染
    m_histogramPlot->setNoAntialiasingOnDrag(true);
    m_histogramPlot->setPlottingHint(QCP::phFastPolylines, true);
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

void Histogram::updateHistogram(const std::vector<std::vector<uint16_t>>& image)
{
    if (image.empty() || m_isCollapsed) return;

    // 只存储指针
    m_imagePtr = &image;

    // 添加更新节流
    if (m_lastUpdate.elapsed() < MIN_UPDATE_INTERVAL) {
        if (!m_updateTimer->isActive()) {
            m_updateTimer->start(UPDATE_INTERVAL_MS);
        }
        return;
    }

    scheduleUpdate();
}

void Histogram::performDelayedUpdate()
{
    if (!m_imagePtr || m_imagePtr->empty()) return;

    calculateFullHistogram();
}

void Histogram::calculateFullHistogram()
{
    const auto& image = *m_imagePtr;
    int rows = image.size();
    int cols = image[0].size();

    // 创建原子计数器数组
    std::vector<std::atomic<int>> histogram(HISTOGRAM_BUFFER_SIZE);
    for (auto& bin : histogram) {
        bin.store(0, std::memory_order_relaxed);
    }

    // 并行计算直方图
#pragma omp parallel for collapse(2)
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            histogram[image[y][x]].fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 计算最大值用于归一化
    double maxCount = 0;
    for (const auto& bin : histogram) {
        maxCount = std::max(maxCount, static_cast<double>(bin.load(std::memory_order_relaxed)));
    }

    // 归一化并存储到显示缓冲区
    m_currentHistogram.resize(DEFAULT_BIN_COUNT);
    double binWidth = HISTOGRAM_BUFFER_SIZE / static_cast<double>(DEFAULT_BIN_COUNT);

    for (int i = 0; i < DEFAULT_BIN_COUNT; ++i) {
        double sum = 0;
        int startBin = static_cast<int>(i * binWidth);
        int endBin = static_cast<int>((i + 1) * binWidth);

        for (int j = startBin; j < endBin; ++j) {
            sum += histogram[j].load(std::memory_order_relaxed);
        }
        m_currentHistogram[i] = sum / maxCount;
    }

    // 更新缓存
    m_cache.data.clear();
    m_cache.data.reserve(HISTOGRAM_BUFFER_SIZE);
    for (const auto& bin : histogram) {
        m_cache.data.push_back(bin.load(std::memory_order_relaxed));
    }
    m_cache.imageWidth = cols;
    m_cache.imageHeight = rows;
    m_cache.checksum = calculateChecksum(image);

    // 更新显示
    updateHistogramDisplay(histogram);
}

void Histogram::updateHistogramDisplay(const std::vector<std::atomic<int>>& histogram)
{
    if (!m_histogramPlot) return;

    // 更新数据范围
    m_currentXRange = QCPRange(0, 65535);
    double maxValue = *std::max_element(m_currentHistogram.begin(), m_currentHistogram.end());
    m_currentYRange = QCPRange(0, maxValue * 1.1);

    // 批量更新绘图数据
    m_histogramPlot->graph(0)->setData(m_xData, m_currentHistogram);
    m_histogramPlot->xAxis->setRange(m_currentXRange);
    m_histogramPlot->yAxis->setRange(m_currentYRange);

    // 使用队列重绘以提高性能
    m_histogramPlot->replot(QCustomPlot::rpQueuedReplot);

    // 如果大直方图窗口可见，也更新它
    if (m_histogramDialog && m_histogramDialog->isVisible()) {
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

    // 找到最近的数据点
    int index = findNearestDataPoint(x);
    if (index >= 0 && index < m_currentHistogram.size()) {
        // 计算实际的像素值和频率
        double pixelValue = m_xData[index];
        double frequency = m_currentHistogram[index];

        // 格式化tooltip文本
        QString tooltipText = QString(
                                  "Pixel Value: %1\n"
                                  "Frequency: %2%"
                                  ).arg(qRound(pixelValue))
                                  .arg(frequency * 100, 0, 'f', 2);

        // 计算tooltip位置
        QPoint pos = event->pos() + QPoint(10, 10);

        // 确保tooltip不会超出绘图区域
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
