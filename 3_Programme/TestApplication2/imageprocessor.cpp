// Disable warnings for external libraries
#define _CRT_SECURE_NO_WARNINGS 

// Include project header first
#include "imageprocessor.h"

// Standard library headers
#include <iostream>
#include <fstream>
#include <sstream>

// STB image implementation (after warning suppression)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class LoadingProgress {
public:
    static void updateProgress(const std::string& filename, float percentage, char* statusText, size_t bufferSize) {
        snprintf(statusText, bufferSize, "Loading %s... %.1f%%",
            filename.c_str(), percentage);
    }
};

ImageProcessor::ImageProcessor()
    : m_window(nullptr)
    , m_glContext(nullptr)
    , m_windowWidth(1280)
    , m_windowHeight(720)
    , m_running(true)
    , m_showLoadDialog(false)
    , m_showSaveDialog(false)
    , m_showControlsWindow(true)
    , m_controlWindowCollapsed(false)
    , m_isPanning(false)
    , m_lastMouseX(0.0)
    , m_lastMouseY(0.0)
    , m_panX(0.0f)
    , m_panY(0.0f)
    , m_displayTexture(0)
    , m_minValue(0)
    , m_maxValue(65535)
    , m_meanValue(0.0)
    , m_currentPath(std::filesystem::current_path().string())
    , m_processedData(nullptr)
    , m_processedRows(0)
    , m_processedCols(0)
{
    std::cout << "Initializing ImageProcessor..." << std::endl;

    m_processImage = std::make_unique<CGProcessImage>();

    // Initialize mouse button states
    for (bool& button : m_mouseButtons) {
        button = false;
    }

    // Initialize status text and filename buffers
    strcpy_s(m_statusText, sizeof(m_statusText), "Ready");
    strcpy_s(m_inputFilename, sizeof(m_inputFilename), "");

    // Initialize SDL and OpenGL
    if (!initializeWindow()) {
        std::cerr << "Failed to initialize window" << std::endl;
        throw std::runtime_error("Failed to initialize window");
    }
    std::cout << "Window initialized successfully" << std::endl;

    // Initialize ImGui
    initializeImGui();
    std::cout << "ImGui initialized" << std::endl;

    // Print OpenGL information
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Initialize graphics items
    std::cout << "Creating graphics items..." << std::endl;
    m_imageItem = std::make_shared<TextureItem>();

    // Add items to scene
    std::cout << "Adding items to scene..." << std::endl;
    m_scene.addItem(m_imageItem);

    // Setup view
    std::cout << "Configuring view..." << std::endl;
    m_view.setScene(&m_scene);
    m_view.setViewportSize(m_windowWidth, m_windowHeight);

    // Initialize mouse position
    m_mousePos = glm::vec2(0.0f);
    m_mousePosOld = glm::vec2(0.0f);

    // Configure OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    std::cout << "ImageProcessor initialization complete" << std::endl;
}

ImageProcessor::~ImageProcessor() {

    if (m_processedData) {
        free2D(m_processedData, m_processedRows);
    }
    if (m_displayTexture) {
        glDeleteTextures(1, &m_displayTexture);
        m_displayTexture = 0;
    }
    cleanup();
}

bool ImageProcessor::initializeWindow() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set OpenGL version to 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // Create window with resizable flag
    m_window = SDL_CreateWindow(
        "Image Processor",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        m_windowWidth, m_windowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED
    );

    if (!m_window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Error initializing GLEW! " << glewGetErrorString(glewError) << std::endl;
        return false;
    }

    // Enable VSync
    SDL_GL_SetSwapInterval(1);

    // Get the actual window size after creation (may be different due to maximization)
    SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);

    // Initialize viewport with actual window size
    glViewport(0, 0, m_windowWidth, m_windowHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    return true;
}

void ImageProcessor::initializeImGui() {
    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ImageProcessor::cleanup() {
    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Cleanup SDL and OpenGL
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }

    if (m_window) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    SDL_Quit();
}

