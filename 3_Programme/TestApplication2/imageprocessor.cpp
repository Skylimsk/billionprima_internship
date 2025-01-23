// Disable warnings for external libraries
#define _CRT_SECURE_NO_WARNINGS 

// Include project header first
#include "imageprocessor.h"

// Standard library headers
#include <iostream>
#include <fstream>
#include <sstream>

// Graphics components
#include "graphics_scene.h"
#include "graphics_view.h"
#include "texture_item.h"
#include "rect_item.h"

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
    , m_isDrawingRect(false)  // Initialize rectangle drawing state
    , m_rectStartPos(0.0f)    // Initialize start position
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
    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Set OpenGL version and profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Set OpenGL attributes for rendering
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    // Additional buffer attributes for color precision
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

    // Create window with specified flags
    m_window = SDL_CreateWindow(
        "Image Processor",                           // Window title
        SDL_WINDOWPOS_CENTERED,                      // Initial x position
        SDL_WINDOWPOS_CENTERED,                      // Initial y position
        m_windowWidth,                               // Initial width
        m_windowHeight,                              // Initial height
        SDL_WINDOW_OPENGL |                          // OpenGL support
        SDL_WINDOW_SHOWN |                           // Show on creation
        SDL_WINDOW_RESIZABLE |                       // Allow resizing
        SDL_WINDOW_ALLOW_HIGHDPI |                   // High DPI support
        SDL_WINDOW_MAXIMIZED                         // Start maximized
    );

    // Check if window creation failed
    if (!m_window) {
        std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create OpenGL context
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext) {
        std::cerr << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        return false;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;  // Enable experimental features
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cerr << "Error initializing GLEW! " << glewGetErrorString(glewError) << std::endl;
        SDL_GL_DeleteContext(m_glContext);
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
        m_glContext = nullptr;
        return false;
    }

    // Enable VSync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::cerr << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
    }

    // Get the actual window size after creation
    SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);

    // Initialize viewport without ImGui dependency
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    // Configure OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

    // Clear any existing OpenGL errors
    while (glGetError() != GL_NO_ERROR) {}

    // Update view with initial dimensions - no ImGui dependency here
    m_view.setViewportSize(m_windowWidth, m_windowHeight);

    // Print OpenGL driver information
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return true;
}

void ImageProcessor::initializeImGui() {
    // Initialize ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
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
    const float CONTROL_PANEL_HEIGHT = 50.0f;
    int imageViewportHeight = m_windowHeight - CONTROL_PANEL_HEIGHT;

    switch (event.window.event) {
    case SDL_WINDOWEVENT_RESIZED:
    case SDL_WINDOWEVENT_SIZE_CHANGED:
        m_windowWidth = event.window.data1;
        m_windowHeight = event.window.data2;
        glViewport(0, 0, m_windowWidth, imageViewportHeight);
        m_view.setViewportSize(m_windowWidth, imageViewportHeight);
        break;

    case SDL_WINDOWEVENT_MAXIMIZED:
        SDL_GetWindowSize(m_window, &m_windowWidth, &m_windowHeight);
        glViewport(0, 0, m_windowWidth, imageViewportHeight);
        m_view.setViewportSize(m_windowWidth, imageViewportHeight);
        break;

    case SDL_WINDOWEVENT_EXPOSED:
        glViewport(0, 0, m_windowWidth, imageViewportHeight);
        break;

    case SDL_WINDOWEVENT_CLOSE:
        m_running = false;
        break;
    }
}

