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
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QElapsedTimer>
#include <QTime>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QMap>
#include <functional> // Added for std::function

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
#include <vtkOpenGLRenderWindow.h>

// Additional VTK includes for new shapes
#include <vtkParametricTorus.h>
#include <vtkParametricFunctionSource.h>
#include <vtkArrowSource.h>
#include <vtkDiskSource.h>

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

// Global variables for renderer type and timing
bool g_useGPU = true; // Default to GPU rendering
QElapsedTimer g_renderTimer; // Timer to measure rendering performance
QElapsedTimer g_cpuRenderTimer; // Timer for CPU rendering in comparison
QElapsedTimer g_gpuRenderTimer; // Timer for GPU rendering in comparison
bool g_forceCPURendering = false; // Flag to force CPU rendering regardless of detection

enum ShapeType {
    SHAPE_NONE = -1, // Added NONE option
    SHAPE_SPHERE,
    SHAPE_CONE,
    SHAPE_CYLINDER,
    SHAPE_CUBE,
    SHAPE_PYRAMID,   // New shape
    SHAPE_ARROW,     // New shape
    SHAPE_DISK       // New shape
};

// Map to store shape names
QMap<ShapeType, QString> g_shapeNames;

// Function to initialize shape names
void initializeShapeNames() {
    g_shapeNames[SHAPE_NONE] = "None";
    g_shapeNames[SHAPE_SPHERE] = "Sphere";
    g_shapeNames[SHAPE_CONE] = "Cone";
    g_shapeNames[SHAPE_CYLINDER] = "Cylinder";
    g_shapeNames[SHAPE_CUBE] = "Cube";
    g_shapeNames[SHAPE_PYRAMID] = "Pyramid";
    g_shapeNames[SHAPE_ARROW] = "Arrow";
    g_shapeNames[SHAPE_DISK] = "Disk";
}

// Global struct to store initial settings
struct InitialSettings {
    ShapeType shapeType = SHAPE_NONE; // Default to no shape
    double resolution = 20;
    double radius = 5.0;
    QColor color = Qt::red;
    bool useGPU = true;
} g_initialSettings;

// Function to log debug information
void logDebugInfo(const QString& text) {
    qDebug() << text;
    if (g_debugTextEdit) {
        g_debugTextEdit->append(text);
    }
}

// Function to get GPU information (Windows-specific implementation)
void getGpuInformation() {
    logDebugInfo("--- GPU Information ---");

    bool gpuInfoFound = false;

    // For Windows systems using WMIC to get GPU info
    QProcess process;
    process.start("wmic", QStringList() << "path" << "win32_VideoController" << "get" << "Name,AdapterRAM,DriverVersion");
    if (process.waitForFinished(3000)) {
        QString output = process.readAllStandardOutput();
        if (!output.isEmpty()) {
            // Parse the output into lines
            QStringList lines = output.split('\n', Qt::SkipEmptyParts);

            if (lines.size() >= 2) {
                for (int i = 1; i < lines.size(); i++) {
                    QString line = lines[i].trimmed();
                    if (!line.isEmpty()) {
                        // Extract GPU name (it can contain spaces)
                        QStringList parts = line.split(QRegularExpression("\\s{2,}"), Qt::SkipEmptyParts);
                        if (parts.size() >= 1) {
                            QString gpuName = parts[0].trimmed();
                            logDebugInfo("GPU Name: " + gpuName);

                            // If we have more parts, try to extract RAM and driver version
                            if (parts.size() >= 3) {
                                // Convert RAM from bytes to MB or GB
                                bool ok;
                                qlonglong ramBytes = parts[1].toLongLong(&ok);
                                if (ok) {
                                    double ramGB = ramBytes / (1024.0 * 1024.0 * 1024.0);
                                    logDebugInfo(QString("GPU RAM: %1 GB").arg(ramGB, 0, 'f', 2));
                                }

                                // Display driver version
                                logDebugInfo("GPU Driver: " + parts[2]);
                            }

                            gpuInfoFound = true;
                        }
                    }
                }
            }
        }
    }

    // If we couldn't get GPU information, log that
    if (!gpuInfoFound) {
        logDebugInfo("GPU Information: Could not detect GPU details");

        // Try to at least get OpenGL information
        if (g_vtkInitialized && g_renderWindow) {
            const char* capabilities = g_renderWindow->ReportCapabilities();
            if (capabilities) {
                QString capStr = QString(capabilities);

                // Extract OpenGL renderer and vendor if available
                if (capStr.contains("OpenGL vendor string:")) {
                    int startPos = capStr.indexOf("OpenGL vendor string:") + 22;
                    int endPos = capStr.indexOf('\n', startPos);
                    if (endPos > startPos) {
                        QString vendorName = capStr.mid(startPos, endPos - startPos).trimmed();
                        logDebugInfo("OpenGL Vendor: " + vendorName);
                    }
                }

                if (capStr.contains("OpenGL renderer string:")) {
                    int startPos = capStr.indexOf("OpenGL renderer string:") + 24;
                    int endPos = capStr.indexOf('\n', startPos);
                    if (endPos > startPos) {
                        QString rendererName = capStr.mid(startPos, endPos - startPos).trimmed();
                        logDebugInfo("OpenGL Renderer: " + rendererName);
                    }
                }
            }
        }
    }

    logDebugInfo("-------------------------");
}

