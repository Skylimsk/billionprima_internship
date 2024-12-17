#include "pointer_operations.h"
#include "control_panel.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QFontMetrics>
#include <QObject>  // Add this include

void PointerOperations::handleDirectStitchRemoval(
    ControlPanel* panel,
    const DarkLineArray* lines,
    std::vector<std::pair<int, int>>& lineIndices,
    bool isInObject) {

    if (!panel || !lines || lineIndices.empty()) return;

    try {
        // Store initial dimensions
        int originalHeight = panel->getImageProcessor().getFinalImageHeight();
        int originalWidth = panel->getImageProcessor().getFinalImageWidth();

        // Initialize and store initial state
        std::unique_ptr<DarkLineArray, DarkLineArrayDeleter> initialLines(createCopy(lines));

        // Get current image data
        const auto& finalImage = panel->getImageProcessor().getFinalImage();
        int height = panel->getImageProcessor().getFinalImageHeight();
        int width = panel->getImageProcessor().getFinalImageWidth();

        // Setup image data with RAII
        ImageData imageData;
        imageData.rows = height;
        imageData.cols = width;
        imageData.data = new double*[height];

        struct DataGuard {
            ImageData& data;
            ~DataGuard() {
                if (data.data) {
                    for (int i = 0; i < data.rows; i++) delete[] data.data[i];
                    delete[] data.data;
                    data.data = nullptr;
                }
            }
        } dataGuard{imageData};

        // Copy image data
        for (int y = 0; y < height; y++) {
            imageData.data[y] = new double[width];
            memcpy(imageData.data[y], finalImage[y], width * sizeof(double));
        }

        panel->getImageProcessor().saveCurrentState();

        // Process line removal
        {
            std::unique_ptr<DarkLine*[]> selectedLinesGuard(new DarkLine*[lineIndices.size()]);
            DarkLine** selectedLines = selectedLinesGuard.get();
            int selectedCount = 0;

            // Setup selected lines
            for (const auto& [i, j] : lineIndices) {
                if (i >= lines->rows || j >= lines->cols) continue;
                selectedLines[selectedCount++] = &(const_cast<DarkLineArray*>(lines)->lines[i][j]);
            }

            if (selectedCount > 0) {
                DarkLinePointerProcessor::removeDarkLinesSequential(
                    imageData,
                    const_cast<DarkLineArray*>(lines),
                    selectedLines,
                    selectedCount,
                    isInObject,
                    true,
                    DarkLinePointerProcessor::RemovalMethod::DIRECT_STITCH
                    );
            }
        }

        // Update processed image
        double** processedImage = new double*[imageData.rows];
        for (int y = 0; y < imageData.rows; y++) {
            processedImage[y] = new double[imageData.cols];
            for (int x = 0; x < imageData.cols; x++) {
                processedImage[y][x] = std::clamp(imageData.data[y][x], 0.0, 65535.0);
            }
        }

        // Update the image display with new dimensions
        panel->getImageProcessor().updateAndSaveFinalImage(processedImage, imageData.rows, imageData.cols);

        // Get final dimensions
        int finalHeight = imageData.rows;
        int finalWidth = imageData.cols;

        // Update image size label with both original and new dimensions
        QString imageSizeText = QString("Image Size: %1 x %2 (Original: %3 x %4)")
                                    .arg(finalWidth)
                                    .arg(finalHeight)
                                    .arg(originalWidth)
                                    .arg(originalHeight);
        panel->m_imageSizeLabel->setText(imageSizeText);

        // Clean up processed image
        for (int y = 0; y < imageData.rows; y++) delete[] processedImage[y];
        delete[] processedImage;

        // Generate removal summary before clearing detection
        QString removalInfo = generateRemovalSummary(
            initialLines.get(),
            lines,
            lineIndices,
            isInObject,
            "Direct Stitch"
            );

        // Clear detection results
        panel->resetDetectedLinesPointer();

        // Update UI components
        panel->getDarkLineInfoLabel()->setText(removalInfo);
        panel->getDarkLineInfoLabel()->setVisible(true);
        panel->updateDarkLineInfoDisplayPointer();

        // Update last action information with size changes
        QString actionParams = QString("Method: Direct Stitch\nSize: %1x%2 -> %3x%4")
                                   .arg(originalWidth)
                                   .arg(originalHeight)
                                   .arg(finalWidth)
                                   .arg(finalHeight);
        panel->updateLastAction("Remove Dark Lines", actionParams);

        // Update the display
        panel->updateImageDisplay();

    } catch (const std::exception& e) {
        if (!QString(e.what()).contains("resource deadlock")) {
            QMessageBox::critical(panel, "Error",
                                  QString("Error in direct stitch removal: %1").arg(e.what()));
        }
    }
}