void ImageProcessor::handleMouseEvent(const SDL_Event& event) {
    const float CONTROL_PANEL_HEIGHT = 50.0f;
    float viewportY = CONTROL_PANEL_HEIGHT;

    // Get current mouse position
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);

    // Get modifier keys state
    const Uint8* keystate = SDL_GetKeyboardState(nullptr);
    bool altPressed = keystate[SDL_SCANCODE_LALT] || keystate[SDL_SCANCODE_RALT];
    bool ctrlPressed = keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL];

    // Convert to scene coordinates
    glm::vec2 scenePos = m_view.mapToScene(glm::vec2(mouseX, mouseY));

    // Only process events if mouse is in image viewport area
    if (mouseY >= 0 && mouseY <= m_windowHeight - CONTROL_PANEL_HEIGHT) {
        switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                if (altPressed) {
                    // Pan operation
                    m_isPanning = true;
                    m_lastMouseX = mouseX;
                    m_lastMouseY = mouseY;
                }
                else {
                    // Start rectangle drawing with exact coordinates
                    if (m_pointItem) {
                        m_scene.removeItem(m_pointItem);
                    }
                    m_pointItem = std::make_shared<PointItem>(scenePos);
                    m_scene.addItem(m_pointItem);

                    m_isDrawingRect = true;
                    m_rectStartPos = scenePos;

                    if (m_rectItem) {
                        m_scene.removeItem(m_rectItem);
                    }
                    m_rectItem = std::make_shared<RectItem>();
                    m_rectItem->setRect(glm::vec4(scenePos.x, scenePos.y, 0, 0));
                    m_scene.addItem(m_rectItem);
                }
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            if (event.button.button == SDL_BUTTON_LEFT) {
                m_isPanning = false;
                if (m_isDrawingRect && m_rectItem) {
                    // Finalize rectangle with exact dimensions
                    float width = scenePos.x - m_rectStartPos.x;
                    float height = scenePos.y - m_rectStartPos.y;

                    glm::vec4 rect;
                    rect.x = width > 0 ? std::round(m_rectStartPos.x) : std::round(scenePos.x);
                    rect.y = height > 0 ? std::round(m_rectStartPos.y) : std::round(scenePos.y);
                    rect.z = std::round(std::abs(width));
                    rect.w = std::round(std::abs(height));

                    m_rectItem->setRect(rect);

                    // Remove tiny rectangles
                    if (rect.z < 1.0f || rect.w < 1.0f) {
                        m_scene.removeItem(m_rectItem);
                        m_rectItem = nullptr;
                    }
                }
                m_isDrawingRect = false;
            }
            break;
        }

        case SDL_MOUSEMOTION: {
            if (m_isPanning && altPressed) {
                // Keep original panning logic
                float deltaX = static_cast<float>(mouseX - m_lastMouseX);
                float deltaY = static_cast<float>(mouseY - m_lastMouseY);

                const float panSpeedFactor = 0.05f;
                float ndcDeltaX = (deltaX / (m_windowWidth * 0.5f)) * panSpeedFactor;
                float ndcDeltaY = (deltaY / (m_windowHeight * 0.5f)) * panSpeedFactor;

                float zoom = m_view.getActualZoom();
                if (zoom != 0.0f) {
                    ndcDeltaX /= zoom;
                    ndcDeltaY /= zoom;
                }

                m_view.pan(glm::vec2(ndcDeltaX, -ndcDeltaY));
                m_lastMouseX = mouseX;
                m_lastMouseY = mouseY;
            }
            else if (m_isDrawingRect && m_rectItem && m_imageItem) {
                // Update rectangle with exact coordinates
                float width = scenePos.x - m_rectStartPos.x;
                float height = scenePos.y - m_rectStartPos.y;

                glm::vec4 rect;
                rect.x = width > 0 ? std::round(m_rectStartPos.x) : std::round(scenePos.x);
                rect.y = height > 0 ? std::round(m_rectStartPos.y) : std::round(scenePos.y);
                rect.z = std::round(std::abs(width));
                rect.w = std::round(std::abs(height));

                m_rectItem->setRect(rect);
            }
            break;
        }

        case SDL_MOUSEWHEEL: {
            if (ImGui::GetIO().WantCaptureMouse) return;

            if (ctrlPressed) {
                float zoomFactor = (event.wheel.y > 0) ? 1.1f : 0.9f;
                glm::vec2 beforePos = m_view.mapToScene(glm::vec2(mouseX, mouseY));
                m_view.zoom(zoomFactor);
                glm::vec2 afterPos = m_view.mapToScene(glm::vec2(mouseX, mouseY));
                m_view.pan(afterPos - beforePos);
            }
            break;
        }
        }
    }
}

