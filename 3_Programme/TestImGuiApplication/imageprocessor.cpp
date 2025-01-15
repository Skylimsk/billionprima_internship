#include "imageprocessor.h"
#include <iostream>
#include <fstream>
#include <sstream>

class LoadingProgress {
public:
    static void updateProgress(const std::string& filename, float percentage, char* statusText, size_t bufferSize) {
        snprintf(statusText, bufferSize, "Loading %s... %.1f%%",
            filename.c_str(), percentage);
    }
};

ImageProcessor::ImageProcessor()
    : m_window(nullptr)
    , m_windowWidth(1280)
    , m_windowHeight(720)
    , m_running(true)
    , m_showLoadDialog(false)
    , m_showSaveDialog(false)
    , m_showControlsWindow(true)
    , m_currentPath(std::filesystem::current_path().string())
    , m_lastPanX(0.0f)
    , m_lastPanY(0.0f)
    , m_regionSelected(false)
{
    if (!initializeWindow()) {
        throw std::runtime_error("Failed to initialize window");
    }

    initializeImGui();

    // Initialize graphics items
    m_imageItem = std::make_shared<TextureItem>();
    m_selectionItem = std::make_shared<RectItem>();

    // Add items to scene
    m_scene.addItem(m_imageItem);
    m_scene.addItem(m_selectionItem);

    // Setup view
    m_view.setScene(&m_scene);
    m_view.setViewportSize(m_windowWidth, m_windowHeight);

    strcpy_s(m_statusText, sizeof(m_statusText), "Ready");
}

ImageProcessor::~ImageProcessor() {
    cleanup();
}

bool ImageProcessor::initializeWindow() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        return false;
    }

    // Request OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Enable MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Image Processor", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return false;
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set default viewport
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    // Additional debug info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    return true;
}

void ImageProcessor::initializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ImageProcessor::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void ImageProcessor::drawControlsWindow() {
    
}

void ImageProcessor::drawStatusBar() {
    ImGui::SetNextWindowPos(ImVec2(0, m_windowHeight - 25));
    ImGui::SetNextWindowSize(ImVec2(m_windowWidth, 25));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse;

    ImGui::Begin("##StatusBar", nullptr, flags);
    ImGui::Text("%s", m_statusText);
    ImGui::End();
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
                            m_selectedFilePath.clear(); // Clear selection when changing directory
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

        // Add buttons at the bottom with proper spacing
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);  // Add some spacing
        float contentWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = 120;
        float spacing = 10;

        // Center the buttons
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
    ImGui::SetNextWindowSize(ImVec2(400, 150), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(m_windowWidth * 0.5f - 200, m_windowHeight * 0.5f - 75),
        ImGuiCond_FirstUseEver
    );

    if (ImGui::Begin("Save Image", &m_showSaveDialog)) {
        ImGui::Text("Save Location: %s", m_currentPath.c_str());
        ImGui::InputText("Filename", m_inputFilename, sizeof(m_inputFilename));

        ImGui::Separator();

        if (ImGui::Button("Save", ImVec2(120, 0))) {
            fs::path savePath = fs::path(m_currentPath) / m_inputFilename;
            if (saveImage(savePath.string())) {
                m_showSaveDialog = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_showSaveDialog = false;
        }

        ImGui::End();
    }
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
        bytesRead += line.length() + 1;  // +1 for newline
        float progress = (float)bytesRead / fileSize * 100.0f;

        // Update progress in status bar
        snprintf(m_statusText, sizeof(m_statusText),
            "Loading %s... %.1f%%", filename.c_str(), progress);

        // Process line
        std::vector<uint16_t> row;
        std::stringstream ss(line);
        uint32_t value;

        while (ss >> value) {
            // Ensure 16-bit range
            uint16_t pixel = static_cast<uint16_t>(std::min(value, uint32_t(65535)));
            row.push_back(pixel);

            // Update statistics
            m_minValue = std::min(m_minValue, pixel);
            m_maxValue = std::max(m_maxValue, pixel);
            sum += pixel;
            totalPixels++;
        }

        if (!row.empty()) {
            m_imgData.push_back(std::move(row));
        }
    }

    // Check if we got any valid data
    if (m_imgData.empty()) {
        snprintf(m_statusText, sizeof(m_statusText), "Error: No valid data in file");
        return false;
    }

    // Check if all rows have the same size
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

    // Store original image for potential reset
    m_originalImg = m_imgData;

    // Update the display
    updateDisplayImage();

    // Update status with image info
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

    std::ofstream outFile(path);
    if (!outFile.is_open()) {
        snprintf(m_statusText, sizeof(m_statusText),
            "Error: Cannot create file %s", path.c_str());
        return false;
    }

    for (const auto& row : m_imgData) {
        for (size_t i = 0; i < row.size(); ++i) {
            outFile << row[i];
            if (i < row.size() - 1) outFile << " ";
        }
        outFile << "\n";
    }
    outFile.close();

    snprintf(m_statusText, sizeof(m_statusText), "Saved: %s", path.c_str());
    return true;
}

