#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <boost/signals2.hpp>
#include <iostream>
#include <string>
#include <Windows.h>

// Signal for window events
boost::signals2::signal<void(const std::string&)> onWindowEvent;

// Global variables for theming
float backgroundColor[3] = { 0.45f, 0.55f, 0.60f }; // RGB values for background color
bool isDarkTheme = true;

void printEvent(const std::string& msg) {
    std::cout << "Event: " << msg << std::endl;
}

// Windows Entry Point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Connect signal
    onWindowEvent.connect(&printEvent);

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "ImGui + GLFW Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Emit window creation event
    onWindowEvent("Window created successfully");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create ImGui window
        {
            ImGui::Begin("Example Window");

            static float value = 0.0f;
            if (ImGui::SliderFloat("Slider", &value, 0.0f, 1.0f)) {
                onWindowEvent("Slider value changed: " + std::to_string(value));
            }

            // Color Preview
            ImGui::ColorEdit3("Background Color", backgroundColor);

            // Enhanced button functionality
            if (ImGui::Button("Click Me!")) {
                // Toggle between dark and light theme
                if (isDarkTheme) {
                    ImGui::StyleColorsLight();
                    backgroundColor[0] = 0.9f; // Light gray background
                    backgroundColor[1] = 0.9f;
                    backgroundColor[2] = 0.9f;
                }
                else {
                    ImGui::StyleColorsDark();
                    backgroundColor[0] = 0.45f; // Original dark blue-gray
                    backgroundColor[1] = 0.55f;
                    backgroundColor[2] = 0.60f;
                }
                isDarkTheme = !isDarkTheme;
                onWindowEvent("Theme changed to: " + std::string(isDarkTheme ? "Dark" : "Light"));
            }

            ImGui::Text("Click the button to toggle between dark and light theme!");

            // Display current theme status
            ImGui::TextColored(
                isDarkTheme ? ImVec4(1.0f, 1.0f, 0.0f, 1.0f) : ImVec4(0.0f, 0.0f, 1.0f, 1.0f),
                "Current Theme: %s",
                isDarkTheme ? "Dark" : "Light"
            );

            // Add Demo Window for reference
            if (ImGui::Button("Toggle Demo Window")) {
                static bool showDemo = false;
                showDemo = !showDemo;
                if (showDemo)
                    ImGui::ShowDemoWindow(&showDemo);
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}