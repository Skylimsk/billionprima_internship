#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

// GLEW must come first
#include <GL/glew.h>

// Then SDL and OpenGL headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// ImGui headers
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>

// External library headers
#include <boost/signals2.hpp>

// Standard library headers
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

// Project headers
#include "graphics_item.h"

#include "CGParams.h"     // For CGImageDisplayParameters and CGImageCalculationVariables
#include "CGProcessImage.h" // For CGProcessImage class
#include "pch.h"         // For malloc2D template function

namespace fs = std::filesystem;

class LoadingProgress;

class ImageProcessor {
    friend class LoadingProgress;

public:
    ImageProcessor();
    ~ImageProcessor();

    void run();

private:
    // SDL and OpenGL context
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
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

    // Image data
    std::vector<std::vector<uint16_t>> m_imgData;
    std::vector<std::vector<uint16_t>> m_originalImg;

    // Signals
    boost::signals2::signal<void(const std::string&)> imageLoadedSignal;

    // Initialization methods
    bool initializeWindow();
    void initializeImGui();
    void cleanup();
    bool createOpenGLContext();
    void destroyOpenGLContext();

    // UI rendering methods
    void drawMainWindow();
    void drawControlsWindow();
    void handleLoadDialog();
    void handleSaveDialog();
    void updateWindowTitle(const std::string& filename);

    // File operations
    bool loadImage(const std::string& path);
    bool saveImage(const std::string& path);
    void updateDisplayImage();
    void convertDataToTexture();

    // Event handling
    void handleEvents();
    void handleMouseEvent(const SDL_Event& event);
    void handleWindowEvent(const SDL_Event& event);
    void processSDLEvent(const SDL_Event& event);
    void updateMouseState();

    // View management
    void resetView();

    // Member variables
    std::string m_selectedFilePath;
    uint16_t m_minValue;
    uint16_t m_maxValue;
    double m_meanValue;
    GLuint m_displayTexture;

    // Window state
    bool m_controlWindowCollapsed;
    bool m_isPanning;
    double m_lastMouseX;
    double m_lastMouseY;

    // Pan variables
    float m_panX;
    float m_panY;

    // Mouse state
    bool m_mouseButtons[5];  // Track mouse button states
    glm::vec2 m_mousePos;    // Current mouse position
    glm::vec2 m_mousePosOld; // Previous mouse position

    void rotateImageClockwise();
    void rotateImageCounterClockwise();

    std::unique_ptr<CGProcessImage> m_processImage;
    double** m_processedData;
    int m_processedRows;
    int m_processedCols;

    void processCurrentImage();
    void updateImageFromProcessed();

    // Helper template function for 2D array deallocation
    template<typename T>
    void free2D(T**& array, int rows) {
        if (array) {
            for (int i = 0; i < rows; i++) {
                if (array[i]) {
                    free(array[i]);
                }
            }
            free(array);
            array = nullptr;
        }
    }

    void drawImageArea();

};

#endif // IMAGEPROCESSOR_H