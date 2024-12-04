#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <QtWidgets/QWidget>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include "third_party/qcustomplot/qcustomplot.h"

class Histogram : public QWidget {
    Q_OBJECT

public:
    explicit Histogram(QWidget* parent = nullptr);
    ~Histogram();

    void updateHistogram(const std::vector<std::vector<uint16_t>>& image);
    QCustomPlot* getHistogramPlot() const { return m_histogramPlot; }
    void setCollapsed(bool collapsed);
    bool isCollapsed() const { return m_isCollapsed; }

protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void showLargeHistogram();
    void updateLargeHistogram();
    void showTooltip(QMouseEvent* event);
    void resetZoom();

private:
    void setupHistogramPlot();
    void setupLargeHistogramDialog();

    QCustomPlot* m_histogramPlot;
    QDialog* m_histogramDialog;
    QCustomPlot* m_largeHistogramPlot;
    QLabel* m_tooltip;
    bool m_isCollapsed;
    int m_expandedHeight;

    QVector<double> m_currentBinCenters;
    QVector<double> m_currentHistogram;
    QCPRange m_currentXRange;
    QCPRange m_currentYRange;
};

#endif // HISTOGRAM_H
