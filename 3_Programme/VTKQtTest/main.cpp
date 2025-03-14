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

// Force VTK factory registration
VTK_MODULE_INIT(vtkRenderingOpenGL2);
VTK_MODULE_INIT(vtkInteractionStyle);

// Global smart pointers to prevent premature destruction
vtkSmartPointer<vtkRenderWindow> g_renderWindow;
vtkSmartPointer<vtkRenderWindowInteractor> g_renderWindowInteractor;
vtkSmartPointer<vtkRenderer> g_renderer;
vtkSmartPointer<vtkActor> g_actor;
bool g_vtkInitialized = false;

enum ShapeType {
    SHAPE_SPHERE,
    SHAPE_CONE,
    SHAPE_CYLINDER,
    SHAPE_CUBE
};

void createShape(ShapeType shapeType, double resolution, double radius, QColor color) {
    qDebug() << "Creating shape type:" << shapeType;

    // If VTK hasn't been initialized, do nothing (shouldn't happen with our new design)
    if (!g_vtkInitialized) {
        qDebug() << "VTK not initialized yet";
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

    qDebug() << "Initializing VTK window...";

    // Create a renderer
    g_renderer = vtkSmartPointer<vtkRenderer>::New();
    g_renderer->SetBackground(0.1, 0.2, 0.4);

    // Create a render window
    g_renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    g_renderWindow->SetSize(600, 400);
    g_renderWindow->AddRenderer(g_renderer);
    g_renderWindow->SetWindowName("VTK Shape Visualization");

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

    // Mark VTK as initialized
    g_vtkInitialized = true;

    // Start the interactor in a non-blocking way
    g_renderWindowInteractor->CreateRepeatingTimer(10);
    g_renderWindowInteractor->Start();
    g_renderWindowInteractor->EnableRenderOff();
}

int main(int argc, char** argv)
{
    // Create Qt application
    QApplication app(argc, argv);

    // Create main Qt window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Qt with VTK Test");
    mainWindow.resize(400, 300);

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

    // Function to update the shape
    auto updateShape = [=, &currentColor]() {
        if (!g_vtkInitialized) {
            // Initialize VTK if not already done
            initializeVtkWindow();
        }

        ShapeType shapeType = static_cast<ShapeType>(shapeTypeCombo->currentData().toInt());
        double resolution = resolutionSpinBox->value();
        double radius = radiusSpinBox->value();

        qDebug() << "Updating shape to:" << shapeType;
        createShape(shapeType, resolution, radius, currentColor);
        };

    // Initialize VTK and show initial shape when the main window appears
    // Use the QShowEvent instead of a non-existent windowShown signal
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
    qDebug() << "Qt main window shown, starting Qt event loop";
    return app.exec();
}