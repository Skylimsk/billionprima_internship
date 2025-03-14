#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QGroupBox>
#include <QSpinBox>
#include <QColorDialog>
#include <QDebug>
#include <QTimer>
#include <QEvent>
#include <QTextEdit>

// VTK includes - minimal set
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkConeSource.h>
#include <vtkCylinderSource.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkAutoInit.h>
#include <vtkProperty.h>
#include <vtkCommand.h>
#include <vtkCallbackCommand.h>
#include <vtkVersion.h>

// Force VTK factory registration
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

// Global smart pointers to prevent premature destruction
vtkSmartPointer<vtkRenderWindow> g_renderWindow;
vtkSmartPointer<vtkRenderWindowInteractor> g_renderWindowInteractor;
vtkSmartPointer<vtkRenderer> g_renderer;
vtkSmartPointer<vtkActor> g_actor;
bool g_vtkInitialized = false;
QTextEdit* g_debugTextEdit = nullptr;

enum ShapeType {
    SHAPE_SPHERE,
    SHAPE_CONE,
    SHAPE_CYLINDER,
    SHAPE_CUBE
};

// Function to log debug information
void logDebugInfo(const QString& text) {
    qDebug() << text;
    if (g_debugTextEdit) {
        g_debugTextEdit->append(text);
    }
}

// Callback function to print render information
void renderCallback(vtkObject* caller, unsigned long eventId, void* clientData, void* callData) {
    vtkRenderWindow* renderWindow = vtkRenderWindow::SafeDownCast(caller);
    if (renderWindow) {
        logDebugInfo("--- Render Information ---");

        // Print VTK version
        logDebugInfo(QString("VTK Version: %1.%2.%3")
            .arg(VTK_MAJOR_VERSION)
            .arg(VTK_MINOR_VERSION)
            .arg(VTK_BUILD_VERSION));

        // Get capabilities (this is the most reliable way to get renderer info)
        const char* capabilities = renderWindow->ReportCapabilities();
        if (capabilities) {
            QString capStr = QString(capabilities);

            // Print renderer name if available
            if (capStr.contains("OpenGL vendor string:")) {
                int startPos = capStr.indexOf("OpenGL vendor string:") + 22;
                int endPos = capStr.indexOf('\n', startPos);
                if (endPos > startPos) {
                    QString vendorName = capStr.mid(startPos, endPos - startPos).trimmed();
                    logDebugInfo("OpenGL Vendor: " + vendorName);
                }
            }

            // Print renderer string if available
            if (capStr.contains("OpenGL renderer string:")) {
                int startPos = capStr.indexOf("OpenGL renderer string:") + 24;
                int endPos = capStr.indexOf('\n', startPos);
                if (endPos > startPos) {
                    QString rendererName = capStr.mid(startPos, endPos - startPos).trimmed();
                    logDebugInfo("OpenGL Renderer: " + rendererName);

                    // Check for software rendering
                    bool isSoftware =
                        rendererName.contains("software", Qt::CaseInsensitive) ||
                        rendererName.contains("llvmpipe", Qt::CaseInsensitive) ||
                        rendererName.contains("mesa", Qt::CaseInsensitive);

                    // Check for hardware vendor names
                    bool isHardware =
                        rendererName.contains("NVIDIA", Qt::CaseInsensitive) ||
                        rendererName.contains("AMD", Qt::CaseInsensitive) ||
                        rendererName.contains("ATI", Qt::CaseInsensitive) ||
                        rendererName.contains("Intel", Qt::CaseInsensitive) ||
                        rendererName.contains("Radeon", Qt::CaseInsensitive) ||
                        rendererName.contains("GeForce", Qt::CaseInsensitive);

                    // Make a determination
                    if (isSoftware) {
                        logDebugInfo("Rendering Method: SOFTWARE (CPU)");
                    }
                    else if (isHardware) {
                        logDebugInfo("Rendering Method: HARDWARE (GPU)");
                    }
                    else {
                        // If we can't determine from the renderer string,
                        // check the full capabilities string
                        if (capStr.contains("software", Qt::CaseInsensitive) &&
                            !capStr.contains("NVIDIA", Qt::CaseInsensitive) &&
                            !capStr.contains("AMD", Qt::CaseInsensitive) &&
                            !capStr.contains("Intel", Qt::CaseInsensitive)) {
                            logDebugInfo("Rendering Method: Likely SOFTWARE (CPU)");
                        }
                        else {
                            logDebugInfo("Rendering Method: Possibly HARDWARE (GPU)");
                        }
                    }
                }
            }

            // Print OpenGL version if available
            if (capStr.contains("OpenGL version string:")) {
                int startPos = capStr.indexOf("OpenGL version string:") + 23;
                int endPos = capStr.indexOf('\n', startPos);
                if (endPos > startPos) {
                    QString versionStr = capStr.mid(startPos, endPos - startPos).trimmed();
                    logDebugInfo("OpenGL Version: " + versionStr);
                }
            }
        }

        logDebugInfo("-------------------------");
    }
}

