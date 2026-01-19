#pragma once

#include <string>
#include <cstdint>

namespace Walnut {

	enum class ImageFormat
	{
		None = 0,
		RGBA,
		RGBA32F
	};

	class Image
	{
	public:
		Image(const std::string& path);
		Image(uint32_t width, uint32_t height, ImageFormat format, const void* data = nullptr);
		~Image();

		void SetData(const void* data);

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }

		// For OpenGL, we return the texture ID as a void pointer (compatible with ImGui)
		void* GetDescriptorSet() const { return reinterpret_cast<void*>(static_cast<uintptr_t>(m_TextureID)); }

	private:
		uint32_t m_Width = 0, m_Height = 0;
		uint32_t m_TextureID = 0;  // OpenGL texture ID

		ImageFormat m_Format = ImageFormat::None;
		std::string m_Filepath;
	};

}