void ImageProcessor::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Let ImGui handle its events
        ImGui_ImplSDL2_ProcessEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            m_running = false;
            break;

        case SDL_WINDOWEVENT:
            handleWindowEvent(event);
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEWHEEL:
            if (!ImGui::GetIO().WantCaptureMouse) {
                handleMouseEvent(event);
            }
            break;
        }
    }
}

void ImageProcessor::handleWindowEvent(const SDL_Event& event) {
    // Declare variables outside switch statement
    float controlPanelHeight = m_controlWindowCollapsed ? 0.0f : (ImGui::GetFrameHeightWithSpacing() * 4);
    int imageViewportHeight = m_windowHeight - static_cast<int>(controlPanelHeight);

    switch (event.window.event) {
    case SDL_WINDOWEVENT_RESIZED:
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        m_windowWidth = event.window.data1;
        m_windowHeight = event.window.data2;
        glViewport(0, 0, m_windowWidth, m_windowHeight);

        // Update viewport sizes
        imageViewportHeight = m_windowHeight - static_cast<int>(controlPanelHeight);
        m_view.setViewportSize(m_windowWidth, imageViewportHeight);
        break;

    case SDL_WINDOWEVENT_MAXIMIZED:
        SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
        glViewport(0, 0, m_windowWidth, m_windowHeight);

        // Update viewport sizes for maximized window
        imageViewportHeight = m_windowHeight - static_cast<int>(controlPanelHeight);
        m_view.setViewportSize(m_windowWidth, imageViewportHeight);
        break;
    }
}

void ImageProcessor::handleMouseEvent(const SDL_Event& event) {
    // Calculate UI boundaries - remove bottom padding
    float controlPanelHeight = m_controlWindowCollapsed ? 0.0f : (ImGui::GetFrameHeightWithSpacing() * 2);
    float viewportY = controlPanelHeight;

    // Get current mouse position and convert to scene coordinates
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    glm::vec2 scenePos = m_view.mapToScene(glm::vec2(mouseX, mouseY - viewportY));

    // Get keyboard state for both Alt and Ctrl key checks
    const Uint8* keystate = SDL_GetKeyboardState(nullptr);
    bool altPressed = keystate[SDL_SCANCODE_LALT] || keystate[SDL_SCANCODE_RALT];
    bool ctrlPressed = keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL];

    switch (event.type) {
    case SDL_MOUSEBUTTONDOWN: {
        if (event.button.y > viewportY && event.button.button == SDL_BUTTON_LEFT && altPressed) {
            m_isPanning = true;
            m_lastMouseX = event.button.x;
            m_lastMouseY = event.button.y;
        }
        break;
    }

    case SDL_MOUSEBUTTONUP: {
        if (event.button.button == SDL_BUTTON_LEFT) {
            m_isPanning = false;
        }
        break;
    }

    case SDL_MOUSEMOTION: {
        if (m_isPanning && altPressed) {
            // Calculate pan delta in screen coordinates
            float deltaX = static_cast<float>(event.motion.x - m_lastMouseX);
            float deltaY = static_cast<float>(event.motion.y - m_lastMouseY);

            // Convert to scene coordinates with speed factor
            const float panSpeedFactor = 0.05f;
            float ndcDeltaX = (deltaX / (m_windowWidth * 0.5f)) * panSpeedFactor;
            float ndcDeltaY = (deltaY / (m_windowHeight * 0.5f)) * panSpeedFactor;

            // Adjust for current zoom level
            float zoom = m_view.getActualZoom();
            if (zoom != 0.0f) {
                ndcDeltaX /= zoom;
                ndcDeltaY /= zoom;
            }

            // Apply pan transformation
            m_view.pan(glm::vec2(ndcDeltaX, -ndcDeltaY));

            // Update last mouse position
            m_lastMouseX = event.motion.x;
            m_lastMouseY = event.motion.y;
        }
        break;
    }

    case SDL_MOUSEWHEEL: {
        if (ctrlPressed) {
            // Get current mouse position
            int mouseX, mouseY;
            SDL_GetMouseState(&mouseX, &mouseY);

            // Only handle zoom if mouse is in viewport area
            if (mouseY >= viewportY &&
                mouseY <= m_windowHeight &&
                mouseX >= 0 &&
                mouseX <= m_windowWidth) {

                // Calculate zoom factor based on wheel direction
                float zoomFactor = (event.wheel.y > 0) ? 1.1f : 0.9f;

                // Store scene coordinates before zoom
                glm::vec2 scenePosBefore = m_view.mapToScene(glm::vec2(mouseX, mouseY));

                // Apply zoom
                m_view.zoom(zoomFactor);

                // Get new scene coordinates after zoom
                glm::vec2 scenePosAfter = m_view.mapToScene(glm::vec2(mouseX, mouseY));

                // Calculate and apply offset to maintain mouse position
                glm::vec2 offset = scenePosAfter - scenePosBefore;
                m_view.pan(-offset);
            }
        }
        break;
    }
    }
}