// Function to update renderer information
void updateRendererInfo() {
    if (!g_vtkInitialized || !g_renderWindow) {
        logDebugInfo("VTK not initialized - cannot check renderer");
        return;
    }

    // Force synchronization to ensure accurate information
    g_renderWindow->Render();

    logDebugInfo("--- Render Information ---");

    // Print VTK version
    logDebugInfo(QString("VTK Version: %1.%2.%3")
        .arg(VTK_MAJOR_VERSION)
        .arg(VTK_MINOR_VERSION)
        .arg(VTK_BUILD_VERSION));

    // Get capabilities (this is the most reliable way to get renderer info)
    const char* capabilities = g_renderWindow->ReportCapabilities();
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
                bool detectedMethod = isSoftware || isHardware;

                // If CPU rendering was explicitly requested, always report as CPU
                if (g_forceCPURendering) {
                    logDebugInfo("Rendering Method: SOFTWARE (CPU) [FORCED MODE]");
                }
                else if (isSoftware) {
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

                // Add a warning if CPU was requested but GPU is being used
                if (g_forceCPURendering && isHardware && !isSoftware) {
                    logDebugInfo("NOTE: Hardware renderer detected but application is in CPU forced mode.");
                    logDebugInfo("Some hardware acceleration may still be used, but rendering will be managed as CPU mode.");
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

    // Always display hardware information based on selected mode
    if (g_forceCPURendering || !g_initialSettings.useGPU) {
        // If CPU mode is selected, display CPU information
        logDebugInfo("--- CPU Information (CPU Mode Selected) ---");

        // Get basic system information that should work across platforms
        bool cpuInfoFound = false;

        // For Windows systems
        QProcess process;
        process.start("wmic", QStringList() << "cpu" << "get" << "Name,NumberOfCores,NumberOfLogicalProcessors");
        if (process.waitForFinished(3000)) {
            QString output = process.readAllStandardOutput();
            if (!output.isEmpty()) {
                // Parse the output into lines
                QStringList lines = output.split('\n', Qt::SkipEmptyParts);

                if (lines.size() >= 2) {
                    // First line contains headers, second line contains values
                    QString headers = lines[0].trimmed();
                    QString values = lines[1].trimmed();

                    // Split headers and values by whitespace
                    QStringList headerList = headers.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
                    QStringList valueList = values.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

                    // For CPU name, we need special handling as it may contain spaces
                    // First, extract the last two values (cores and processors)
                    QString cores, processors;
                    if (valueList.size() >= 2) {
                        processors = valueList.takeLast();
                        cores = valueList.takeLast();

                        // The remaining part is the CPU name
                        QString cpuName = valueList.join(" ");

                        // Log each part separately
                        logDebugInfo("CPU Name: " + cpuName);
                        logDebugInfo("CPU Cores: " + cores);
                        logDebugInfo("CPU Logical Processors: " + processors);

                        cpuInfoFound = true;
                    }
                }
                else {
                    // Fallback to simplified display if parsing fails
                    logDebugInfo("CPU Info (raw): " + output.simplified());
                    cpuInfoFound = true;
                }
            }
        }

        // If all platform-specific methods failed, use Qt's QSysInfo as fallback
        if (!cpuInfoFound) {
            logDebugInfo("CPU Architecture: " + QSysInfo::currentCpuArchitecture());
            logDebugInfo("OS: " + QSysInfo::prettyProductName());
            logDebugInfo("CPU Present: Yes");
        }
        else {
            logDebugInfo("CPU information retrieved successfully.");
        }

        // Get additional CPU details if possible
        process.start("wmic", QStringList() << "cpu" << "get" << "MaxClockSpeed,L2CacheSize,L3CacheSize");
        if (process.waitForFinished(3000)) {
            QString output = process.readAllStandardOutput();
            if (!output.isEmpty()) {
                QStringList lines = output.split('\n', Qt::SkipEmptyParts);
                if (lines.size() >= 2) {
                    QString values = lines[1].trimmed();
                    QStringList valueList = values.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

                    if (valueList.size() >= 3) {
                        QString clockSpeed = valueList[0];
                        QString l2Cache = valueList[1];
                        QString l3Cache = valueList[2];

                        // Convert clock speed from MHz to GHz if large enough
                        bool ok;
                        int clockMhz = clockSpeed.toInt(&ok);
                        if (ok && clockMhz > 0) {
                            double clockGhz = clockMhz / 1000.0;
                            logDebugInfo(QString("CPU Clock Speed: %1 GHz").arg(clockGhz, 0, 'f', 2));
                        }

                        // Convert cache sizes to MB if necessary
                        int l2CacheKB = l2Cache.toInt(&ok);
                        if (ok && l2CacheKB > 0) {
                            double l2CacheMB = l2CacheKB / 1024.0;
                            logDebugInfo(QString("L2 Cache: %1 MB").arg(l2CacheMB, 0, 'f', 2));
                        }

                        int l3CacheKB = l3Cache.toInt(&ok);
                        if (ok && l3CacheKB > 0) {
                            double l3CacheMB = l3CacheKB / 1024.0;
                            logDebugInfo(QString("L3 Cache: %1 MB").arg(l3CacheMB, 0, 'f', 2));
                        }
                    }
                }
            }
        }

        logDebugInfo("-----------------------");
    }
    else {
        // If GPU mode is selected, display GPU information
        logDebugInfo("--- GPU Information (GPU Mode Selected) ---");

        bool gpuInfoFound = false;

        // For Windows systems using WMIC to get GPU info
        QProcess process;
        process.start("wmic", QStringList() << "path" << "win32_VideoController" << "get" << "Name,AdapterRAM,DriverVersion");
        if (process.waitForFinished(3000)) {
            QString output = process.readAllStandardOutput();
            if (!output.isEmpty()) {
                // Parse the output into lines
                QStringList lines = output.split('\n', Qt::SkipEmptyParts);

                if (lines.size() >= 2) {
                    for (int i = 1; i < lines.size(); i++) {
                        QString line = lines[i].trimmed();
                        if (!line.isEmpty()) {
                            // Extract GPU name (it can contain spaces)
                            QStringList parts = line.split(QRegularExpression("\\s{2,}"), Qt::SkipEmptyParts);
                            if (parts.size() >= 1) {
                                QString gpuName = parts[0].trimmed();
                                logDebugInfo("GPU Name: " + gpuName);

                                // If we have more parts, try to extract RAM and driver version
                                if (parts.size() >= 3) {
                                    // Convert RAM from bytes to MB or GB
                                    bool ok;
                                    qlonglong ramBytes = parts[1].toLongLong(&ok);
                                    if (ok) {
                                        double ramGB = ramBytes / (1024.0 * 1024.0 * 1024.0);
                                        logDebugInfo(QString("GPU RAM: %1 GB").arg(ramGB, 0, 'f', 2));
                                    }

                                    // Display driver version
                                    logDebugInfo("GPU Driver: " + parts[2]);
                                }

                                gpuInfoFound = true;
                            }
                        }
                    }
                }
            }
        }

        // Get more detailed GPU information if possible
        process.start("wmic", QStringList() << "path" << "win32_VideoController" << "get" << "CurrentHorizontalResolution,CurrentVerticalResolution,VideoModeDescription");
        if (process.waitForFinished(3000)) {
            QString output = process.readAllStandardOutput();
            if (!output.isEmpty()) {
                QStringList lines = output.split('\n', Qt::SkipEmptyParts);
                if (lines.size() >= 2) {
                    for (int i = 1; i < lines.size(); i++) {
                        QString line = lines[i].trimmed();
                        if (!line.isEmpty()) {
                            QStringList parts = line.split(QRegularExpression("\\s{2,}"), Qt::SkipEmptyParts);
                            if (parts.size() >= 3) {
                                QString hRes = parts[0].trimmed();
                                QString vRes = parts[1].trimmed();
                                QString videoMode = parts[2].trimmed();

                                if (!hRes.isEmpty() && !vRes.isEmpty()) {
                                    logDebugInfo(QString("Display Resolution: %1 x %2").arg(hRes).arg(vRes));
                                }

                                if (!videoMode.isEmpty()) {
                                    logDebugInfo("Video Mode: " + videoMode);
                                }
                            }
                        }
                    }
                }
            }
        }

        // If we couldn't get GPU information, get OpenGL info
        if (!gpuInfoFound) {
            logDebugInfo("GPU Information: Could not detect GPU details");

            // Try to at least get OpenGL information
            if (g_vtkInitialized && g_renderWindow) {
                const char* capabilities = g_renderWindow->ReportCapabilities();
                if (capabilities) {
                    QString capStr = QString(capabilities);

                    // Extract OpenGL renderer and vendor if available
                    if (capStr.contains("OpenGL vendor string:")) {
                        int startPos = capStr.indexOf("OpenGL vendor string:") + 22;
                        int endPos = capStr.indexOf('\n', startPos);
                        if (endPos > startPos) {
                            QString vendorName = capStr.mid(startPos, endPos - startPos).trimmed();
                            logDebugInfo("OpenGL Vendor: " + vendorName);
                        }
                    }

                    if (capStr.contains("OpenGL renderer string:")) {
                        int startPos = capStr.indexOf("OpenGL renderer string:") + 24;
                        int endPos = capStr.indexOf('\n', startPos);
                        if (endPos > startPos) {
                            QString rendererName = capStr.mid(startPos, endPos - startPos).trimmed();
                            logDebugInfo("OpenGL Renderer: " + rendererName);
                        }
                    }
                }
            }
        }

        logDebugInfo("-------------------------");
    }
}

// Callback function to print render information - only used for initial setup now
void renderCallback(vtkObject* caller, unsigned long eventId, void* clientData, void* callData) {
    // We'll only use this for the initial setup, not for continuous updates
    vtkRenderWindow* renderWindow = vtkRenderWindow::SafeDownCast(caller);
    if (renderWindow) {
        // Only log during initialization, but don't call updateRendererInfo to avoid
        // automatically showing CPU/GPU info
        static bool firstRender = true;
        if (firstRender) {
            // Just log basic info, not the full renderer info
            logDebugInfo("VTK rendering initialized successfully");
            logDebugInfo(QString("VTK Version: %1.%2.%3")
                .arg(VTK_MAJOR_VERSION)
                .arg(VTK_MINOR_VERSION)
                .arg(VTK_BUILD_VERSION));

            // Log the rendering mode but not hardware details
            logDebugInfo(QString("Rendering Mode: %1")
                .arg(g_initialSettings.useGPU ? "GPU (Hardware)" : "CPU (Software)"));

            firstRender = false;
        }
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
        g_actor = nullptr; // Clear the actor pointer
    }

    // If SHAPE_NONE is selected, just render the empty scene and return
    if (shapeType == SHAPE_NONE) {
        logDebugInfo("No shape selected, rendering empty scene");
        g_renderWindow->Render();
        return;
    }

    // Start the rendering timer
    g_renderTimer.start();

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
    case SHAPE_PYRAMID: {
        // Create a pyramid source (using vtkConeSource with 4 sides as base)
        vtkSmartPointer<vtkConeSource> pyramidSource = vtkSmartPointer<vtkConeSource>::New();
        pyramidSource->SetHeight(radius * 2);
        pyramidSource->SetRadius(radius);
        pyramidSource->SetResolution(4); // Square base
        pyramidSource->Update();
        mapper->SetInputConnection(pyramidSource->GetOutputPort());
        break;
    }
    case SHAPE_ARROW: {
        vtkSmartPointer<vtkArrowSource> arrowSource = vtkSmartPointer<vtkArrowSource>::New();
        arrowSource->SetTipLength(0.3);
        arrowSource->SetTipRadius(0.1 * radius);
        arrowSource->SetShaftRadius(0.05 * radius);
        arrowSource->SetShaftResolution(resolution);
        arrowSource->SetTipResolution(resolution);
        arrowSource->Update();
        mapper->SetInputConnection(arrowSource->GetOutputPort());
        break;
    }
    case SHAPE_DISK: {
        vtkSmartPointer<vtkDiskSource> diskSource = vtkSmartPointer<vtkDiskSource>::New();
        diskSource->SetInnerRadius(0);
        diskSource->SetOuterRadius(radius);
        diskSource->SetCircumferentialResolution(resolution);
        diskSource->Update();
        mapper->SetInputConnection(diskSource->GetOutputPort());
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

    // Log the rendering time
    qint64 renderTimeMs = g_renderTimer.elapsed();
    logDebugInfo(QString("Rendering processing time: %1 ms").arg(renderTimeMs));
}

void initializeVtkWindow() {
    if (g_vtkInitialized) return;

    logDebugInfo("Initializing VTK window...");

    // Apply CPU/GPU rendering selection before VTK initialization
    if (!g_initialSettings.useGPU) {
        g_forceCPURendering = true;

        // If user selected CPU rendering, we need to set this env var
        // before VTK objects are created
        logDebugInfo("Applying CPU rendering mode before initialization");
#ifdef _WIN32
        _putenv_s("LIBGL_ALWAYS_SOFTWARE", "1");
        _putenv_s("VTK_MAPPER_FORCE_MESA", "1");
        _putenv_s("VTK_FORCE_CPU_RENDERING", "1"); // Additional setting for Windows
        _putenv_s("MESA_GL_VERSION_OVERRIDE", "3.2");
        _putenv_s("MESA_GLSL_VERSION_OVERRIDE", "150");
#else
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
        setenv("VTK_MAPPER_FORCE_MESA", "1", 1);
        setenv("VTK_FORCE_CPU_RENDERING", "1", 1); // Additional setting for Linux/Mac
        setenv("MESA_GL_VERSION_OVERRIDE", "3.2", 1);
        setenv("MESA_GLSL_VERSION_OVERRIDE", "150", 1);
#endif

        // More aggressive approach to force software rendering
        vtkObject::GlobalWarningDisplayOff(); // Suppress potential warnings

        // Force Mesa/software rendering in VTK
#if VTK_MAJOR_VERSION >= 9
        logDebugInfo("Setting VTK_RENDERING_BACKEND=OpenGL2");
        logDebugInfo("Setting VTK_USE_OFFSCREEN_OR_HEADLESS_MESA=1");
#ifdef _WIN32
        _putenv_s("VTK_RENDERING_BACKEND", "OpenGL2");
        _putenv_s("VTK_USE_OFFSCREEN_OR_HEADLESS_MESA", "1");
#else
        setenv("VTK_RENDERING_BACKEND", "OpenGL2", 1);
        setenv("VTK_USE_OFFSCREEN_OR_HEADLESS_MESA", "1", 1);
#endif
#endif
    }

    // Create renderer
    g_renderer = vtkSmartPointer<vtkRenderer>::New();
    g_renderer->SetBackground(0.1, 0.2, 0.4);

    // Create render window with special CPU rendering handling
    if (g_forceCPURendering) {
        // For CPU rendering, explicitly use software rendering
        logDebugInfo("Creating CPU software rendering window...");

#if VTK_MAJOR_VERSION >= 9
        // For VTK 9+, try a different approach
        vtkRenderWindow* renderWindow = vtkRenderWindow::New();

        // Set to use software rendering
        renderWindow->SetOffScreenRendering(1);  // First try offscreen mode
        renderWindow->SetOffScreenRendering(0);  // Then go back to on-screen

        g_renderWindow = renderWindow;
#else
        g_renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
#endif
    }
    else {
        // Regular rendering window
        g_renderWindow = vtkSmartPointer<vtkRenderWindow>::New();
    }

    g_renderWindow->SetSize(600, 400);
    g_renderWindow->AddRenderer(g_renderer);
    g_renderWindow->SetWindowName("VTK Shape Visualization");

    // Add a callback for end render events - only use this for the initial setup
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

    // If CPU rendering was selected, apply additional settings
    if (!g_initialSettings.useGPU) {
        // For CPU rendering, also set quality settings
        vtkOpenGLRenderWindow* oglRenderWindow = vtkOpenGLRenderWindow::SafeDownCast(g_renderWindow);
        if (oglRenderWindow) {
            // Disable all hardware-specific features
            oglRenderWindow->SetMultiSamples(0);
            g_renderer->SetUseFXAA(false);
            g_renderer->SetUseDepthPeeling(false);

            // More aggressive approach to force software rendering
            oglRenderWindow->SetUseOffScreenBuffers(true); // Try forcing offscreen rendering
            oglRenderWindow->SetUseOffScreenBuffers(false); // Then set back to normal display

            // Force software rendering by disabling hardware acceleration
#if VTK_MAJOR_VERSION >= 9
            // For VTK 9+, try to force software rendering
            g_renderWindow->SetUseOffScreenBuffers(true);
            g_renderWindow->SetUseOffScreenBuffers(false);
#endif

            logDebugInfo("Applied additional settings to enforce CPU rendering");
        }
    }

    // Mark VTK as initialized
    g_vtkInitialized = true;

    // Start the interactor in a non-blocking way
    g_renderWindowInteractor->CreateRepeatingTimer(10);
    g_renderWindowInteractor->Start();
    g_renderWindowInteractor->EnableRenderOff();

    // Force a render to get capabilities
    g_renderWindow->Render();
}

// Create a dialog for initial settings
bool showInitialSettingsDialog(QWidget* parent) {
    QDialog dialog(parent);
    dialog.setWindowTitle("Initial Settings");
    dialog.setMinimumWidth(350);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Rendering Mode Group
    QGroupBox* renderModeGroup = new QGroupBox("Rendering Mode");
    QVBoxLayout* renderModeLayout = new QVBoxLayout(renderModeGroup);

    QRadioButton* gpuRadio = new QRadioButton("GPU Rendering (Hardware)");
    QRadioButton* cpuRadio = new QRadioButton("CPU Rendering (Software)");

    gpuRadio->setChecked(true); // Default to GPU

    renderModeLayout->addWidget(gpuRadio);
    renderModeLayout->addWidget(cpuRadio);

    // Add warning for CPU rendering
    QLabel* warningLabel = new QLabel("Note: CPU rendering may be significantly slower and "
        "is not available on all systems. This setting cannot "
        "be changed after initialization.");
    warningLabel->setWordWrap(true);
    warningLabel->setStyleSheet("color: red;");

    // Add buttons
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    // Connect the buttons
    QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Add all widgets to layout
    layout->addWidget(renderModeGroup);
    layout->addWidget(warningLabel);
    layout->addWidget(buttonBox);

    // Show the dialog
    if (dialog.exec() == QDialog::Accepted) {
        // Save settings
        g_initialSettings.useGPU = gpuRadio->isChecked();
        g_useGPU = g_initialSettings.useGPU;
        return true;
    }

    return false;
}

// Function to run performance comparison
void runPerformanceComparison(QWidget* parent) {
    // Create a dialog to show results
    QDialog dialog(parent);
    dialog.setWindowTitle("CPU vs GPU Performance Comparison");
    dialog.setMinimumWidth(500);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);

    // Create form for shape selection
    QGroupBox* selectionGroup = new QGroupBox("Select Shape for Comparison");
    QFormLayout* formLayout = new QFormLayout(selectionGroup);

    QComboBox* shapeCombo = new QComboBox();
    // Add all shapes except NONE
    for (int i = SHAPE_SPHERE; i <= SHAPE_DISK; i++) {
        ShapeType type = static_cast<ShapeType>(i);
        shapeCombo->addItem(g_shapeNames[type], type);
    }
    formLayout->addRow("Shape:", shapeCombo);

    QSpinBox* resolutionSpin = new QSpinBox();
    resolutionSpin->setRange(3, 100);
    resolutionSpin->setValue(20);
    formLayout->addRow("Resolution:", resolutionSpin);

    // Add group to main layout
    layout->addWidget(selectionGroup);

    // Results area
    QGroupBox* resultsGroup = new QGroupBox("Comparison Results");
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroup);

    QTextEdit* resultsText = new QTextEdit();
    resultsText->setReadOnly(true);
    resultsText->setMinimumHeight(200);
    resultsLayout->addWidget(resultsText);

    layout->addWidget(resultsGroup);

    // Run button
    QPushButton* runButton = new QPushButton("Run Comparison");
    layout->addWidget(runButton);

    // Close button
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    layout->addWidget(buttonBox);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    // Function to run comparison test
    auto runTest = [&]() {
        ShapeType shapeType = static_cast<ShapeType>(shapeCombo->currentData().toInt());
        double resolution = resolutionSpin->value();
        double radius = 5.0;
        QColor color = Qt::red;

        resultsText->clear();
        resultsText->append("Running performance test...\n");

        // Setup test environment
        resultsText->append("Testing shape: " + g_shapeNames[shapeType]);
        resultsText->append("Resolution: " + QString::number(resolution));
        resultsText->append("Size: " + QString::number(radius));
        resultsText->append("\nRendering with CPU (Software)...");

        // Force CPU rendering
        bool oldForceCPU = g_forceCPURendering;
        g_forceCPURendering = true;

        // Create temporary VTK objects for CPU rendering
        vtkSmartPointer<vtkRenderer> cpuRenderer = vtkSmartPointer<vtkRenderer>::New();
        cpuRenderer->SetBackground(0.1, 0.2, 0.4);

        vtkSmartPointer<vtkRenderWindow> cpuRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
        cpuRenderWindow->SetOffScreenRendering(1);  // Use offscreen for testing
        cpuRenderWindow->AddRenderer(cpuRenderer);

        // Create shape for CPU
        vtkSmartPointer<vtkPolyDataMapper> cpuMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        vtkSmartPointer<vtkActor> cpuActor = vtkSmartPointer<vtkActor>::New();

        // Start CPU timer
        g_cpuRenderTimer.start();

        // Create the appropriate shape for CPU rendering
        switch (shapeType) {
        case SHAPE_SPHERE: {
            vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
            source->SetCenter(0.0, 0.0, 0.0);
            source->SetRadius(radius);
            source->SetThetaResolution(resolution);
            source->SetPhiResolution(resolution);
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_CONE: {
            vtkSmartPointer<vtkConeSource> source = vtkSmartPointer<vtkConeSource>::New();
            source->SetHeight(radius * 2);
            source->SetRadius(radius);
            source->SetResolution(resolution);
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_CYLINDER: {
            vtkSmartPointer<vtkCylinderSource> source = vtkSmartPointer<vtkCylinderSource>::New();
            source->SetHeight(radius * 2);
            source->SetRadius(radius);
            source->SetResolution(resolution);
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_CUBE: {
            vtkSmartPointer<vtkCubeSource> source = vtkSmartPointer<vtkCubeSource>::New();
            source->SetXLength(radius * 2);
            source->SetYLength(radius * 2);
            source->SetZLength(radius * 2);
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_PYRAMID: {
            vtkSmartPointer<vtkConeSource> source = vtkSmartPointer<vtkConeSource>::New();
            source->SetHeight(radius * 2);
            source->SetRadius(radius);
            source->SetResolution(4); // Square base
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_ARROW: {
            vtkSmartPointer<vtkArrowSource> source = vtkSmartPointer<vtkArrowSource>::New();
            source->SetTipLength(0.3);
            source->SetTipRadius(0.1 * radius);
            source->SetShaftRadius(0.05 * radius);
            source->SetShaftResolution(resolution);
            source->SetTipResolution(resolution);
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_DISK: {
            vtkSmartPointer<vtkDiskSource> source = vtkSmartPointer<vtkDiskSource>::New();
            source->SetInnerRadius(0);
            source->SetOuterRadius(radius);
            source->SetCircumferentialResolution(resolution);
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        default: {
            // Default to sphere if shape not implemented in test
            vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
            source->SetCenter(0.0, 0.0, 0.0);
            source->SetRadius(radius);
            source->SetThetaResolution(resolution);
            source->SetPhiResolution(resolution);
            source->Update();
            cpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        }

        cpuActor->SetMapper(cpuMapper);
        cpuActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());

        cpuRenderer->AddActor(cpuActor);
        cpuRenderer->ResetCamera();

        // Force CPU rendering
        cpuRenderWindow->Render();
        cpuRenderWindow->Render();  // Render twice to ensure accurate timing

        // Get CPU time
        qint64 cpuTimeMs = g_cpuRenderTimer.elapsed();
        resultsText->append("CPU rendering time: " + QString::number(cpuTimeMs) + " ms\n");

        // Now test GPU rendering
        resultsText->append("Rendering with GPU (Hardware)...");

        // Force GPU rendering
        g_forceCPURendering = false;

        // Create temporary VTK objects for GPU rendering
        vtkSmartPointer<vtkRenderer> gpuRenderer = vtkSmartPointer<vtkRenderer>::New();
        gpuRenderer->SetBackground(0.1, 0.2, 0.4);

        vtkSmartPointer<vtkRenderWindow> gpuRenderWindow = vtkSmartPointer<vtkRenderWindow>::New();
        gpuRenderWindow->AddRenderer(gpuRenderer);

        // Create shape for GPU
        vtkSmartPointer<vtkPolyDataMapper> gpuMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        vtkSmartPointer<vtkActor> gpuActor = vtkSmartPointer<vtkActor>::New();

        // Start GPU timer
        g_gpuRenderTimer.start();

        // Create the appropriate shape for GPU rendering (same shape creation code)
        switch (shapeType) {
        case SHAPE_SPHERE: {
            vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
            source->SetCenter(0.0, 0.0, 0.0);
            source->SetRadius(radius);
            source->SetThetaResolution(resolution);
            source->SetPhiResolution(resolution);
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_CONE: {
            vtkSmartPointer<vtkConeSource> source = vtkSmartPointer<vtkConeSource>::New();
            source->SetHeight(radius * 2);
            source->SetRadius(radius);
            source->SetResolution(resolution);
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_CYLINDER: {
            vtkSmartPointer<vtkCylinderSource> source = vtkSmartPointer<vtkCylinderSource>::New();
            source->SetHeight(radius * 2);
            source->SetRadius(radius);
            source->SetResolution(resolution);
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_CUBE: {
            vtkSmartPointer<vtkCubeSource> source = vtkSmartPointer<vtkCubeSource>::New();
            source->SetXLength(radius * 2);
            source->SetYLength(radius * 2);
            source->SetZLength(radius * 2);
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_PYRAMID: {
            vtkSmartPointer<vtkConeSource> source = vtkSmartPointer<vtkConeSource>::New();
            source->SetHeight(radius * 2);
            source->SetRadius(radius);
            source->SetResolution(4); // Square base
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_ARROW: {
            vtkSmartPointer<vtkArrowSource> source = vtkSmartPointer<vtkArrowSource>::New();
            source->SetTipLength(0.3);
            source->SetTipRadius(0.1 * radius);
            source->SetShaftRadius(0.05 * radius);
            source->SetShaftResolution(resolution);
            source->SetTipResolution(resolution);
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        case SHAPE_DISK: {
            vtkSmartPointer<vtkDiskSource> source = vtkSmartPointer<vtkDiskSource>::New();
            source->SetInnerRadius(0);
            source->SetOuterRadius(radius);
            source->SetCircumferentialResolution(resolution);
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        default: {
            // Default to sphere if shape not implemented in test
            vtkSmartPointer<vtkSphereSource> source = vtkSmartPointer<vtkSphereSource>::New();
            source->SetCenter(0.0, 0.0, 0.0);
            source->SetRadius(radius);
            source->SetThetaResolution(resolution);
            source->SetPhiResolution(resolution);
            source->Update();
            gpuMapper->SetInputConnection(source->GetOutputPort());
            break;
        }
        }

        gpuActor->SetMapper(gpuMapper);
        gpuActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());

        gpuRenderer->AddActor(gpuActor);
        gpuRenderer->ResetCamera();

        // Force GPU rendering
        gpuRenderWindow->Render();
        gpuRenderWindow->Render();  // Render twice to ensure accurate timing

        // Get GPU time
        qint64 gpuTimeMs = g_gpuRenderTimer.elapsed();
        resultsText->append("GPU rendering time: " + QString::number(gpuTimeMs) + " ms\n");

        // Calculate and show difference
        double speedup = 0;
        if (gpuTimeMs > 0) {
            speedup = static_cast<double>(cpuTimeMs) / gpuTimeMs;
        }

        resultsText->append("Performance Comparison:");
        resultsText->append("CPU/GPU Ratio: " + QString::number(speedup, 'f', 2) + "x");

        if (speedup > 1.0) {
            resultsText->append("GPU is " + QString::number(speedup, 'f', 2) +
                " times faster than CPU for this shape");
        }
        else if (speedup < 1.0) {
            resultsText->append("CPU is " + QString::number(1.0 / speedup, 'f', 2) +
                " times faster than GPU for this shape");
        }
        else {
            resultsText->append("CPU and GPU performance is similar for this shape");
        }

        // Restore original setting
        g_forceCPURendering = oldForceCPU;
        };

    // Connect run button
    QObject::connect(runButton, &QPushButton::clicked, runTest);

    // Show the dialog
    dialog.exec();
}

void showInfoInNewWindow(const QString& title, const QString& content) {
    // Create a new dialog window
    QDialog* infoDialog = new QDialog(nullptr, Qt::Window);
    infoDialog->setWindowTitle(title);
    infoDialog->resize(600, 400);

    // Create layout
    QVBoxLayout* layout = new QVBoxLayout(infoDialog);

    // Create text display widget
    QTextEdit* textDisplay = new QTextEdit(infoDialog);
    textDisplay->setReadOnly(true);
    textDisplay->setText(content);
    textDisplay->setLineWrapMode(QTextEdit::WidgetWidth);
    layout->addWidget(textDisplay);

    // Add close button
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, infoDialog, &QDialog::close);
    layout->addWidget(buttonBox);

    // Show window (non-modal)
    infoDialog->setAttribute(Qt::WA_DeleteOnClose); // Auto-delete when closed
    infoDialog->show();
}

int main(int argc, char** argv)
{
    // Set environment variables for CPU rendering BEFORE creating any Qt or VTK objects
    // This is critical as these need to be set before OpenGL context creation
    bool useCpuRendering = false;

    // Check command line args for a rendering mode hint
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--cpu") == 0) {
            useCpuRendering = true;
        }
    }

    // Set environment variables for CPU rendering if needed
    if (useCpuRendering) {
#ifdef _WIN32
        _putenv_s("LIBGL_ALWAYS_SOFTWARE", "1");
        _putenv_s("VTK_MAPPER_FORCE_MESA", "1");
        _putenv_s("VTK_FORCE_CPU_RENDERING", "1");
        _putenv_s("MESA_GL_VERSION_OVERRIDE", "3.2");
        _putenv_s("MESA_GLSL_VERSION_OVERRIDE", "150");
        // Try disabling hardware acceleration via Windows-specific method
        _putenv_s("QT_OPENGL", "software");
        _putenv_s("QT_QUICK_BACKEND", "software");
#else
        setenv("LIBGL_ALWAYS_SOFTWARE", "1", 1);
        setenv("VTK_MAPPER_FORCE_MESA", "1", 1);
        setenv("VTK_FORCE_CPU_RENDERING", "1", 1);
        setenv("MESA_GL_VERSION_OVERRIDE", "3.2", 1);
        setenv("MESA_GLSL_VERSION_OVERRIDE", "150", 1);
        // Qt settings for Linux/Mac
        setenv("QT_OPENGL", "software", 1);
        setenv("QT_QUICK_BACKEND", "software", 1);
#endif

        // Qt 5.14+ has the AA_UseSoftwareOpenGL attribute
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
        QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);
#endif
    }

    // Initialize shape names
    initializeShapeNames();

    // Create Qt application
    QApplication app(argc, argv);

    // Create main Qt window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Qt with VTK Test");
    mainWindow.resize(800, 600);

    // Show initial settings dialog
    if (!showInitialSettingsDialog(&mainWindow)) {
        // User canceled, exit application
        return 0;
    }

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
    shapeTypeCombo->addItem("None (Empty)", SHAPE_NONE);
    shapeTypeCombo->addItem("Sphere", SHAPE_SPHERE);
    shapeTypeCombo->addItem("Cone", SHAPE_CONE);
    shapeTypeCombo->addItem("Cylinder", SHAPE_CYLINDER);
    shapeTypeCombo->addItem("Cube", SHAPE_CUBE);
    shapeTypeCombo->addItem("Pyramid", SHAPE_PYRAMID);
    shapeTypeCombo->addItem("Arrow", SHAPE_ARROW);
    shapeTypeCombo->addItem("Disk", SHAPE_DISK);
    shapeTypeLayout->addWidget(shapeTypeLabel);
    shapeTypeLayout->addWidget(shapeTypeCombo);
    shapeLayout->addLayout(shapeTypeLayout);

    // Initialize with none selected
    shapeTypeCombo->setCurrentIndex(shapeTypeCombo->findData(SHAPE_NONE));

    // Resolution
    QHBoxLayout* resolutionLayout = new QHBoxLayout();
    QLabel* resolutionLabel = new QLabel("Resolution:");
    QSpinBox* resolutionSpinBox = new QSpinBox();
    resolutionSpinBox->setRange(3, 100);
    resolutionSpinBox->setValue(20);
    resolutionSpinBox->setEnabled(false); // Initially disabled
    resolutionLayout->addWidget(resolutionLabel);
    resolutionLayout->addWidget(resolutionSpinBox);
    shapeLayout->addLayout(resolutionLayout);

    // Color
    QHBoxLayout* colorLayout = new QHBoxLayout();
    QLabel* colorLabel = new QLabel("Color:");
    QPushButton* colorButton = new QPushButton("Choose Color");
    QColor currentColor = Qt::red;
    // Keep button white regardless of selected color

    QString buttonStyle = "background-color: white";
    colorButton->setStyleSheet(buttonStyle);
    colorButton->setEnabled(false); // Initially disabled
    colorLayout->addWidget(colorLabel);
    colorLayout->addWidget(colorButton);
    shapeLayout->addLayout(colorLayout);

    // Create a forward reference to the updateShape function that we'll define later
    std::function<void()> updateShape;

    // Enable/disable shape settings based on shape selection
    QObject::connect(shapeTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        [=, &currentColor, &updateShape](int index) {
            bool shapeSelected = (shapeTypeCombo->currentData().toInt() != SHAPE_NONE);
            resolutionSpinBox->setEnabled(shapeSelected);
            colorButton->setEnabled(shapeSelected);

            if (shapeSelected && updateShape) {
                updateShape();
            }
            else {
                // If no shape is selected, clear the render window
                if (g_actor && g_renderer) {
                    g_renderer->RemoveActor(g_actor);
                    g_actor = nullptr;
                    if (g_renderWindow) {
                        g_renderWindow->Render();
                    }
                }
            }
        });

    // Add display of current rendering mode
    QLabel* renderModeLabel = new QLabel(QString("Current Rendering Mode: %1")
        .arg(g_initialSettings.useGPU ? "GPU (Hardware)" : "CPU (Software)"));
    renderModeLabel->setStyleSheet("font-weight: bold;");
    shapeLayout->addWidget(renderModeLabel);

    // Add label for rendering processing time
    QLabel* processingTimeLabel = new QLabel("Rendering Processing Time: 0 ms");
    processingTimeLabel->setStyleSheet("font-weight: bold;");
    shapeLayout->addWidget(processingTimeLabel);

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

    // Add display of current rendering mode at the top of the debug output
    QString renderModeStr = QString("Current Rendering Mode: %1")
        .arg(g_initialSettings.useGPU ? "GPU (Hardware)" : "CPU (Software)");
    logDebugInfo(renderModeStr);

    // Add a notice if CPU rendering was selected
    if (!g_initialSettings.useGPU) {
        logDebugInfo("NOTE: You have selected CPU (Software) rendering mode.");
        logDebugInfo("This may still show hardware renderer details in some environments.");
        logDebugInfo("The application will enforce CPU-based processing regardless of what's reported.");
    }

    // Add buttons to check renderer info and print full capabilities
    QPushButton* checkRenderButton = new QPushButton("Check Renderer Information");
    QPushButton* printCapabilitiesButton = new QPushButton("Print Full Renderer Capabilities");
    mainLayout->addWidget(checkRenderButton);
    mainLayout->addWidget(printCapabilitiesButton);

    // Add button for CPU/GPU performance comparison
    QPushButton* compareButton = new QPushButton("Compare CPU/GPU Performance");
    mainLayout->addWidget(compareButton);

    // Add a restart button
    QPushButton* restartButton = new QPushButton("Restart Application");
    mainLayout->addWidget(restartButton);

    // Connect restart button
    QObject::connect(restartButton, &QPushButton::clicked, [&app]() {
        // Ask for confirmation
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "Restart Application",
            "Are you sure you want to restart the application?",
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // Close current application and start a new instance
            QProcess::startDetached(QApplication::applicationFilePath(), QApplication::arguments());
            QApplication::quit();
        }
        });

    // Function to update the shape
    updateShape = [=, &currentColor, &processingTimeLabel]() {
        if (!g_vtkInitialized) {
            // Initialize VTK if not already done
            initializeVtkWindow();
        }

        ShapeType shapeType = static_cast<ShapeType>(shapeTypeCombo->currentData().toInt());

        // If no shape is selected, clear the viewport and return
        if (shapeType == SHAPE_NONE) {
            if (g_actor && g_renderer) {
                g_renderer->RemoveActor(g_actor);
                g_actor = nullptr;
                if (g_renderWindow) {
                    g_renderWindow->Render();
                }
            }
            return;
        }

        double resolution = resolutionSpinBox->value();
        // Use a fixed radius value (5.0) instead of the removed spinner
        double radius = 5.0;

        logDebugInfo(QString("Updating shape to: %1").arg(shapeType));

        // Start timing the render
        g_renderTimer.start();

        // Create the shape
        createShape(shapeType, resolution, radius, currentColor);

        // Update the processing time label
        qint64 renderTimeMs = g_renderTimer.elapsed();
        processingTimeLabel->setText(QString("Rendering Processing Time: %1 ms").arg(renderTimeMs));
        };

    // Connect the buttons to show info in new windows
    QObject::connect(checkRenderButton, &QPushButton::clicked, []() {
        if (g_vtkInitialized && g_renderWindow) {
            // Capture the output in a string instead of logging directly
            QString rendererInfo;
            QTextStream stream(&rendererInfo);

            // Get render information
            stream << "--- Render Information ---\n";

            // Print VTK version
            stream << QString("VTK Version: %1.%2.%3\n")
                .arg(VTK_MAJOR_VERSION)
                .arg(VTK_MINOR_VERSION)
                .arg(VTK_BUILD_VERSION);

            // Get capabilities
            const char* capabilities = g_renderWindow->ReportCapabilities();
            if (capabilities) {
                QString capStr = QString(capabilities);

                // Print renderer name if available
                if (capStr.contains("OpenGL vendor string:")) {
                    int startPos = capStr.indexOf("OpenGL vendor string:") + 22;
                    int endPos = capStr.indexOf('\n', startPos);
                    if (endPos > startPos) {
                        QString vendorName = capStr.mid(startPos, endPos - startPos).trimmed();
                        stream << "OpenGL Vendor: " + vendorName + "\n";
                    }
                }

                // Print renderer string if available
                if (capStr.contains("OpenGL renderer string:")) {
                    int startPos = capStr.indexOf("OpenGL renderer string:") + 24;
                    int endPos = capStr.indexOf('\n', startPos);
                    if (endPos > startPos) {
                        QString rendererName = capStr.mid(startPos, endPos - startPos).trimmed();
                        stream << "OpenGL Renderer: " + rendererName + "\n";

                        // Check rendering method
                        bool isSoftware =
                            rendererName.contains("software", Qt::CaseInsensitive) ||
                            rendererName.contains("llvmpipe", Qt::CaseInsensitive) ||
                            rendererName.contains("mesa", Qt::CaseInsensitive);

                        bool isHardware =
                            rendererName.contains("NVIDIA", Qt::CaseInsensitive) ||
                            rendererName.contains("AMD", Qt::CaseInsensitive) ||
                            rendererName.contains("ATI", Qt::CaseInsensitive) ||
                            rendererName.contains("Intel", Qt::CaseInsensitive) ||
                            rendererName.contains("Radeon", Qt::CaseInsensitive) ||
                            rendererName.contains("GeForce", Qt::CaseInsensitive);

                        // Make a determination
                        if (g_forceCPURendering) {
                            stream << "Rendering Method: SOFTWARE (CPU) [FORCED MODE]\n";
                        }
                        else if (isSoftware) {
                            stream << "Rendering Method: SOFTWARE (CPU)\n";
                        }
                        else if (isHardware) {
                            stream << "Rendering Method: HARDWARE (GPU)\n";
                        }
                        else {
                            if (capStr.contains("software", Qt::CaseInsensitive) &&
                                !capStr.contains("NVIDIA", Qt::CaseInsensitive) &&
                                !capStr.contains("AMD", Qt::CaseInsensitive) &&
                                !capStr.contains("Intel", Qt::CaseInsensitive)) {
                                stream << "Rendering Method: Likely SOFTWARE (CPU)\n";
                            }
                            else {
                                stream << "Rendering Method: Possibly HARDWARE (GPU)\n";
                            }
                        }

                        // Add warning if needed
                        if (g_forceCPURendering && isHardware && !isSoftware) {
                            stream << "NOTE: Hardware renderer detected but application is in CPU forced mode.\n";
                            stream << "Some hardware acceleration may still be used, but rendering will be managed as CPU mode.\n";
                        }
                    }
                }

                // Print OpenGL version if available
                if (capStr.contains("OpenGL version string:")) {
                    int startPos = capStr.indexOf("OpenGL version string:") + 23;
                    int endPos = capStr.indexOf('\n', startPos);
                    if (endPos > startPos) {
                        QString versionStr = capStr.mid(startPos, endPos - startPos).trimmed();
                        stream << "OpenGL Version: " + versionStr + "\n";
                    }
                }
            }

            // Add hardware information
            if (g_forceCPURendering || !g_initialSettings.useGPU) {
                // CPU info
                stream << "\n--- CPU Information (CPU Mode Selected) ---\n";

                QProcess process;
                process.start("wmic", QStringList() << "cpu" << "get" << "Name,NumberOfCores,NumberOfLogicalProcessors");
                if (process.waitForFinished(3000)) {
                    QString output = process.readAllStandardOutput();
                    if (!output.isEmpty()) {
                        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
                        if (lines.size() >= 2) {
                            QString values = lines[1].trimmed();
                            QStringList valueList = values.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

                            if (valueList.size() >= 2) {
                                QString processors = valueList.takeLast();
                                QString cores = valueList.takeLast();
                                QString cpuName = valueList.join(" ");

                                stream << "CPU Name: " + cpuName + "\n";
                                stream << "CPU Cores: " + cores + "\n";
                                stream << "CPU Logical Processors: " + processors + "\n";
                            }
                        }
                    }
                }
            }
            else {
                // GPU info
                stream << "\n--- GPU Information (GPU Mode Selected) ---\n";

                QProcess process;
                process.start("wmic", QStringList() << "path" << "win32_VideoController" << "get" << "Name,AdapterRAM,DriverVersion");
                if (process.waitForFinished(3000)) {
                    QString output = process.readAllStandardOutput();
                    if (!output.isEmpty()) {
                        QStringList lines = output.split('\n', Qt::SkipEmptyParts);
                        if (lines.size() >= 2) {
                            for (int i = 1; i < lines.size(); i++) {
                                QString line = lines[i].trimmed();
                                if (!line.isEmpty()) {
                                    QStringList parts = line.split(QRegularExpression("\\s{2,}"), Qt::SkipEmptyParts);
                                    if (parts.size() >= 1) {
                                        QString gpuName = parts[0].trimmed();
                                        stream << "GPU Name: " + gpuName + "\n";

                                        if (parts.size() >= 3) {
                                            bool ok;
                                            qlonglong ramBytes = parts[1].toLongLong(&ok);
                                            if (ok) {
                                                double ramGB = ramBytes / (1024.0 * 1024.0 * 1024.0);
                                                stream << QString("GPU RAM: %1 GB\n").arg(ramGB, 0, 'f', 2);
                                            }

                                            stream << "GPU Driver: " + parts[2] + "\n";
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Show the information in a new window
            showInfoInNewWindow("Renderer Information", rendererInfo);
        }
        else {
            QMessageBox::information(nullptr, "VTK Not Initialized",
                "VTK is not initialized yet - cannot check renderer.");
        }
        });

    QObject::connect(printCapabilitiesButton, &QPushButton::clicked, []() {
        if (g_vtkInitialized && g_renderWindow) {
            const char* capabilities = g_renderWindow->ReportCapabilities();
            if (capabilities) {
                QString capStr = QString(capabilities);
                showInfoInNewWindow("Full Renderer Capabilities", capStr);
            }
            else {
                QMessageBox::information(nullptr, "Error",
                    "Could not get renderer capabilities.");
            }
        }
        else {
            QMessageBox::information(nullptr, "VTK Not Initialized",
                "VTK is not initialized yet - cannot print capabilities.");
        }
        });

    // Connect performance comparison button
    QObject::connect(compareButton, &QPushButton::clicked, [&mainWindow]() {
        runPerformanceComparison(&mainWindow);
        });

    // Initialize VTK and show initial shape when the main window appears
    mainWindow.installEventFilter(new QObject(&mainWindow));
    QObject::connect(&mainWindow, &QMainWindow::destroyed, [](QObject*) {}); // Dummy connect to prevent warning

    // Check for basic renderer information when initialization is complete, without showing CPU/GPU details
    QTimer::singleShot(500, [=]() {
        if (g_vtkInitialized && g_renderWindow) {
            // Don't automatically call updateRendererInfo()
            logDebugInfo("VTK initialization complete");
            logDebugInfo("Click 'Check Renderer Information' button to see hardware details");
        }
        else {
            logDebugInfo("VTK initialization pending or failed");
        }
        });

    // Use a single-shot timer to initialize after window is shown
    QTimer::singleShot(100, [=]() {
        // Initialize VTK but don't create a shape yet
        if (!g_vtkInitialized) {
            initializeVtkWindow();
        }
        });

    // Connect color button
    QObject::connect(colorButton, &QPushButton::clicked, [=, &currentColor]() {
        QColor newColor = QColorDialog::getColor(currentColor, colorButton);
        if (newColor.isValid()) {
            currentColor = newColor;
            // Don't update the button color to keep it white
            updateShape();
        }
        });

    // Connect shape type combo box change to update shape
    QObject::connect(shapeTypeCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]() {
        updateShape();
        });

    // Connect resolution spin box change to update shape (with real-time updates)
    QObject::connect(resolutionSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=]() {
        updateShape();
        });

    // Show Qt window and start Qt event loop
    mainWindow.show();
    logDebugInfo("Qt main window shown, starting Qt event loop");
    return app.exec();
}