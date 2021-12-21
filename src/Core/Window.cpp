#include "Window.hpp"


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
    
}

void Window::errorCallback(int error, const char* description){
    std::string strerror = "glfwError" + error;
    strerror += description;
    throw std::runtime_error(strerror);
}


} // namespace hyd