void PointerOperations::updateLineList(ControlPanel* panel, QRadioButton* inObjectRadio, QListWidget* lineList) {
    if (!lineList || !panel->getDetectedLinesPointer()) return;

    lineList->clear();
    bool showInObject = inObjectRadio->isChecked();

    for (int i = 0; i < panel->getDetectedLinesPointer()->rows; i++) {
        for (int j = 0; j < panel->getDetectedLinesPointer()->cols; j++) {
            const auto& line = panel->getDetectedLinesPointer()->lines[i][j];
            if (line.inObject == showInObject) {
                QString coordinates;
                if (line.isVertical) {
                    coordinates = QString("x=%1 (%2-%3)")
                    .arg(line.x)
                        .arg(line.startY)
                        .arg(line.endY);
                } else {
                    coordinates = QString("y=%1 (%2-%3)")
                    .arg(line.y)
                        .arg(line.startX)
                        .arg(line.endX);
                }

                QString lineInfo = QString("Line %1: %2 at %3 with width %4")
                                       .arg(i * panel->getDetectedLinesPointer()->cols + j + 1)
                                       .arg(line.isVertical ? "Vertical" : "Horizontal")
                                       .arg(coordinates)
                                       .arg(line.width);

                QListWidgetItem* item = new QListWidgetItem(lineInfo);
                item->setData(Qt::UserRole, QPoint(i, j));
                lineList->addItem(item);
            }
        }
    }
}

QGroupBox* PointerOperations::createRemovalTypeBox(int inObjectCount, int isolatedCount) {
    QGroupBox* removalTypeBox = new QGroupBox("Select Lines to Remove");
    QVBoxLayout* typeLayout = new QVBoxLayout();

    QRadioButton* inObjectRadio = new QRadioButton(
        QString("Remove In Object Lines (%1 lines)").arg(inObjectCount));
    QRadioButton* isolatedRadio = new QRadioButton(
        QString("Remove Isolated Lines (%1 lines)").arg(isolatedCount));

    inObjectRadio->setObjectName("inObjectRadio");
    isolatedRadio->setObjectName("isolatedRadio");
    inObjectRadio->setChecked(true);

    typeLayout->addWidget(inObjectRadio);
    typeLayout->addWidget(isolatedRadio);
    removalTypeBox->setLayout(typeLayout);

    return removalTypeBox;
}

QGroupBox* PointerOperations::createMethodSelectionBox() {
    QGroupBox* methodBox = new QGroupBox("Removal Method");
    QVBoxLayout* methodLayout = new QVBoxLayout();

    QRadioButton* neighborValuesRadio = new QRadioButton("Use Neighbor Values");
    QRadioButton* stitchRadio = new QRadioButton("Direct Stitch");

    neighborValuesRadio->setObjectName("neighborValuesRadio");
    stitchRadio->setObjectName("stitchRadio");
    neighborValuesRadio->setChecked(true);

    methodLayout->addWidget(neighborValuesRadio);
    methodLayout->addWidget(stitchRadio);
    methodBox->setLayout(methodLayout);

    return methodBox;
}