void ImageProcessor::run() {
    while (m_running) {
        // Handle SDL events
        handleEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Clear the entire window with background color
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw control panel
        drawMainWindow();

        // Calculate viewport for image area
        float controlPanelHeight = m_controlWindowCollapsed ? 0.0f : (ImGui::GetFrameHeightWithSpacing() * 2);
        int imageViewportHeight = m_windowHeight - static_cast<int>(controlPanelHeight);

        // Set OpenGL viewport for image rendering
        glViewport(0, 0, m_windowWidth, imageViewportHeight);

        // Update view with new dimensions
        m_view.setViewportSize(m_windowWidth, imageViewportHeight);

        // Render the scene
        m_scene.draw(m_view.getViewMatrix());

        // Handle dialogs
        if (m_showLoadDialog) handleLoadDialog();
        if (m_showSaveDialog) handleSaveDialog();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        SDL_GL_SwapWindow(m_window);
    }
}

void ImageProcessor::resetView() {
    if (!m_imgData.empty()) {
        // Reset zoom to 1.0
        m_view.setZoom(1.0f);

        // Reset view matrix and fit image
        m_view.setViewMatrix(glm::mat4(1.0f));
        m_view.fitInView(glm::vec4(0, 0, m_imgData[0].size(), m_imgData.size()));

        // Reset pan values
        m_panX = 0.0f;
        m_panY = 0.0f;
        m_lastPanX = 0.0f;
        m_lastPanY = 0.0f;
    }
}

void ImageProcessor::updateMouseState() {
    // Update mouse position
    int x, y;
    uint32_t buttons = SDL_GetMouseState(&x, &y);

    m_mousePosOld = m_mousePos;
    m_mousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));

    // Update mouse button states
    m_mouseButtons[SDL_BUTTON_LEFT - 1] = (buttons & SDL_BUTTON_LMASK) != 0;
    m_mouseButtons[SDL_BUTTON_MIDDLE - 1] = (buttons & SDL_BUTTON_MMASK) != 0;
    m_mouseButtons[SDL_BUTTON_RIGHT - 1] = (buttons & SDL_BUTTON_RMASK) != 0;
}

void ImageProcessor::processSDLEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_QUIT:
        m_running = false;
        break;

    case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            m_windowWidth = event.window.data1;
            m_windowHeight = event.window.data2;
            m_view.setViewportSize(m_windowWidth, m_windowHeight);
        }
        break;

    case SDL_KEYDOWN:
        // Handle keyboard input if needed
        break;

    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEMOTION:
    case SDL_MOUSEWHEEL:
        if (!ImGui::GetIO().WantCaptureMouse) {
            handleMouseEvent(event);
        }
        break;
    }
}