void createShape(ShapeType shapeType, double resolution, double radius, QColor color) {
    logDebugInfo(QString("Creating shape type: %1").arg(shapeType));

    // If VTK hasn't been initialized, do nothing (shouldn't happen with our new design)
    if (!g_vtkInitialized) {
        logDebugInfo("VTK not initialized yet");
        return;
    }

    // Remove existing actor if present
    if (g_actor) {
        g_renderer->RemoveActor(g_actor);
    }

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();

    // Create the appropriate shape based on selection
    switch (shapeType) {
    case SHAPE_SPHERE: {
        vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
        sphereSource->SetCenter(0.0, 0.0, 0.0);
        sphereSource->SetRadius(radius);
        sphereSource->SetThetaResolution(resolution);
        sphereSource->SetPhiResolution(resolution);
        sphereSource->Update();
        mapper->SetInputConnection(sphereSource->GetOutputPort());
        break;
    }
    case SHAPE_CONE: {
        vtkSmartPointer<vtkConeSource> coneSource = vtkSmartPointer<vtkConeSource>::New();
        coneSource->SetHeight(radius * 2);
        coneSource->SetRadius(radius);
        coneSource->SetResolution(resolution);
        coneSource->Update();
        mapper->SetInputConnection(coneSource->GetOutputPort());
        break;
    }
    case SHAPE_CYLINDER: {
        vtkSmartPointer<vtkCylinderSource> cylinderSource = vtkSmartPointer<vtkCylinderSource>::New();
        cylinderSource->SetHeight(radius * 2);
        cylinderSource->SetRadius(radius);
        cylinderSource->SetResolution(resolution);
        cylinderSource->Update();
        mapper->SetInputConnection(cylinderSource->GetOutputPort());
        break;
    }
    case SHAPE_CUBE: {
        vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
        cubeSource->SetXLength(radius * 2);
        cubeSource->SetYLength(radius * 2);
        cubeSource->SetZLength(radius * 2);
        cubeSource->Update();
        mapper->SetInputConnection(cubeSource->GetOutputPort());
        break;
    }
    }

    // Create the actor and set its properties
    g_actor = vtkSmartPointer<vtkActor>::New();
    g_actor->SetMapper(mapper);
    g_actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());

    // Add actor to renderer
    g_renderer->AddActor(g_actor);

    // Reset camera and render
    g_renderer->ResetCamera();
    g_renderWindow->Render();
}

