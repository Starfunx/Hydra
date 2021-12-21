#include "Window.hpp"

#include "Events/ApplicationEvent.hpp"
#include "Events/KeyEvent.hpp"
#include "Events/MouseEvent.hpp"

// std
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace hyd
{

Window::Window(int w, int h, std::string windowName)
{   
    m_data.width = w;
    m_data.height = h;
    m_data.title = windowName;

    initWindow();
}

Window::~Window()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::initWindow(){

    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
        throw std::runtime_error("Unable to initialize window!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_data.width, m_data.height, m_data.title.c_str(), 
        nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, &m_data);
    
    // Set GLFW callbacks
    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        assert(data.eventCallback && "Window callback must be set using setSetEventCallback method");
        data.width = width;
        data.height = height;

        WindowResizeEvent event(width, height);

        data.eventCallback(event);
    });

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        assert(data.eventCallback && "Window callback must be set using setSetEventCallback method");
        WindowCloseEvent event;
        data.eventCallback(event);
    });

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        assert(data.eventCallback && "Window callback must be set using setSetEventCallback method");

        switch (action)
        {
            case GLFW_PRESS:
            {
                KeyPressedEvent event(key, 0);
                data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                KeyReleasedEvent event(key);
                data.eventCallback(event);
                break;
            }
            case GLFW_REPEAT:
            {
                KeyPressedEvent event(key, 1);
                data.eventCallback(event);
                break;
            }
        }
    });

    glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int key)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        assert(data.eventCallback && "Window callback must be set using setSetEventCallback method");

        KeyTypedEvent event(key);
        data.eventCallback(event);
    });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        assert(data.eventCallback && "Window callback must be set using setSetEventCallback method");

        switch (action)
        {
            case GLFW_PRESS:
            {
                MouseButtonPressedEvent event(button);
                data.eventCallback(event);
                break;
            }
            case GLFW_RELEASE:
            {
                MouseButtonReleasedEvent event(button);
                data.eventCallback(event);
                break;
            }
        }
    });

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        assert(data.eventCallback && "Window callback must be set using setSetEventCallback method");

        MouseScrolledEvent event((float)xOffset, (float)yOffset);
        data.eventCallback(event);
    });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xPos, double yPos)
    {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        assert(data.eventCallback && "Window callback must be set using setSetEventCallback method");

        MouseMovedEvent event((float)xPos, (float)yPos);
        if( data.eventCallback)
            data.eventCallback(event);       
    });

}

void Window::errorCallback(int error, const char* description){
    std::string strerror = "glfwError" + error;
    strerror += description;
    throw std::runtime_error(strerror);
}


} // namespace hyd
