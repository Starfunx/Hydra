#pragma once

// libs
#include <GLFW/glfw3.h>

// std
#include <string>

namespace hyd
{

class HydWindow
{
public:
    HydWindow(int w, int h, std::string windowName);
    ~HydWindow();

    HydWindow(const HydWindow &) = delete;
    HydWindow &operator=(const HydWindow &) = delete;

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
    std::string m_windowName;
    int m_width;
    int m_height;
};

} // namespace hyd