void initializeVtkWindow() {
    if (g_vtkInitialized) return;

    logDebugInfo("Initializing VTK window...");

    // Create a renderer
    g_renderer = vtkSmartPointer<vtkRenderer>::New();
    g_renderer->SetBackground(0.1, 0.2, 0.4);

    // Create a render window
    g_renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    g_renderWindow->SetSize(600, 400);
    g_renderWindow->AddRenderer(g_renderer);
    g_renderWindow->SetWindowName("VTK Shape Visualization");

    // Add a callback for end render events to print debug info
    vtkSmartPointer<vtkCallbackCommand> callback = vtkSmartPointer<vtkCallbackCommand>::New();
    callback->SetCallback(::renderCallback);
    g_renderWindow->AddObserver(vtkCommand::EndEvent, callback);

    // Create a render window interactor
    g_renderWindowInteractor = vtkSmartPointer<vtkRenderWindowInteractor>::New();

    // Set interactor style
    vtkSmartPointer<vtkInteractorStyleTrackballCamera> style =
        vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
    g_renderWindowInteractor->SetInteractorStyle(style);

    // Set the render window
    g_renderWindowInteractor->SetRenderWindow(g_renderWindow);

    // Initialize and start rendering
    g_renderWindowInteractor->Initialize();
    g_renderWindow->Render();

    // Print VTK version information
    logDebugInfo(QString("VTK Version: %1.%2.%3")
        .arg(VTK_MAJOR_VERSION)
        .arg(VTK_MINOR_VERSION)
        .arg(VTK_BUILD_VERSION));

    // Mark VTK as initialized
    g_vtkInitialized = true;

    // Start the interactor in a non-blocking way
    g_renderWindowInteractor->CreateRepeatingTimer(10);
    g_renderWindowInteractor->Start();
    g_renderWindowInteractor->EnableRenderOff();

    // Force a render to get capabilities
    g_renderWindow->Render();
}