void ImageProcessor::run() {
    while (m_running) {
        handleEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        const float CONTROL_PANEL_HEIGHT = 50.0f;
        int imageViewportHeight = m_windowHeight - CONTROL_PANEL_HEIGHT;

        // Clear window
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClearColor(0.94f, 0.94f, 0.94f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw control panel
        glViewport(0, m_windowHeight - CONTROL_PANEL_HEIGHT,
            m_windowWidth, CONTROL_PANEL_HEIGHT);
        drawMainWindow();

        // Set viewport for image area
        glViewport(0, 0, m_windowWidth, imageViewportHeight);
        m_view.setViewportSize(m_windowWidth, imageViewportHeight);
        m_scene.draw(m_view.getViewMatrix());

        // Handle dialogs and render ImGui
        if (m_showLoadDialog) handleLoadDialog();
        if (m_showSaveDialog) handleSaveDialog();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
    const float CONTROL_PANEL_HEIGHT = 50.0f;
    ImGui::SetNextWindowPos(ImVec2(0, m_windowHeight - CONTROL_PANEL_HEIGHT));
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(m_windowWidth), CONTROL_PANEL_HEIGHT));

    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoTitleBar;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 4));

    if (ImGui::Begin("##TopPanel", nullptr, windowFlags)) {
        const float totalWidth = ImGui::GetContentRegionAvail().x;
        const float buttonHeight = ImGui::GetFrameHeight();
        const float buttonSpacing = ImGui::GetStyle().ItemSpacing.x;
        const float zoomTextWidth = 120.0f;
        const float availableWidth = totalWidth - zoomTextWidth - buttonSpacing;
        const float standardButtonWidth = (availableWidth - buttonSpacing * 9) / 9;
        const float zoomButtonWidth = standardButtonWidth * 0.4f;
        const float fitButtonWidth = standardButtonWidth * 0.8f;

        if (ImGui::Button("Load", ImVec2(standardButtonWidth, buttonHeight))) {
            m_showLoadDialog = true;
        }
        ImGui::SameLine();

        if (ImGui::Button("Save", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) m_showSaveDialog = true;
        }
        ImGui::SameLine();

        // Zoom controls
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        if (ImGui::Button("-", ImVec2(zoomButtonWidth, buttonHeight))) {
            m_view.zoom(0.9f);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine(0, buttonSpacing);

        if (ImGui::Button("Fit", ImVec2(fitButtonWidth, buttonHeight))) {
            resetView();
        }
        ImGui::SameLine(0, buttonSpacing);

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        if (ImGui::Button("+", ImVec2(zoomButtonWidth, buttonHeight))) {
            m_view.zoom(1.1f);
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();

        // Rotation and other controls
        if (ImGui::Button("Rotate L", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                pushToHistory();
                rotateImageCounterClockwise();
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Rotate R", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                pushToHistory();
                rotateImageClockwise();
            }
        }
        ImGui::SameLine();

        if (ImGui::Button("Undo", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) undo();
        }
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.4f, 1.0f));
        if (ImGui::Button("Process", ImVec2(standardButtonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                pushToHistory();
                processCurrentImage();
            }
        }
        ImGui::PopStyleColor(2);
        ImGui::SameLine();

        if (ImGui::Button("Crop", ImVec2(standardButtonWidth, buttonHeight))) {
            if (m_rectItem) cropToSelection();
        }

        // Zoom level text
        ImGui::SameLine();
        ImGui::SetCursorPosX(totalWidth - zoomTextWidth);
        ImGui::Text("Zoom: %.2fx", m_view.getZoom());

        // Status text
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);
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

    pushToHistory();

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
    m_scene.clear();
    m_scene.addItem(m_imageItem);
    m_rectItem = nullptr;
    m_isDrawingRect = false;

    double** tempMatrix = nullptr;
    int rows = 0, cols = 0;

    try {
        ImageReader::ReadTextToU2D(path, tempMatrix, rows, cols);

        // Clear existing data
        m_imgData.clear();
        m_undoHistory.clear();
        m_imgData.resize(rows);

        // Initialize statistics 
        m_minValue = 65535;
        m_maxValue = 0;
        double sum = 0.0;

        // Convert double matrix to uint16_t vectors
        for (int i = 0; i < rows; i++) {
            m_imgData[i].resize(cols);
            for (int j = 0; j < cols; j++) {
                uint16_t value = static_cast<uint16_t>(std::min(tempMatrix[i][j], 65535.0));
                m_imgData[i][j] = value;

                m_minValue = std::min(m_minValue, value);
                m_maxValue = std::max(m_maxValue, value);
                sum += value;
            }
        }

        // Store original image
        m_originalImg = m_imgData;

        // Calculate mean
        m_meanValue = sum / (rows * cols);

        // Update display
        updateDisplayImage();
        resetView();
        m_view.fitInView(glm::vec4(0, 0, cols, rows));

        // Update status
        std::string filename = std::filesystem::path(path).filename().string();
        snprintf(m_statusText, sizeof(m_statusText),
            "Loaded: %s (%dx%d) Range:[%u,%u] Mean:%.1f",
            filename.c_str(), cols, rows,
            m_minValue, m_maxValue, m_meanValue);

        // Cleanup
        for (int i = 0; i < rows; i++) {
            free(tempMatrix[i]);
        }
        free(tempMatrix);

        return true;
    }
    catch (const std::exception& e) {
        snprintf(m_statusText, sizeof(m_statusText),
            "Error loading image: %s", e.what());
        return false;
    }
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

    // Center the image in view
    m_view.fitInView(glm::vec4(0, 0, width, height));
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

    pushToHistory();

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

    pushToHistory();

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

void ImageProcessor::clearSelection() {
    if (m_rectItem) {
        m_scene.removeItem(m_rectItem);
        m_rectItem = nullptr;
    }
    m_isDrawingRect = false;
}

void ImageProcessor::pushToHistory() {
    // Save current state to history
    m_undoHistory.push_back(m_imgData);
}

void ImageProcessor::undo() {
    if (m_undoHistory.empty()) {
        snprintf(m_statusText, sizeof(m_statusText), "No more undo history");
        return;
    }

    // Clear any existing selection
    clearSelection();

    // Restore previous state
    m_imgData = m_undoHistory.back();
    m_undoHistory.pop_back();

    // Update display with full restored image
    updateDisplayImage();

    snprintf(m_statusText, sizeof(m_statusText), "Undo successful (%zux%zu)",
        m_imgData[0].size(), m_imgData.size());
}

void ImageProcessor::cropToSelection() {
    if (!m_rectItem || m_imgData.empty()) return;

    pushToHistory();

    // Get exact rect coordinates in scene space
    glm::vec4 rect = m_rectItem->rect();
    int startX = std::max(0, static_cast<int>(rect.x));
    int startY = std::max(0, static_cast<int>(rect.y));
    int width = static_cast<int>(rect.z);
    int height = static_cast<int>(rect.w);

    // Ensure within image bounds
    width = std::min(width, static_cast<int>(m_imgData[0].size() - startX));
    height = std::min(height, static_cast<int>(m_imgData.size() - startY));

    // Create cropped image
    std::vector<std::vector<uint16_t>> cropped;
    cropped.reserve(height);

    // Copy the selected region
    for (int i = 0; i < height; i++) {
        cropped.emplace_back(m_imgData[startY + i].begin() + startX,
            m_imgData[startY + i].begin() + startX + width);
    }

    // Update image data and display
    m_imgData = std::move(cropped);
    clearSelection();
    updateDisplayImage();
}