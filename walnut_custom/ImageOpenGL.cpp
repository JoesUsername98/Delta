#include "Walnut/Image.h"

#include <iostream>

namespace Walnut {

	Image::Image(const std::string& path)
		: m_Filepath(path)
	{
		// OpenGL image loading implementation would go here
		// For now, this is a stub implementation
		std::cout << "Loading image: " << path << std::endl;
	}

	Image::Image(uint32_t width, uint32_t height, ImageFormat format, const void* data)
		: m_Width(width), m_Height(height), m_Format(format)
	{
		// OpenGL image creation implementation would go here
		// For now, this is a stub implementation
		std::cout << "Creating image: " << width << "x" << height << std::endl;
	}

	Image::~Image()
	{
		// Cleanup would go here - delete OpenGL texture
	}

	void Image::SetData(const void* data)
	{
		// Data update implementation would go here
	}

}