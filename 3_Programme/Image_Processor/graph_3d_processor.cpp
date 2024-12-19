// graph_3d_processor.cpp
#include "graph_3d_processor.h"
#include <QObject>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <algorithm>

std::unique_ptr<Graph3DProcessor> Graph3DProcessor::create(QWidget* parent) {
    return std::make_unique<Graph3DProcessor>(parent);
}

Graph3DProcessor::Graph3DProcessor(QWidget* parent)
    : QObject(parent)
    , m_parent(parent)
    , m_dialog(nullptr)
    , m_plot(nullptr)
    , m_isProcessing(false) {
}

Graph3DProcessor::~Graph3DProcessor() {
    if (m_dialog) {
        m_dialog->hide();
        delete m_dialog;
    }
}

void Graph3DProcessor::show3DGraph(double** image, int height, int width) {
    if (!image || height <= 0 || width <= 0) {
        QMessageBox::warning(m_parent, "Error", "Invalid image data for 3D visualization");
        return;
    }

    // Prevent multiple processing attempts
    bool expected = false;
    if (!m_isProcessing.compare_exchange_strong(expected, true)) {
        QMessageBox::information(m_parent, "Processing",
                                 "Please wait while the previous visualization is being prepared.");
        return;
    }

    try {
        // Create dialog if not exists
        createOrUpdateDialog();

        // Get the surface plot from QCustomPlot
        QCPColorMap* colorMap = qobject_cast<QCPColorMap*>(m_plot->plottable(0));
        if (!colorMap) {
            colorMap = new QCPColorMap(m_plot->xAxis, m_plot->yAxis);
        }

        // Configure data dimensions
        colorMap->data()->setSize(width, height);
        colorMap->data()->setRange(QCPRange(0, width), QCPRange(0, height));

        // Find min/max values for normalization
        double minVal = std::numeric_limits<double>::max();
        double maxVal = std::numeric_limits<double>::lowest();

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                double value = std::clamp(image[y][x], 0.0, 65535.0);
                minVal = std::min(minVal, value);
                maxVal = std::max(maxVal, value);
            }
        }

        // Set up the 3D surface data
        double range = maxVal - minVal;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Normalize value between 0 and 1
                double normalizedValue = (image[y][x] - minVal) / range;
                colorMap->data()->setCell(x, y, normalizedValue);
            }
        }

        // Configure axes
        m_plot->xAxis->setLabel("Column");
        m_plot->yAxis->setLabel("Row");

        // Add or update color scale
        QCPColorScale* colorScale = nullptr;
        for (int i = 0; i < m_plot->plotLayout()->elementCount(); ++i) {
            if (QCPColorScale* scale = qobject_cast<QCPColorScale*>(m_plot->plotLayout()->elementAt(i))) {
                colorScale = scale;
                break;
            }
        }

        if (!colorScale) {
            colorScale = new QCPColorScale(m_plot);
            m_plot->plotLayout()->addElement(0, 1, colorScale);
        }

        colorScale->setType(QCPAxis::atRight);
        colorScale->setLabel(QString("Intensity\n[%1 - %2]").arg(minVal, 0, 'f', 0).arg(maxVal, 0, 'f', 0));
        colorMap->setColorScale(colorScale);

        // Set color gradient to match your example (blue to red)
        QCPColorGradient gradient;
        gradient.setColorStops({
            {0.0, QColor(0, 0, 255)},   // Blue for low values
            {0.5, QColor(128, 0, 128)},  // Purple for mid values
            {1.0, QColor(255, 0, 0)}     // Red for high values
        });
        colorMap->setGradient(gradient);

        // Configure plot margins and spacing
        m_plot->axisRect()->setupFullAxesBox(true);
        m_plot->axisRect()->setMargins(QMargins(10, 10, 30, 10));

        // Enable antialiasing for smoother display
        m_plot->setAntialiasedElements(QCP::aeAll);
        colorMap->setInterpolate(true);

        // Update data ranges
        colorMap->rescaleDataRange();
        m_plot->rescaleAxes();

        // Add grid lines
        m_plot->xAxis->grid()->setVisible(true);
        m_plot->yAxis->grid()->setVisible(true);

        // Update plot
        m_plot->replot();

        // Show dialog
        m_dialog->show();
        m_dialog->raise();
        m_dialog->activateWindow();

        m_isProcessing = false;

    } catch (const std::exception& e) {
        m_isProcessing = false;
        QMessageBox::critical(m_parent, "Error",
                              QString("Failed to create 3D visualization: %1").arg(e.what()));
    }
}

void Graph3DProcessor::createOrUpdateDialog() {
    if (!m_dialog) {
        m_dialog = new QDialog(m_parent);
        m_dialog->setWindowTitle("Intensity Surface Plot");
        m_dialog->resize(800, 600);

        QVBoxLayout* layout = new QVBoxLayout(m_dialog);

        // Create plot widget
        m_plot = new QCustomPlot(m_dialog);
        m_plot->setMinimumSize(700, 500);

        // Add plot manipulation buttons
        QHBoxLayout* controlLayout = new QHBoxLayout();

        QPushButton* resetViewBtn = new QPushButton("Reset View", m_dialog);
        connect(resetViewBtn, &QPushButton::clicked, this, &Graph3DProcessor::resetView);

        QPushButton* closeBtn = new QPushButton("Close", m_dialog);
        connect(closeBtn, &QPushButton::clicked, m_dialog, &QDialog::close);

        controlLayout->addWidget(resetViewBtn);
        controlLayout->addWidget(closeBtn);

        // Add widgets to layout
        layout->addWidget(m_plot);
        layout->addLayout(controlLayout);

        // Enable interactions
        m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    }
}

void Graph3DProcessor::updateVisualization(const Graph3DData* data) {
    if (!m_plot || !data || !data->data) return;

    // Get the color map
    auto colorMap = qobject_cast<QCPColorMap*>(m_plot->plottable(0));
    if (!colorMap) return;

    // Set up the color map data
    colorMap->data()->setSize(data->width, data->height);
    colorMap->data()->setRange(QCPRange(0, data->width), QCPRange(0, data->height));

    // Fill the color map with normalized data
    double range = data->maxValue - data->minValue;
    if (range > 0) {
        for (int y = 0; y < data->height; ++y) {
            for (int x = 0; x < data->width; ++x) {
                double normalizedValue = (data->data[y * data->width + x] - data->minValue) / range;
                colorMap->data()->setCell(x, y, normalizedValue);
            }
        }
    }

    // Update color map appearance
    colorMap->setGradient(QCPColorGradient::gpJet);
    colorMap->rescaleDataRange();

    // Add margin for color scale
    m_plot->axisRect()->setMargins(QMargins(10, 10, 30, 10));

    // Update the plot
    m_plot->rescaleAxes();
    m_plot->replot();

    // Show the dialog
    m_dialog->show();
    m_dialog->raise();
    m_dialog->activateWindow();
}

void Graph3DProcessor::resetView() {
    if (m_plot) {
        m_plot->xAxis->setRange(0, m_plot->xAxis->range().size());
        m_plot->yAxis->setRange(0, m_plot->yAxis->range().size());
        m_plot->replot();
    }
}
