#ifndef POINTER_OPERATIONS_H
#define POINTER_OPERATIONS_H

#include <QString>
#include <QObject>
#include <QPointF>
#include <QPainter>
#include <QtWidgets/QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QListWidget>
#include <vector>
#include <utility>
#include "darkline_pointer.h"  // Include this first to get DarkLine and DarkLineArray definitions
#include "image_processor.h"

class ControlPanel;  // Forward declaration

class PointerOperations {
public:

    struct DarkLineArrayDeleter {
        void operator()(DarkLineArray* array) {
            if (array) {
                DarkLinePointerProcessor::destroyDarkLineArray(array);
            }
        }
    };

    struct ImageDataGuard {
        ImageData& data;
        ImageDataGuard(ImageData& d) : data(d) {}
        ~ImageDataGuard() { cleanup(data); }
    private:
        static void cleanup(ImageData& data) {
            if (data.data) {
                for (int i = 0; i < data.rows; i++) {
                    delete[] data.data[i];
                }
                delete[] data.data;
                data.data = nullptr;
            }
        }
    };

    static QString generateDarkLineInfo(const DarkLineArray* lines);
    static void handleRemoveLinesDialog(ControlPanel* panel);

    static void handleDirectStitchRemoval(
        ControlPanel* panel,
        const DarkLineArray* lines,
        std::vector<std::pair<int, int>>& lineIndices,
        bool isInObject);

    static void handleNeighborValuesRemoval(
        ControlPanel* panel,
        DarkLineArray* lines,
        const std::vector<std::pair<int, int>>& lineIndices,
        bool isInObject);

    static void handleIsolatedLinesRemoval(ControlPanel* panel);

    static QGroupBox* createRemovalTypeBox(int inObjectCount, int isolatedCount);
    static QGroupBox* createMethodSelectionBox();
    static QGroupBox* createLineSelectionBox(const DarkLineArray* lines);
    static void updateLineList(ControlPanel* panel, QRadioButton* inObjectRadio, QListWidget* lineList);

    static QString generateRemovalSummary(
        const DarkLineArray* initialLines,
        const DarkLineArray* finalLines,
        const std::vector<std::pair<int, int>>& removedIndices,
        bool removedInObject,
        const QString& methodStr);

    static void countLinesInArray(const DarkLineArray* array, int& inObjectCount, int& isolatedCount);

    static DarkLineArray* createCopy(const DarkLineArray* source) {
        DarkLineArray* dest = new DarkLineArray();
        DarkLinePointerProcessor::copyDarkLineArray(source, dest);
        return dest;
    }
};

#endif // POINTER_OPERATIONS_H
