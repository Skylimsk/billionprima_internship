#include "histogram.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolTip>

Histogram::Histogram(QWidget* parent)
    : QWidget(parent)
    , m_histogramPlot(nullptr)
    , m_histogramDialog(nullptr)
    , m_largeHistogramPlot(nullptr)
    , m_isCollapsed(false)
    , m_expandedHeight(350)
    , m_tooltip(nullptr)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setupHistogramPlot();
    setCollapsed(false);  // Start expanded by default
}

Histogram::~Histogram()
{
    if (m_histogramDialog) {
        m_histogramDialog->deleteLater();
    }
    if (m_tooltip) {
        delete m_tooltip;
    }
}

bool Histogram::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == m_largeHistogramPlot) {
        if (event->type() == QEvent::Leave) {
            if (m_tooltip) {
                m_tooltip->hide();
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void Histogram::setupHistogramPlot()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_histogramPlot = new QCustomPlot(this);
    m_histogramPlot->setMinimumSize(250, 350);
    layout->addWidget(m_histogramPlot);

    // Setup the plot appearance
    m_histogramPlot->xAxis->setLabel("Pixel Value");
    m_histogramPlot->yAxis->setLabel("Frequency");
    m_histogramPlot->setBackground(Qt::white);

    // Create the graph
    m_histogramPlot->addGraph();
    m_histogramPlot->graph(0)->setPen(QPen(Qt::blue));
    m_histogramPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 50)));

    // Remove interactions - only allow clicking to open large view
    m_histogramPlot->setInteractions(QCP::iNone);

    // Make the plot clickable
    m_histogramPlot->setCursor(Qt::PointingHandCursor);
    connect(m_histogramPlot, &QCustomPlot::mousePress, this, &Histogram::showLargeHistogram);

    // Style improvements
    m_histogramPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 8));
    m_histogramPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 8));
    m_histogramPlot->xAxis->setLabelFont(QFont(QFont().family(), 9));
    m_histogramPlot->yAxis->setLabelFont(QFont(QFont().family(), 9));
}

void Histogram::setCollapsed(bool collapsed)
{
    m_isCollapsed = collapsed;
    if (m_isCollapsed) {
        m_expandedHeight = height();
        setFixedHeight(0);
        m_histogramPlot->hide();
    } else {
        setFixedHeight(m_expandedHeight);
        m_histogramPlot->show();
        m_histogramPlot->replot();
    }
    updateGeometry();
    if (parentWidget()) {
        parentWidget()->adjustSize();
    }
}

void Histogram::showLargeHistogram()
{
    if (!m_histogramPlot || !m_histogramPlot->graph(0)) return;

    if (!m_histogramDialog) {
        m_histogramDialog = new QDialog(this);
        m_histogramDialog->setWindowTitle("Histogram");
        m_histogramDialog->resize(800, 600);
        setupLargeHistogramDialog();
    }

    updateLargeHistogram();
    m_histogramDialog->show();
    m_histogramDialog->raise();
    m_histogramDialog->activateWindow();
}

void Histogram::setupLargeHistogramDialog()
{
    QVBoxLayout* dialogLayout = new QVBoxLayout(m_histogramDialog);

    // Create new large histogram plot
    m_largeHistogramPlot = new QCustomPlot(m_histogramDialog);
    m_largeHistogramPlot->setMinimumSize(700, 500);

    // Copy settings from small plot
    m_largeHistogramPlot->xAxis->setLabel("Pixel Value");
    m_largeHistogramPlot->yAxis->setLabel("Frequency");
    m_largeHistogramPlot->setBackground(Qt::white);
    m_largeHistogramPlot->addGraph();
    m_largeHistogramPlot->graph(0)->setPen(QPen(Qt::blue));
    m_largeHistogramPlot->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 50)));

    // Enable mouse tracking for tooltip
    m_largeHistogramPlot->setMouseTracking(true);
    m_largeHistogramPlot->installEventFilter(this);

    // Create tooltip label
    m_tooltip = new QLabel(m_largeHistogramPlot);
    m_tooltip->setStyleSheet(
        "QLabel {"
        "   background-color: rgba(255, 255, 255, 0.9);"
        "   border: 1px solid #999;"
        "   border-radius: 4px;"
        "   padding: 4px;"
        "   font-size: 10pt;"
        "}"
        );
    m_tooltip->hide();

    // Connect mouse move signal
    connect(m_largeHistogramPlot, &QCustomPlot::mouseMove, this, &Histogram::showTooltip);

    // Style improvements for large plot
    m_largeHistogramPlot->xAxis->setTickLabelFont(QFont(QFont().family(), 10));
    m_largeHistogramPlot->yAxis->setTickLabelFont(QFont(QFont().family(), 10));
    m_largeHistogramPlot->xAxis->setLabelFont(QFont(QFont().family(), 12));
    m_largeHistogramPlot->yAxis->setLabelFont(QFont(QFont().family(), 12));

    // Add plot interaction modes
    m_largeHistogramPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    // Add widgets to dialog layout
    dialogLayout->addWidget(m_largeHistogramPlot);

    // Add buttons
    QPushButton* closeButton = new QPushButton("Close", m_histogramDialog);
    closeButton->setFixedWidth(100);
    connect(closeButton, &QPushButton::clicked, m_histogramDialog, &QDialog::close);

    QPushButton* resetZoomButton = new QPushButton("Reset Zoom", m_histogramDialog);
    resetZoomButton->setFixedWidth(100);
    connect(resetZoomButton, &QPushButton::clicked, this, &Histogram::resetZoom);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(resetZoomButton);
    buttonLayout->addWidget(closeButton);
    buttonLayout->addStretch();

    dialogLayout->addLayout(buttonLayout);
    m_histogramDialog->setLayout(dialogLayout);
}