QGroupBox* PointerOperations::createLineSelectionBox(const DarkLineArray* lines) {
    QGroupBox* lineSelectionBox = new QGroupBox("Select Lines to Process");
    QVBoxLayout* selectionLayout = new QVBoxLayout();
    QListWidget* lineList = new QListWidget();

    lineList->setObjectName("lineList");
    lineList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    lineList->setMinimumHeight(200);
    lineList->setMaximumHeight(300);

    // Add lines to list...
    for (int i = 0; i < lines->rows; i++) {
        for (int j = 0; j < lines->cols; j++) {
            const auto& line = lines->lines[i][j];
            if (line.inObject) {
                QString coordinates;
                if (line.isVertical) {
                    coordinates = QString("x=%1 (%2-%3)")
                    .arg(line.x)
                        .arg(line.startY)
                        .arg(line.endY);
                } else {
                    coordinates = QString("y=%1 (%2-%3)")
                    .arg(line.y)
                        .arg(line.startX)
                        .arg(line.endX);
                }

                QString lineInfo = QString("Line %1: %2 at %3 with width %4")
                                       .arg(i * lines->cols + j + 1)
                                       .arg(line.isVertical ? "Vertical" : "Horizontal")
                                       .arg(coordinates)
                                       .arg(line.width);

                QListWidgetItem* item = new QListWidgetItem(lineInfo);
                item->setData(Qt::UserRole, QPoint(i, j));
                lineList->addItem(item);
            }
        }
    }

    selectionLayout->addWidget(lineList);
    lineSelectionBox->setLayout(selectionLayout);

    return lineSelectionBox;
}

void PointerOperations::handleRemoveLinesDialog(ControlPanel* panel) {
    if (!panel || !panel->getDetectedLinesPointer()) return;

    // Count lines by type
    int inObjectCount = 0;
    int isolatedCount = 0;
    for (int i = 0; i < panel->getDetectedLinesPointer()->rows; ++i) {
        for (int j = 0; j < panel->getDetectedLinesPointer()->cols; ++j) {
            if (panel->getDetectedLinesPointer()->lines[i][j].inObject) {
                inObjectCount++;
            } else {
                isolatedCount++;
            }
        }
    }

    // Create dialog
    QDialog dialog(panel);
    dialog.setWindowTitle("Remove Lines (2D Pointer)");
    dialog.setMinimumWidth(400);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Create dialog components
    QGroupBox* removalTypeBox = createRemovalTypeBox(inObjectCount, isolatedCount);
    QGroupBox* methodBox = createMethodSelectionBox();
    QGroupBox* lineSelectionBox = createLineSelectionBox(panel->getDetectedLinesPointer());

    layout->addWidget(removalTypeBox);
    layout->addWidget(methodBox);
    layout->addWidget(lineSelectionBox);

    // Get radio buttons
    QRadioButton* inObjectRadio = removalTypeBox->findChild<QRadioButton*>("inObjectRadio");
    QRadioButton* isolatedRadio = removalTypeBox->findChild<QRadioButton*>("isolatedRadio");
    QRadioButton* neighborValuesRadio = methodBox->findChild<QRadioButton*>("neighborValuesRadio");
    QRadioButton* stitchRadio = methodBox->findChild<QRadioButton*>("stitchRadio");
    QListWidget* lineList = lineSelectionBox->findChild<QListWidget*>("lineList");

    // Add select all button
    QPushButton* selectAllButton = new QPushButton("Select All Lines");
    layout->addWidget(selectAllButton);

    // Setup visibility controls
    auto updateVisibility = [&]() {
        bool isInObject = inObjectRadio->isChecked();
        methodBox->setVisible(isInObject);
        lineSelectionBox->setVisible(isInObject);
        selectAllButton->setVisible(isInObject && neighborValuesRadio->isChecked());
        dialog.adjustSize();
    };

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout->addWidget(buttonBox);

    // Change all connect calls to:
    QObject::connect(inObjectRadio, &QRadioButton::toggled, [&]() {
        updateVisibility();
        updateLineList(panel, inObjectRadio, lineList);
    });

    QObject::connect(isolatedRadio, &QRadioButton::toggled, updateVisibility);

    QObject::connect(neighborValuesRadio, &QRadioButton::toggled, [&](bool checked) {
        selectAllButton->setVisible(checked);
        lineList->setSelectionMode(QAbstractItemView::ExtendedSelection);
    });

    QObject::connect(stitchRadio, &QRadioButton::toggled, [&](bool checked) {
        selectAllButton->setVisible(!checked);
        if (checked) {
            lineList->setSelectionMode(QAbstractItemView::SingleSelection);
            if (lineList->selectedItems().count() > 1) {
                lineList->clearSelection();
                if (lineList->count() > 0) {
                    lineList->item(0)->setSelected(true);
                }
            }
        }
    });

    QObject::connect(selectAllButton, &QPushButton::clicked, lineList, &QListWidget::selectAll);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    updateVisibility();

    // Handle dialog result
    if (dialog.exec() == QDialog::Accepted) {
        bool removeInObject = inObjectRadio->isChecked();

        if (removeInObject) {
            auto selectedItems = lineList->selectedItems();
            if (selectedItems.isEmpty()) {
                QMessageBox::warning(panel, "Warning", "Please select at least one line for processing.");
                return;
            }

            std::vector<std::pair<int, int>> selectedIndices;
            for (QListWidgetItem* item : selectedItems) {
                QPoint indices = item->data(Qt::UserRole).toPoint();
                selectedIndices.emplace_back(indices.x(), indices.y());
            }

            if (neighborValuesRadio->isChecked()) {
                handleNeighborValuesRemoval(panel, panel->getDetectedLinesPointer(), selectedIndices, true);
            } else {
                handleDirectStitchRemoval(panel, panel->getDetectedLinesPointer(), selectedIndices, true);
            }
        } else {
            handleIsolatedLinesRemoval(panel);
        }
    }
}