void ImageProcessor::drawMainWindow() {
    float controlPanelHeight = ImGui::GetFrameHeightWithSpacing() * 2;

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoTitleBar;

    // Position window at the very top
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(m_windowWidth, controlPanelHeight));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));
    if (ImGui::Begin("##TopPanel", nullptr, windowFlags)) {
        float totalWidth = ImGui::GetContentRegionAvail().x;
        float buttonHeight = ImGui::GetFrameHeight();
        float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;

        // Calculate button widths using full width minus space for zoom text
        float zoomTextWidth = 120; // Width reserved for zoom text
        float availableWidth = totalWidth - zoomTextWidth - buttonSpacing;
        float standardButtonWidth = (availableWidth - buttonSpacing * 7) / 7;
        float zoomButtonWidth = standardButtonWidth * 0.4f;
        float fitButtonWidth = standardButtonWidth * 0.8f;

        // First row: Buttons
        // File Operations
        if (ImGui::Button("Load", ImVec2(standardButtonWidth, buttonHeight))) {
            m_showLoadDialog = true;
        }
        ImGui::SameLine();

        if (ImGui::Button("Save", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                m_showSaveDialog = true;
            }
        }
        ImGui::SameLine();

        // Zoom Controls Group
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            if (ImGui::Button("-", ImVec2(zoomButtonWidth, buttonHeight))) {
                m_view.zoom(0.9f);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine(0, buttonSpacing);
        }

        if (ImGui::Button("Fit", ImVec2(fitButtonWidth, buttonHeight))) {
            resetView();
        }
        ImGui::SameLine(0, buttonSpacing);

        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
            if (ImGui::Button("+", ImVec2(zoomButtonWidth, buttonHeight))) {
                m_view.zoom(1.1f);
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
        }

        if (ImGui::Button("Rotate L", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                rotateImageCounterClockwise();
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Rotate R", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                rotateImageClockwise();
            }
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
        if (ImGui::Button("Process", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                processCurrentImage();
            }
        }
        ImGui::PopStyleColor(2);

        // Zoom level text (aligned to the right)
        ImGui::SameLine();
        float currentZoom = m_view.getZoom();
        ImGui::SetCursorPosX(totalWidth - zoomTextWidth);
        ImGui::Text("Zoom: %.2fx", currentZoom);

        // Second row: Status text
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); // Small spacing after buttons
        ImGui::Text("%s", m_statusText);
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

// Status bar is now integrated into the main window
void ImageProcessor::drawStatusBar() {
    // Empty implementation as status is now part of main window
}


void ImageProcessor::handleLoadDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(m_windowWidth * 0.5f, m_windowHeight * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
    );

    if (ImGui::Begin("Load Image##Dialog", &m_showLoadDialog)) {
        ImGui::Text("Current Path: %s", m_currentPath.c_str());
        ImGui::Separator();

        if (ImGui::BeginChild("Files", ImVec2(0, -40), true)) {
            if (ImGui::Button("..")) {
                fs::path current(m_currentPath);
                if (current.has_parent_path()) {
                    m_currentPath = current.parent_path().string();
                }
            }

            try {
                for (const auto& entry : fs::directory_iterator(m_currentPath)) {
                    const auto& path = entry.path();
                    std::string filename = path.filename().string();

                    if (fs::is_directory(entry)) {
                        if (ImGui::Selectable((filename + "/").c_str())) {
                            m_currentPath = path.string();
                            m_selectedFilePath.clear();
                        }
                    }
                    else if (path.extension() == ".txt") {
                        bool isSelected = (m_selectedFilePath == path.string());
                        if (ImGui::Selectable(filename.c_str(), isSelected)) {
                            m_selectedFilePath = path.string();
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
            }

            ImGui::EndChild();
        }

        // Add buttons with proper spacing
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        float contentWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = 120;
        float spacing = 10;

        ImGui::SetCursorPosX((contentWidth - (2 * buttonWidth + spacing)) * 0.5f);

        if (ImGui::Button("Load", ImVec2(buttonWidth, 0))) {
            if (!m_selectedFilePath.empty()) {
                if (loadImage(m_selectedFilePath)) {
                    m_showLoadDialog = false;
                }
            }
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            m_showLoadDialog = false;
            m_selectedFilePath.clear();
        }

        ImGui::End();
    }
}

void ImageProcessor::handleSaveDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(m_windowWidth * 0.5f, m_windowHeight * 0.5f),
        ImGuiCond_FirstUseEver,
        ImVec2(0.5f, 0.5f)
    );

    if (ImGui::Begin("Save Image##Dialog", &m_showSaveDialog)) {
        ImGui::Text("Current Path: %s", m_currentPath.c_str());
        ImGui::Separator();

        if (ImGui::BeginChild("Files", ImVec2(0, -90), true)) {
            if (ImGui::Button("..")) {
                fs::path current(m_currentPath);
                if (current.has_parent_path()) {
                    m_currentPath = current.parent_path().string();
                }
            }

            try {
                for (const auto& entry : fs::directory_iterator(m_currentPath)) {
                    const auto& path = entry.path();
                    std::string filename = path.filename().string();

                    if (fs::is_directory(entry)) {
                        if (ImGui::Selectable((filename + "/").c_str())) {
                            m_currentPath = path.string();
                        }
                    }
                    else if (path.extension() == ".png") {
                        if (ImGui::Selectable(filename.c_str())) {
                            strcpy_s(m_inputFilename, sizeof(m_inputFilename), filename.c_str());
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
            }

            ImGui::EndChild();
        }

        ImGui::Text("Filename:");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##filename", m_inputFilename, sizeof(m_inputFilename))) {
            std::string filename = m_inputFilename;
            if (!filename.empty() && filename.substr(filename.length() - 4) != ".png") {
                filename += ".png";
                strcpy_s(m_inputFilename, sizeof(m_inputFilename), filename.c_str());
            }
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        float contentWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = 120;
        float spacing = 10;

        ImGui::SetCursorPosX((contentWidth - (2 * buttonWidth + spacing)) * 0.5f);

        if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
            std::string filename = m_inputFilename;
            if (!filename.empty()) {
                if (filename.substr(filename.length() - 4) != ".png") {
                    filename += ".png";
                }
                fs::path savePath = fs::path(m_currentPath) / filename;
                if (saveImage(savePath.string())) {
                    m_showSaveDialog = false;
                }
            }
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            m_showSaveDialog = false;
        }

        ImGui::End();
    }
}

void ImageProcessor::processCurrentImage() {
    if (m_imgData.empty() || !m_processImage) return;

    try {
        // Convert current image data to double array
        int rows = m_imgData.size();
        int cols = m_imgData[0].size();
        double** inputMatrix = nullptr;
        malloc2D(inputMatrix, rows, cols);
        if (!inputMatrix) {
            snprintf(m_statusText, sizeof(m_statusText), "Error: Failed to allocate memory for processing");
            return;
        }

        // Copy data to input matrix
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                inputMatrix[i][j] = static_cast<double>(m_imgData[i][j]);
            }
        }

        // Safely configure process image settings
        if (m_processImage) {
            m_processImage->SetEnergyMode(CGImageDisplayParameters.LaneData.EnergyMode);
            m_processImage->SetDualRowMode(CGImageDisplayParameters.LaneData.DualRowMode);
            m_processImage->SetDualRowDirection(CGImageDisplayParameters.LaneData.DualRowDirection);
            m_processImage->SetHighLowLayout(CGImageDisplayParameters.LaneData.HighLowLayout);
            m_processImage->SetPixelMaxValue(65535);
            m_processImage->SetAirSampleStart(CGImageCalculationVariables.AirSampleStart);
            m_processImage->SetAirSampleEnd(CGImageCalculationVariables.AirSampleEnd);

            // Process the image
            m_processImage->Process(inputMatrix, rows, cols);
        }

        // Get the processed data
        if (m_processImage->GetMergedData(m_processedData, m_processedRows, m_processedCols)) {
            // After interlacing, separate into left and right halves and merge with weighted average
            int halfWidth = m_processedCols / 2;

            // Allocate memory for the two halves
            double** leftHalf = nullptr;
            double** rightHalf = nullptr;
            malloc2D(leftHalf, m_processedRows, halfWidth);
            malloc2D(rightHalf, m_processedRows, halfWidth);

            // Separate into left and right halves
            for (int i = 0; i < m_processedRows; i++) {
                for (int j = 0; j < halfWidth; j++) {
                    leftHalf[i][j] = m_processedData[i][j];
                    rightHalf[i][j] = m_processedData[i][j + halfWidth];
                }
            }

            // Create final merged result
            double** mergedResult = nullptr;
            malloc2D(mergedResult, m_processedRows, halfWidth);

            // Perform weighted average merge (e.g., 0.6 for left, 0.4 for right)
            const double leftWeight = 0.6;
            const double rightWeight = 0.4;
            for (int i = 0; i < m_processedRows; i++) {
                for (int j = 0; j < halfWidth; j++) {
                    mergedResult[i][j] = (leftHalf[i][j] * leftWeight) +
                        (rightHalf[i][j] * rightWeight);
                }
            }

            // Update processed data with merged result
            free2D(m_processedData, m_processedRows);
            m_processedData = mergedResult;
            m_processedCols = halfWidth;

            // Cleanup temporary arrays
            free2D(leftHalf, m_processedRows);
            free2D(rightHalf, m_processedRows);

            // Update the display
            updateImageFromProcessed();
            snprintf(m_statusText, sizeof(m_statusText),
                "Image processed successfully (%dx%d)", m_processedCols, m_processedRows);
        }
        else {
            snprintf(m_statusText, sizeof(m_statusText), "Error: Failed to get processed data");
        }

        // Cleanup input matrix
        free2D(inputMatrix, rows);
    }
    catch (const std::exception& e) {
        snprintf(m_statusText, sizeof(m_statusText),
            "Error processing image: %s", e.what());
    }
}


void ImageProcessor::updateImageFromProcessed() {
    if (!m_processedData || m_processedRows == 0 || m_processedCols == 0) return;

    // Convert processed data back to image format
    m_imgData.clear();
    m_imgData.resize(m_processedRows);

    for (int i = 0; i < m_processedRows; i++) {
        m_imgData[i].resize(m_processedCols);
        for (int j = 0; j < m_processedCols; j++) {
            // Clamp values to valid range for uint16_t
            double val = std::max(0.0, std::min(65535.0, m_processedData[i][j]));
            m_imgData[i][j] = static_cast<uint16_t>(val);
        }
    }

    // Update the display
    updateDisplayImage();
}

bool ImageProcessor::loadImage(const std::string& path) {
    std::ifstream inFile(path, std::ios::binary | std::ios::ate);
    if (!inFile) {
        snprintf(m_statusText, sizeof(m_statusText), "Error: Cannot open file %s", path.c_str());
        return false;
    }

    // Get file size for progress calculation
    std::streamsize fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // Clear existing data
    m_imgData.clear();

    // Get filename for status updates
    std::string filename = std::filesystem::path(path).filename().string();
    size_t bytesRead = 0;
    std::string line;

    // Initialize statistics
    m_minValue = 65535;  // Max possible value for uint16_t
    m_maxValue = 0;      // Min possible value for uint16_t
    double sum = 0.0;
    size_t totalPixels = 0;

    // Read file line by line
    while (std::getline(inFile, line)) {
        bytesRead += line.length() + 1;
        float progress = (float)bytesRead / fileSize * 100.0f;

        // Update progress in status bar
        snprintf(m_statusText, sizeof(m_statusText),
            "Loading %s... %.1f%%", filename.c_str(), progress);

        // Process line
        std::vector<uint16_t> row;
        std::stringstream ss(line);
        uint32_t value;

        while (ss >> value) {
            uint16_t pixel = static_cast<uint16_t>(std::min(value, uint32_t(65535)));
            row.push_back(pixel);

            m_minValue = std::min(m_minValue, pixel);
            m_maxValue = std::max(m_maxValue, pixel);
            sum += pixel;
            totalPixels++;
        }

        if (!row.empty()) {
            m_imgData.push_back(std::move(row));
        }
    }

    if (m_imgData.empty()) {
        snprintf(m_statusText, sizeof(m_statusText), "Error: No valid data in file");
        return false;
    }

    // Check row consistency
    size_t firstRowSize = m_imgData[0].size();
    for (size_t i = 1; i < m_imgData.size(); ++i) {
        if (m_imgData[i].size() != firstRowSize) {
            snprintf(m_statusText, sizeof(m_statusText),
                "Error: Inconsistent row lengths in image data");
            m_imgData.clear();
            return false;
        }
    }

    // Calculate mean value
    m_meanValue = totalPixels > 0 ? sum / totalPixels : 0.0;

    // Store original image
    m_originalImg = m_imgData;

    // Update display
    updateDisplayImage();

    // Update status
    snprintf(m_statusText, sizeof(m_statusText),
        "Loaded: %s (%zux%zu) Range:[%u,%u] Mean:%.1f",
        filename.c_str(),
        m_imgData[0].size(), m_imgData.size(),
        m_minValue, m_maxValue, m_meanValue);

    return true;
}

bool ImageProcessor::saveImage(const std::string& path) {
    if (m_imgData.empty()) {
        snprintf(m_statusText, sizeof(m_statusText), "Error: No image to save");
        return false;
    }

    int width = m_imgData[0].size();
    int height = m_imgData.size();

    // Allocate memory for pixel data
    std::vector<unsigned char> pixels(width * height);

    // Convert 16-bit grayscale to 8-bit grayscale
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float normalized = static_cast<float>(m_imgData[y][x] - m_minValue) /
                static_cast<float>(m_maxValue - m_minValue);
            pixels[y * width + x] = static_cast<unsigned char>(normalized * 255.0f);
        }
    }

    // Save as PNG
    if (!stbi_write_png(path.c_str(), width, height, 1, pixels.data(), width)) {
        snprintf(m_statusText, sizeof(m_statusText),
            "Error: Failed to save PNG file %s", path.c_str());
        return false;
    }

    snprintf(m_statusText, sizeof(m_statusText), "Saved: %s", path.c_str());
    return true;
}

