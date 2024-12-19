#ifndef GRAPH_3D_PROCESSOR_H
#define GRAPH_3D_PROCESSOR_H

#pragma once

#include <QWidget>
#include <QDialog>
#include "third_party/qcustomplot/qcustomplot.h"
#include <QObject>
#include <memory>
#include <atomic>

class Graph3DProcessor : public QObject {
    Q_OBJECT
public:
    struct Graph3DData {
        std::unique_ptr<double[]> data;
        int width = 0;
        int height = 0;
        double minValue = 0.0;
        double maxValue = 0.0;
    };

    static std::unique_ptr<Graph3DProcessor> create(QWidget* parent = nullptr);
    explicit Graph3DProcessor(QWidget* parent = nullptr);
    ~Graph3DProcessor();

    // Prevent copying and assignment
    Graph3DProcessor(const Graph3DProcessor&) = delete;
    Graph3DProcessor& operator=(const Graph3DProcessor&) = delete;

    void show3DGraph(double** image, int height, int width);

private:
    void createOrUpdateDialog();
    void updateVisualization(const Graph3DData* data);
    void resetView();

    QWidget* m_parent;
    QDialog* m_dialog;
    QCustomPlot* m_plot;
    std::atomic<bool> m_isProcessing;
};

#endif // GRAPH_3D_PROCESSOR_H
