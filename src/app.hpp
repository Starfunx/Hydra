#pragma once

#include "Core/hyd_window.hpp" 

namespace hyd
{
    
class App
{
public:
    App(/* args */);
    ~App();
    
    void run();

private:
    /* data */
    HydWindow m_window;
};


} // namespace hyd
