#ifndef GRAPHICS_VIEW_H
#define GRAPHICS_VIEW_H

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include "graphics_scene.h"
#include "rect_item.h"

class GraphicsView {
public:
    GraphicsView(Scene* scene = nullptr);
    ~GraphicsView();

    // Scene management
    void setScene(Scene* scene);
    Scene* scene() const { return m_scene; }

    // View transformations 
    void zoom(float factor);
    void pan(const glm::vec2& delta);
    void fitInView(const glm::vec4& rect);

    // Coordinate mapping
    glm::vec2 mapToScene(const glm::vec2& viewPos) const;
    glm::vec2 mapFromScene(const glm::vec2& scenePos) const;

    // Mouse event handlers
    void handleMousePress(const SDL_Event& event);
    void handleMouseMove(const SDL_Event& event);
    void handleMouseRelease(const SDL_Event& event);
    void handleMouseWheel(const SDL_Event& event);

    // View matrix access
    const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
    void setViewMatrix(const glm::mat4& matrix) { m_viewMatrix = matrix; }

    // Window dimensions
    void setViewportSize(int width, int height);

    // Zoom control
    void setZoom(float newZoom) { m_zoom = newZoom; }
    float getZoom() const { return m_zoom; }
    float getActualZoom() const;

    void resetView();

private:
    void updateViewMatrix();

    std::shared_ptr<RectItem> m_currentRect;
    bool m_isDrawing = false;
    glm::vec2 m_lastMousePos;

    Scene* m_scene;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_projMatrix;
    glm::vec2 m_viewportSize;
    float m_zoom;

    // Added member variables:
    int m_windowWidth;
    int m_windowHeight;
    bool m_controlWindowCollapsed;
    bool m_isPanning;
};

#endif // GRAPHICS_VIEW_H