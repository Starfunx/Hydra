#pragma once


// libs
#include <GLFW/glfw3.h>

// std
#include <string>
#include <functional> // std::function

namespace hyd
{

class Window
{
public:
    Window(int w, int h, std::string windowName);
    ~Window();

    Window(const Window &) = delete;
    Window &operator=(const Window &) = delete;

    bool shouldClose() { return glfwWindowShouldClose(m_window); }
    
    GLFWwindow* getGLFWwindow() const { return m_window; }
    inline void getFrameBufferSize(int* width, int* height) {
        glfwGetFramebufferSize(m_window, width, height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_window, width, height);
            glfwWaitEvents();
        }
    }


private:
    static void errorCallback(int error, const char* description);
    void initWindow();
    /* data */
    GLFWwindow* m_window;

    struct WindowData
    {
        std::string title;
        unsigned int width, height;

    };
    WindowData m_data;

};

} // namespace hyd