void ImageProcessor::drawMainWindow() {
    // Main window takes up full window width
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(m_windowWidth, m_windowHeight - 25));

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("##MainWindow", nullptr, flags)) {
        // Get the window's content region
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentPos = ImGui::GetCursorScreenPos();
        ImVec2 contentSize = ImGui::GetContentRegionAvail();

        // Update viewport for proper rendering
        glViewport(
            contentPos.x - windowPos.x,
            contentPos.y - windowPos.y,
            contentSize.x,
            contentSize.y
        );

        // Set the view size for correct image display
        m_view.setViewportSize(contentSize.x, contentSize.y);

        // Create a full-window button to capture mouse events
        ImGui::InvisibleButton("##canvas", contentSize);

        ImGui::End();
    }

    // Fixed control panel in top-right corner
    ImVec2 panelPos(m_windowWidth - 310, 10);
    ImGui::SetNextWindowPos(panelPos);
    ImGui::SetNextWindowSize(ImVec2(300, ImGui::GetFrameHeight() * 16));
    ImGui::SetNextWindowBgAlpha(0.85f);

    ImGuiWindowFlags controlFlags =
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar;

    if (ImGui::Begin("##Controls", nullptr, controlFlags)) {
        if (ImGui::CollapsingHeader("File Operations")) {
            if (ImGui::Button("Load Image", ImVec2(140, 0))) {
                m_showLoadDialog = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Save Image", ImVec2(140, 0))) {
                if (!m_imgData.empty()) {
                    m_showSaveDialog = true;
                }
            }
        }

        if (ImGui::CollapsingHeader("View Controls")) {
            static float zoomLevel = 1.0f;
            ImGui::SetNextItemWidth(-1);
            if (ImGui::SliderFloat("##zoom", &zoomLevel, 0.1f, 5.0f, "%.2fx", ImGuiSliderFlags_AlwaysClamp)) {
                float zoomFactor = zoomLevel / m_view.getZoom();
                m_view.zoom(1.0f / zoomFactor);
            }

            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("Pan X", &m_panX, -100.0f, 100.0f);

            ImGui::SetNextItemWidth(-1);
            ImGui::SliderFloat("Pan Y", &m_panY, -100.0f, 100.0f);

            if (ImGui::Button("Reset View", ImVec2(-1, 0))) {
                resetView();
            }
        }

        if (ImGui::CollapsingHeader("Image Information")) {
            if (!m_imgData.empty()) {
                ImGui::Text("Dimensions: %dx%d", m_imgData[0].size(), m_imgData.size());
                ImGui::Text("Bit Depth: 16-bit");
                ImGui::Text("Data Range: [%u, %u]", m_minValue, m_maxValue);
                ImGui::Text("Mean Value: %.2f", m_meanValue);
                ImGui::Text("Zoom: %.2fx", m_view.getZoom());

                ImVec2 mousePos = ImGui::GetMousePos();
                glm::vec2 scenePos = m_view.mapToScene({ mousePos.x, mousePos.y });
                int x = static_cast<int>(scenePos.x);
                int y = static_cast<int>(scenePos.y);

                if (x >= 0 && x < m_imgData[0].size() && y >= 0 && y < m_imgData.size()) {
                    uint16_t pixelValue = m_imgData[y][x];
                    float normalizedValue = static_cast<float>(pixelValue - m_minValue) /
                        (m_maxValue - m_minValue);

                    ImGui::Text("Position: (%d, %d)", x, y);
                    ImGui::Text("Value: %u (%.1f%%)",
                        pixelValue,
                        normalizedValue * 100.0f);
                }
            }
            else {
                ImGui::Text("No image loaded");
            }
        }

        ImGui::End();
    }
}

void ImageProcessor::run() {
    while (!glfwWindowShouldClose(m_window) && m_running) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Set initial viewport
        glViewport(0, 0, m_windowWidth, m_windowHeight);

        // Clear the entire window first
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw main window first
        drawMainWindow();

        // Draw scene BEFORE controls and dialogs
        m_scene.draw(m_view.getViewMatrix());

        // Draw UI components
        drawControlsWindow();
        drawStatusBar();

        if (m_showLoadDialog) handleLoadDialog();
        if (m_showSaveDialog) handleSaveDialog();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }
}

void ImageProcessor::updateDisplayImage() {
    if (m_imgData.empty() || m_imgData[0].empty()) return;

    int height = m_imgData.size();
    int width = m_imgData[0].size();

    // Convert 2D image data to 1D texture data
    std::vector<uint16_t> textureData;
    textureData.reserve(width * height);
    for (const auto& row : m_imgData) {
        textureData.insert(textureData.end(), row.begin(), row.end());
    }

    // Update the texture item with new image data
    m_imageItem->setImage(textureData, width, height);

    // Reset and center the view
    resetView();
}

void ImageProcessor::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto app = static_cast<ImageProcessor*>(glfwGetWindowUserPointer(window));
    app->m_windowWidth = width;
    app->m_windowHeight = height;
    app->m_view.setViewportSize(width, height);
    glViewport(0, 0, width, height);
}

void ImageProcessor::glfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void ImageProcessor::resetView() {
    if (!m_imgData.empty()) {
        m_view.fitInView(glm::vec4(0, 0, m_imgData[0].size(), m_imgData.size()));
        m_panX = 0.0f;
        m_panY = 0.0f;
        m_lastPanX = 0.0f;
        m_lastPanY = 0.0f;
    }
}