QString PointerOperations::generateRemovalSummary(
    const DarkLineArray* initialLines,
    const DarkLineArray* finalLines,
    const std::vector<std::pair<int, int>>& removedIndices,
    bool removedInObject,
    const QString& methodStr) {

    QString removalInfo = "Line Removal Summary (2D Pointer):\n\n";
    removalInfo += QString("Method Used: %1\n").arg(methodStr);
    removalInfo += QString("Type: %1\n\n").arg(removedInObject ? "In-Object Lines" : "Isolated Lines");

    // Count initial lines
    int initialInObjectCount = 0;
    int initialIsolatedCount = 0;
    countLinesInArray(initialLines, initialInObjectCount, initialIsolatedCount);

    removalInfo += "Initial Lines:\n";
    removalInfo += QString("Total Lines: %1\n").arg(initialLines->rows * initialLines->cols);
    removalInfo += QString("In-Object Lines: %1\n").arg(initialInObjectCount);
    removalInfo += QString("Isolated Lines: %1\n\n").arg(initialIsolatedCount);

    // Add removed lines info with simplified coordinates
    removalInfo += "Removed Lines:\n";
    if (removedInObject) {
        for (const auto& [i, j] : removedIndices) {
            const auto& line = initialLines->lines[i][j];
            QString coordinates;
            if (line.isVertical) {
                coordinates = QString("(%1,0)").arg(line.x);
            } else {
                coordinates = QString("(0,%1)").arg(line.y);
            }

            removalInfo += QString("Line at %1 with width %2 pixels (%3)\n")
                               .arg(coordinates)
                               .arg(line.width)
                               .arg(line.inObject ? "In Object" : "Isolated");
        }
    } else {
        // For isolated lines, show all removed lines by comparing initial and final arrays
        for (int i = 0; i < initialLines->rows; i++) {
            for (int j = 0; j < initialLines->cols; j++) {
                const auto& initialLine = initialLines->lines[i][j];
                bool wasRemoved = true;

                // Check if line still exists in final array
                for (int fi = 0; fi < finalLines->rows; fi++) {
                    for (int fj = 0; fj < finalLines->cols; fj++) {
                        const auto& finalLine = finalLines->lines[fi][fj];
                        if ((initialLine.isVertical == finalLine.isVertical) &&
                            (initialLine.isVertical ?
                                 (initialLine.x == finalLine.x) :
                                 (initialLine.y == finalLine.y)) &&
                            (initialLine.width == finalLine.width)) {
                            wasRemoved = false;
                            break;
                        }
                    }
                    if (!wasRemoved) break;
                }

                if (wasRemoved) {
                    QString coordinates;
                    if (initialLine.isVertical) {
                        coordinates = QString("(%1,0)").arg(initialLine.x);
                    } else {
                        coordinates = QString("(0,%1)").arg(initialLine.y);
                    }

                    removalInfo += QString("Line at %1 with width %2 pixels (%3)\n")
                                       .arg(coordinates)
                                       .arg(initialLine.width)
                                       .arg(initialLine.inObject ? "In Object" : "Isolated");
                }
            }
        }
    }

    // Add remaining lines summary with simplified coordinates
    removalInfo += "\nRemaining Lines:\n";
    for (int i = 0; i < finalLines->rows; i++) {
        for (int j = 0; j < finalLines->cols; j++) {
            const auto& line = finalLines->lines[i][j];
            QString coordinates;
            if (line.isVertical) {
                coordinates = QString("(%1,0)").arg(line.x);
            } else {
                coordinates = QString("(0,%1)").arg(line.y);
            }

            removalInfo += QString("Line %1: %2 with width %3 pixels (%4)\n")
                               .arg(i * finalLines->cols + j + 1)
                               .arg(coordinates)
                               .arg(line.width)
                               .arg(line.inObject ? "In Object" : "Isolated");
        }
    }

    // Count remaining lines
    int remainingInObjectCount = 0;
    int remainingIsolatedCount = 0;
    countLinesInArray(finalLines, remainingInObjectCount, remainingIsolatedCount);

    removalInfo += QString("\nSummary of Remaining Lines:\n");
    removalInfo += QString("Total Lines: %1\n").arg(finalLines->rows * finalLines->cols);
    removalInfo += QString("In-Object Lines: %1\n").arg(remainingInObjectCount);
    removalInfo += QString("Isolated Lines: %1\n").arg(remainingIsolatedCount);

    return removalInfo;
}

