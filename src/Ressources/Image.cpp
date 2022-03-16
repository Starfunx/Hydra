#include "Image.hpp"

#define STB_IMAGE_IMPLEMENTATION   // use of stb functions once and for all
#include "stb_image.h"

// std
#include <stdexcept>

namespace hyd
{
        
Image::Image(const std::string& filepath)
{
    m_imgData = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channelsNumber, STBI_rgb_alpha);

    if(m_imgData == nullptr)
        throw(std::runtime_error("Failed to load Image: " + filepath));
}

Image::~Image()
{
    stbi_image_free(m_imgData);
}

} // namespace hyd