void ImageProcessor::updateDisplayImage() {
    if (m_imgData.empty() || m_imgData[0].empty()) return;

    int width = m_imgData[0].size();
    int height = m_imgData.size();

    // Convert 2D image data to 1D vector
    std::vector<uint16_t> textureData;
    textureData.reserve(width * height);
    for (const auto& row : m_imgData) {
        textureData.insert(textureData.end(), row.begin(), row.end());
    }

    // Update the texture item with new image data
    m_imageItem->setImage(textureData, width, height);

    // Set zoom to 1.0 first
    m_view.setZoom(1.0f);

    // Then reset and center the view
    resetView();
}

void ImageProcessor::convertDataToTexture() {
    if (m_imgData.empty() || m_imgData[0].empty()) return;

    int width = m_imgData[0].size();
    int height = m_imgData.size();

    // Generate single channel 8-bit image data
    std::vector<unsigned char> pixels(width * height);

    // Convert 16-bit data to 8-bit and normalize
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float normalized = static_cast<float>(m_imgData[y][x] - m_minValue) /
                static_cast<float>(m_maxValue - m_minValue);
            pixels[y * width + x] = static_cast<unsigned char>(normalized * 255.0f);
        }
    }

    // Delete existing texture if it exists
    if (m_displayTexture) {
        glDeleteTextures(1, &m_displayTexture);
    }

    // Create new OpenGL texture
    glGenTextures(1, &m_displayTexture);
    glBindTexture(GL_TEXTURE_2D, m_displayTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set swizzle mask to show grayscale properly
    GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    // Upload texture data as single channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,
        GL_RED, GL_UNSIGNED_BYTE, pixels.data());
}