// Also update the generateDarkLineInfo function for consistency
QString PointerOperations::generateDarkLineInfo(const DarkLineArray* lines) {
    QString detectionInfo = "Detected Lines (2D Pointer):\n\n";
    int inObjectCount = 0;
    int isolatedCount = 0;

    for (int i = 0; i < lines->rows; ++i) {
        for (int j = 0; j < lines->cols; ++j) {
            const auto& line = lines->lines[i][j];
            QString coordinates;

            if (line.isVertical) {
                coordinates = QString("(%1,0)").arg(line.x);
            } else {
                coordinates = QString("(0,%1)").arg(line.y);
            }

            detectionInfo += QString("Line %1: %2 with width %3 pixels (%4)\n")
                                 .arg(i * lines->cols + j + 1)
                                 .arg(coordinates)
                                 .arg(line.width)
                                 .arg(line.inObject ? "In Object" : "Isolated");

            if (line.inObject) {
                inObjectCount++;
            } else {
                isolatedCount++;
            }
        }
    }

    detectionInfo += QString("\nSummary:\n");
    detectionInfo += QString("Total Lines: %1\n").arg(lines->rows * lines->cols);
    detectionInfo += QString("In-Object Lines: %1\n").arg(inObjectCount);
    detectionInfo += QString("Isolated Lines: %1\n").arg(isolatedCount);

    return detectionInfo;
}

void PointerOperations::countLinesInArray(const DarkLineArray* array, int& inObjectCount, int& isolatedCount) {
    inObjectCount = 0;
    isolatedCount = 0;

    for (int i = 0; i < array->rows; i++) {
        for (int j = 0; j < array->cols; j++) {
            if (array->lines[i][j].inObject) {
                inObjectCount++;
            } else {
                isolatedCount++;
            }
        }
    }
}