void Histogram::showTooltip(QMouseEvent* event)
{
    if (!m_largeHistogramPlot || !m_tooltip) return;

    // Get the mouse position in plot coordinates
    double x = m_largeHistogramPlot->xAxis->pixelToCoord(event->pos().x());
    double y = m_largeHistogramPlot->yAxis->pixelToCoord(event->pos().y());

    // Find the closest data point
    QCPGraph* graph = m_largeHistogramPlot->graph(0);
    if (!graph) return;

    // Find the bin index
    int binIndex = -1;
    double minDistance = std::numeric_limits<double>::max();

    for (int i = 0; i < m_currentBinCenters.size(); ++i) {
        double dist = qAbs(m_currentBinCenters[i] - x);
        if (dist < minDistance) {
            minDistance = dist;
            binIndex = i;
        }
    }

    if (binIndex >= 0 && binIndex < m_currentBinCenters.size()) {
        // Format the tooltip text
        QString tooltipText = QString("Value: %1\nFrequency: %2%")
                                  .arg(qRound(m_currentBinCenters[binIndex]))
                                  .arg(m_currentHistogram[binIndex] * 100, 0, 'f', 2);

        // Position the tooltip near the mouse cursor
        QPoint tooltipPos = event->pos() + QPoint(10, 10);

        // Ensure tooltip stays within the plot bounds
        if (tooltipPos.x() + m_tooltip->width() > m_largeHistogramPlot->width()) {
            tooltipPos.setX(event->pos().x() - m_tooltip->width() - 10);
        }
        if (tooltipPos.y() + m_tooltip->height() > m_largeHistogramPlot->height()) {
            tooltipPos.setY(event->pos().y() - m_tooltip->height() - 10);
        }

        m_tooltip->setText(tooltipText);
        m_tooltip->adjustSize();
        m_tooltip->move(tooltipPos);
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

void Histogram::updateLargeHistogram()
{
    if (!m_largeHistogramPlot || m_currentHistogram.isEmpty()) return;

    m_largeHistogramPlot->graph(0)->setData(m_currentBinCenters, m_currentHistogram);
    m_largeHistogramPlot->xAxis->setRange(m_currentXRange);
    m_largeHistogramPlot->yAxis->setRange(m_currentYRange);
    m_largeHistogramPlot->replot();
}

void Histogram::updateHistogram(const std::vector<std::vector<uint16_t>>& image)
{
    if (image.empty() || m_isCollapsed) return;

    // Calculate histogram with 256 bins
    const int numBins = 256;
    QVector<double> histogram(numBins, 0);
    QVector<double> binCenters(numBins);

    // Find the maximum pixel value in the image
    uint16_t maxPixelValue = 0;
    for (const auto& row : image) {
        for (uint16_t value : row) {
            maxPixelValue = std::max(maxPixelValue, value);
        }
    }

    // Calculate bin width based on actual max value
    double binWidth = static_cast<double>(maxPixelValue + 1) / numBins;

    // Fill bin centers
    for (int i = 0; i < numBins; ++i) {
        binCenters[i] = i * binWidth + binWidth / 2;
    }

    // Count pixels in each bin
    size_t totalPixels = 0;
    for (const auto& row : image) {
        for (uint16_t value : row) {
            int bin = static_cast<int>(value / binWidth);
            if (bin >= numBins) bin = numBins - 1;
            histogram[bin]++;
            totalPixels++;
        }
    }

    // Normalize histogram
    if (totalPixels > 0) {
        for (double& count : histogram) {
            count /= totalPixels;
        }
    }

    // Store the current data for later use
    m_currentBinCenters = binCenters;
    m_currentHistogram = histogram;

    // Find the maximum frequency
    double maxFrequency = 0;
    for (double freq : histogram) {
        maxFrequency = std::max(maxFrequency, freq);
    }

    // Add 10% padding to the ranges for better visualization
    double xMaxPadded = maxPixelValue * 1.1;
    double yMaxPadded = maxFrequency * 1.1;

    // Update the small plot
    m_histogramPlot->graph(0)->setData(binCenters, histogram);
    m_histogramPlot->xAxis->setRange(0, xMaxPadded);
    m_histogramPlot->yAxis->setRange(0, yMaxPadded);
    m_histogramPlot->replot();

    // Store current ranges
    m_currentXRange = m_histogramPlot->xAxis->range();
    m_currentYRange = m_histogramPlot->yAxis->range();

    // Update the large plot if it's visible
    if (m_histogramDialog && m_histogramDialog->isVisible()) {
        updateLargeHistogram();
    }
}
