//imageprocessor.h
#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <boost/signals2.hpp>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "graphics_items.h"

namespace fs = std::filesystem;

class LoadingProgress;

class ImageProcessor {

    friend class LoadingProgress;

public:
    ImageProcessor();
    ~ImageProcessor();

    void run();

private:
    // Window and OpenGL context
    GLFWwindow* m_window;
    int m_windowWidth;
    int m_windowHeight;
    bool m_running;

    // UI state
    bool m_showLoadDialog;
    bool m_showSaveDialog;
    bool m_showControlsWindow;
    std::string m_currentPath;
    char m_statusText[256];
    char m_inputFilename[256];
    float m_lastPanX;
    float m_lastPanY;

    // Scene components
    Scene m_scene;
    GraphicsView m_view;
    std::shared_ptr<TextureItem> m_imageItem;
    std::shared_ptr<RectItem> m_selectionItem;

    // Image data
    std::vector<std::vector<uint16_t>> m_imgData;
    std::vector<std::vector<uint16_t>> m_originalImg;
    bool m_regionSelected;

    // Signals
    boost::signals2::signal<void(const std::string&)> imageLoadedSignal;
    boost::signals2::signal<void(const glm::vec4&)> regionSelectedSignal;

    // Initialization
    bool initializeWindow();
    void initializeImGui();
    void cleanup();

    // UI rendering
    void drawMainWindow();
    void drawControlsWindow();
    void drawStatusBar();
    void handleLoadDialog();
    void handleSaveDialog();

    // File operations
    bool loadImage(const std::string& path);
    bool saveImage(const std::string& path);
    void updateDisplayImage();

    // Static callbacks
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void glfwErrorCallback(int error, const char* description);

    // Add pan variables
    float m_panX = 0.0f;
    float m_panY = 0.0f;

    // Add resetView function declaration
    void resetView();

    std::string m_selectedFilePath;

    uint16_t m_minValue = 0;
    uint16_t m_maxValue = 0;
    double m_meanValue = 0.0;
};

#endif // IMAGEPROCESSOR_H