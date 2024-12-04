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
#include "darkline_pointer.h"
#include "image_processor.h"

class ControlPanel;  // Forward declaration

class PointerOperations {
public:
    static QString generateDarkLineInfo(const DarkLineArray* lines);
    static void handleRemoveLinesDialog(ControlPanel* panel);
    static QGroupBox* createRemovalTypeBox(int inObjectCount, int isolatedCount);
    static QGroupBox* createMethodSelectionBox();
    static QGroupBox* createLineSelectionBox(const DarkLineArray* lines);
    static void connectDialogControls(
        QRadioButton* inObjectRadio,
        QRadioButton* isolatedRadio,
        QRadioButton* neighborValuesRadio,
        QRadioButton* stitchRadio,
        QPushButton* selectAllButton,
        QListWidget* lineList,
        QGroupBox* methodBox,
        QGroupBox* lineSelectionBox);
    static void updateDialogVisibility(
        QRadioButton* inObjectRadio,
        QGroupBox* methodBox,
        QGroupBox* lineSelectionBox,
        QPushButton* selectAllButton,
        QRadioButton* neighborValuesRadio);
    static void updateLineList(ControlPanel* panel, QRadioButton* inObjectRadio, QListWidget* lineList);
    static void handleNeighborValuesRemoval(
        ControlPanel* panel,
        DarkLineArray* lines,
        const std::vector<std::pair<int, int>>& lineIndices,
        bool isInObject);
    static void handleIsolatedLinesRemoval(ControlPanel* panel);
    static void handleDirectStitchRemoval(
        ControlPanel* panel,
        const DarkLineArray* lines,
        std::vector<std::pair<int, int>>& lineIndices,
        bool isInObject);
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