void ImageProcessor::drawControlsWindow() {
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(m_windowWidth, m_windowHeight * 0.2f));

    if (ImGui::Begin("Controls", nullptr, window_flags)) {
        // Image info
        if (!m_imgData.empty()) {
            ImGui::Text("Image Size: %dx%d",
                static_cast<int>(m_imgData[0].size()),
                static_cast<int>(m_imgData.size()));
            ImGui::Text("Value Range: [%d, %d]", m_minValue, m_maxValue);
            ImGui::Text("Mean Value: %.2f", m_meanValue);
        }

        // View controls
        if (ImGui::CollapsingHeader("View Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            float zoom = m_view.getZoom();
            if (ImGui::SliderFloat("Zoom", &zoom, 0.1f, 10.0f)) {
                float zoomFactor = zoom / m_view.getZoom();
                m_view.zoom(zoomFactor);
            }

            if (ImGui::Button("Reset View")) {
                resetView();
            }
        }
    }
    ImGui::End();
}

void ImageProcessor::updateWindowTitle(const std::string& filename) {
    if (m_window) {
        std::string title = "Image Processor - " + filename;
        SDL_SetWindowTitle(m_window, title.c_str());
    }
}

bool ImageProcessor::createOpenGLContext() {
    // Set OpenGL context attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // Create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "OpenGL context could not be created! SDL Error: "
            << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Error initializing GLEW! "
            << glewGetErrorString(glewError) << std::endl;
        return false;
    }

    // Enable vsync
    SDL_GL_SetSwapInterval(1);

    return true;
}