int main(int argc, char** argv)
{
    // Create Qt application
    QApplication app(argc, argv);

    // Create main Qt window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Qt with VTK Test");
    mainWindow.resize(800, 600);

    // Create central widget and layout
    QWidget* centralWidget = new QWidget(&mainWindow);
    mainWindow.setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Create shape selection controls
    QGroupBox* shapeGroupBox = new QGroupBox("Shape Settings");
    QVBoxLayout* shapeLayout = new QVBoxLayout(shapeGroupBox);

    // Shape type
    QHBoxLayout* shapeTypeLayout = new QHBoxLayout();
    QLabel* shapeTypeLabel = new QLabel("Shape Type:");
    QComboBox* shapeTypeCombo = new QComboBox();
    shapeTypeCombo->addItem("Sphere", SHAPE_SPHERE);
    shapeTypeCombo->addItem("Cone", SHAPE_CONE);
    shapeTypeCombo->addItem("Cylinder", SHAPE_CYLINDER);
    shapeTypeCombo->addItem("Cube", SHAPE_CUBE);
    shapeTypeLayout->addWidget(shapeTypeLabel);
    shapeTypeLayout->addWidget(shapeTypeCombo);
    shapeLayout->addLayout(shapeTypeLayout);

    // Resolution
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    QLabel* resolutionLabel = new QLabel("Resolution:");
    QSpinBox* resolutionSpinBox = new QSpinBox();
    resolutionSpinBox->setRange(3, 100);
    resolutionSpinBox->setValue(20);
    resolutionLayout->addWidget(resolutionLabel);
    resolutionLayout->addWidget(resolutionSpinBox);
    shapeLayout->addLayout(resolutionLayout);

    // Size/Radius
    QHBoxLayout* radiusLayout = new QHBoxLayout();
    QLabel* radiusLabel = new QLabel("Size/Radius:");
    QDoubleSpinBox* radiusSpinBox = new QDoubleSpinBox();
    radiusSpinBox->setRange(0.1, 20.0);
    radiusSpinBox->setValue(5.0);
    radiusSpinBox->setSingleStep(0.5);
    radiusLayout->addWidget(radiusLabel);
    radiusLayout->addWidget(radiusSpinBox);
    shapeLayout->addLayout(radiusLayout);

    // Color
    QHBoxLayout* colorLayout = new QHBoxLayout();
    QLabel* colorLabel = new QLabel("Color:");
    QPushButton* colorButton = new QPushButton("Choose Color");
    QColor currentColor = Qt::red;
    QString buttonStyle = QString("background-color: %1").arg(currentColor.name());
    colorButton->setStyleSheet(buttonStyle);
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorButton);
    shapeLayout->addLayout(colorLayout);

    // Add the group box to main layout
    mainLayout->addWidget(shapeGroupBox);

    // Create a text edit for displaying debug information
    QGroupBox* debugGroupBox = new QGroupBox("Rendering Debug Information");
    QVBoxLayout* debugLayout = new QVBoxLayout(debugGroupBox);
    g_debugTextEdit = new QTextEdit();
    g_debugTextEdit->setReadOnly(true);
    g_debugTextEdit->setMinimumHeight(200);
    debugLayout->addWidget(g_debugTextEdit);
    mainLayout->addWidget(debugGroupBox);

    // Add buttons to check renderer info and print full capabilities
    QPushButton* checkRenderButton = new QPushButton("Check Renderer Information");
    QPushButton* printCapabilitiesButton = new QPushButton("Print Full Renderer Capabilities");
    mainLayout->addWidget(checkRenderButton);
    mainLayout->addWidget(printCapabilitiesButton);

    // Function to update the shape
    auto updateShape = [=, &currentColor]() {
        if (!g_vtkInitialized) {
            // Initialize VTK if not already done
            initializeVtkWindow();
        }

        ShapeType shapeType = static_cast<ShapeType>(shapeTypeCombo->currentData().toInt());
        double resolution = resolutionSpinBox->value();
        double radius = radiusSpinBox->value();

        logDebugInfo(QString("Updating shape to: %1").arg(shapeType));
        createShape(shapeType, resolution, radius, currentColor);
        };

    // Connect the buttons
    QObject::connect(checkRenderButton, &QPushButton::clicked, []() {
        if (g_vtkInitialized && g_renderWindow) {
            // Force a render to trigger the callback
            g_renderWindow->Render();
        }
        else {
            logDebugInfo("VTK not initialized - cannot check renderer");
        }
        });

    QObject::connect(printCapabilitiesButton, &QPushButton::clicked, []() {
        if (g_vtkInitialized && g_renderWindow) {
            const char* capabilities = g_renderWindow->ReportCapabilities();
            if (capabilities) {
                logDebugInfo("--- FULL RENDERER CAPABILITIES ---");
                QString capStr = QString(capabilities);
                QStringList lines = capStr.split("\n");
                for (const QString& line : lines) {
                    if (!line.trimmed().isEmpty()) {
                        logDebugInfo(line.trimmed());
                    }
                }
                logDebugInfo("---------------------------------");
            }
            else {
                logDebugInfo("Could not get renderer capabilities");
            }
        }
        else {
            logDebugInfo("VTK not initialized - cannot print capabilities");
        }
        });

    // Initialize VTK and show initial shape when the main window appears
    mainWindow.installEventFilter(new QObject(&mainWindow));
    QObject::connect(&mainWindow, &QMainWindow::destroyed, [](QObject*) {}); // Dummy connect to prevent warning

    // Use a single-shot timer to initialize after window is shown
    QTimer::singleShot(100, [=]() {
        updateShape();
        });

    // Connect color button
    QObject::connect(colorButton, &QPushButton::clicked, [=, &currentColor]() {
        QColor newColor = QColorDialog::getColor(currentColor, colorButton);
        if (newColor.isValid()) {
            currentColor = newColor;
            QString style = QString("background-color: %1").arg(currentColor.name());
            colorButton->setStyleSheet(style);
            updateShape();
        }
        });

    // Connect shape type combo box change to update shape
    QObject::connect(shapeTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]() {
        updateShape();
        });

    // Connect resolution spin box change to update shape
    QObject::connect(resolutionSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]() {
        updateShape();
        });

    // Connect radius spin box change to update shape
    QObject::connect(radiusSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=]() {
        updateShape();
        });

    // Show Qt window and start Qt event loop
    mainWindow.show();
    logDebugInfo("Qt main window shown, starting Qt event loop");
    return app.exec();
}