#include "hyd_window.hpp"

#include <stdexcept>

namespace hyd
{

HydWindow::HydWindow(int w, int h, std::string windowName):
m_width{w}, m_height{h}, m_windowName{windowName}
{
    initWindow();
}

HydWindow::~HydWindow()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void HydWindow::initWindow(){

    glfwSetErrorCallback(errorCallback);
    if (!glfwInit())
        throw std::runtime_error("Unable to initialize window!");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(m_width, m_height, m_windowName.c_str(), 
        nullptr, nullptr);

    glfwSetWindowUserPointer(m_window, this);
    // glfwSetFramebufferSizeCallback(m_window, );
}

void HydWindow::errorCallback(int error, const char* description){
    std::string strerror = "glfwError" + error;
    strerror += description;
    throw std::runtime_error(strerror);
}


} // namespace hyd