void PointerOperations::handleNeighborValuesRemoval(
    ControlPanel* panel,
    DarkLineArray* lines,
    const std::vector<std::pair<int, int>>& lineIndices,
    bool isInObject) {

    try {
        // Store initial state - create a deep copy
        DarkLineArray* initialLines = PointerOperations::createCopy(lines);

        // Get current image dimensions and data
        const auto& finalImage = panel->getImageProcessor().getFinalImage();
        int height = panel->getImageProcessor().getFinalImageHeight();
        int width = panel->getImageProcessor().getFinalImageWidth();

        auto imageData = panel->convertToImageData(finalImage, height, width);

        // Create array of selected lines
        DarkLine** selectedLines = new DarkLine*[lineIndices.size()];
        int selectedCount = 0;

        // Collect selected lines
        for (const auto& [i, j] : lineIndices) {
            selectedLines[selectedCount++] = &(lines->lines[i][j]);
        }

        panel->getImageProcessor().saveCurrentState();

        // Process using neighbor values method
        DarkLinePointerProcessor::removeDarkLinesSequential(
            imageData,
            lines,
            selectedLines,
            selectedCount,
            isInObject,
            false,
            DarkLinePointerProcessor::RemovalMethod::NEIGHBOR_VALUES
            );

        // Update image and detect remaining lines
        auto processedImage = panel->convertFromImageData(imageData);
        panel->getImageProcessor().updateAndSaveFinalImage(processedImage, height, width);

        // Clean up processed image after updating
        for (int i = 0; i < height; i++) {
            delete[] processedImage[i];
        }
        delete[] processedImage;

        panel->resetDetectedLinesPointer();
        DarkLineArray* remainingLines = DarkLinePointerProcessor::detectDarkLines(imageData);
        panel->setDetectedLinesPointer(remainingLines);

        // Generate and display removal summary
        QString removalInfo = generateRemovalSummary(
            initialLines,
            remainingLines,
            lineIndices,
            isInObject,
            "Neighbor Values"
            );

        // Update info label
        panel->getDarkLineInfoLabel()->setText(removalInfo);
        panel->updateDarkLineInfoDisplayPointer();

        // Cleanup
        delete[] selectedLines;
        DarkLinePointerProcessor::destroyDarkLineArray(initialLines);

        panel->updateImageDisplay();

    } catch (const std::exception& e) {
        QMessageBox::critical(panel, "Error",
                              QString("Error in neighbor values removal: %1").arg(e.what()));
    }
}

void PointerOperations::handleIsolatedLinesRemoval(ControlPanel* panel) {
    try {
        // Store initial state - create a deep copy
        DarkLineArray* initialLines = PointerOperations::createCopy(panel->getDetectedLinesPointer());

        // Get current image dimensions and data
        const auto& finalImage = panel->getImageProcessor().getFinalImage();
        int height = panel->getImageProcessor().getFinalImageHeight();
        int width = panel->getImageProcessor().getFinalImageWidth();

        auto imageData = panel->convertToImageData(finalImage, height, width);
        panel->getImageProcessor().saveCurrentState();

        DarkLinePointerProcessor::removeDarkLinesSelective(
            imageData,
            panel->getDetectedLinesPointer(),
            false,
            true,
            DarkLinePointerProcessor::RemovalMethod::NEIGHBOR_VALUES
            );

        auto processedImage = panel->convertFromImageData(imageData);
        panel->getImageProcessor().updateAndSaveFinalImage(processedImage, height, width);

        // Clean up processed image after updating
        for (int i = 0; i < height; i++) {
            delete[] processedImage[i];
        }
        delete[] processedImage;

        panel->resetDetectedLinesPointer();
        DarkLineArray* remainingLines = DarkLinePointerProcessor::detectDarkLines(imageData);
        panel->setDetectedLinesPointer(remainingLines);

        // Generate and display removal summary
        QString removalInfo = generateRemovalSummary(
            initialLines,
            remainingLines,
            std::vector<std::pair<int, int>>(),  // Empty for isolated lines
            false,
            "Neighbor Values"
            );

        // Update info label
        panel->getDarkLineInfoLabel()->setText(removalInfo);
        panel->updateDarkLineInfoDisplayPointer();

        // Cleanup
        DarkLinePointerProcessor::destroyDarkLineArray(initialLines);

        panel->updateImageDisplay();

    } catch (const std::exception& e) {
        QMessageBox::critical(panel, "Error",
                              QString("Error in isolated lines removal: %1").arg(e.what()));
    }
}
