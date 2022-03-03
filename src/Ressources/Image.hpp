#pragma once

// std
#include <string>

namespace hyd
{
    
class Image
{
public:
    Image(const std::string& filepath);
    ~Image();

    int getWidth() { return m_width; }
    int getHeight() { return m_height; }
    int getChannelsNumber() { return m_channelsNumber; }
    unsigned char* getData()  { return m_imgData; }

private:
    /* data */
    unsigned char* m_imgData;
    int m_width;
    int m_height;
    int m_channelsNumber;
};
    
} // namespace hyd