void ImageProcessor::destroyOpenGLContext() {
    if (m_glContext) {
        SDL_GL_DeleteContext(m_glContext);
        m_glContext = nullptr;
    }
}

void ImageProcessor::rotateImageClockwise() {
    if (m_imgData.empty()) return;

    size_t oldHeight = m_imgData.size();
    size_t oldWidth = m_imgData[0].size();

    // Create a new vector with swapped dimensions
    std::vector<std::vector<uint16_t>> rotated(oldWidth, std::vector<uint16_t>(oldHeight));

    // Perform 90-degree clockwise rotation
    for (size_t i = 0; i < oldHeight; ++i) {
        for (size_t j = 0; j < oldWidth; ++j) {
            rotated[j][oldHeight - 1 - i] = m_imgData[i][j];
        }
    }

    // Update image data
    m_imgData = std::move(rotated);

    // Update display
    updateDisplayImage();

    // Update status
    snprintf(m_statusText, sizeof(m_statusText),
        "Image rotated clockwise (%zux%zu)", m_imgData[0].size(), m_imgData.size());
}

void ImageProcessor::rotateImageCounterClockwise() {
    if (m_imgData.empty()) return;

    size_t oldHeight = m_imgData.size();
    size_t oldWidth = m_imgData[0].size();

    // Create a new vector with swapped dimensions
    std::vector<std::vector<uint16_t>> rotated(oldWidth, std::vector<uint16_t>(oldHeight));

    // Perform 90-degree counter-clockwise rotation
    for (size_t i = 0; i < oldHeight; ++i) {
        for (size_t j = 0; j < oldWidth; ++j) {
            rotated[oldWidth - 1 - j][i] = m_imgData[i][j];
        }
    }

    // Update image data
    m_imgData = std::move(rotated);

    // Update display
    updateDisplayImage();

    // Update status
    snprintf(m_statusText, sizeof(m_statusText),
        "Image rotated counter-clockwise (%zux%zu)", m_imgData[0].size(), m_imgData.size());
}