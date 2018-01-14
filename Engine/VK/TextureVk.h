//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

#include "GpuResourceVk.h"

namespace Kodiak
{

// Forward declarations
class ManagedTexture;


class Texture : public GpuResource
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	Texture() = default;
	explicit Texture(VkImageView imageView) : m_imageView(imageView) {}
	~Texture() { Destroy(); }

	// Create a 1-level 1D texture
	void Create1D(uint32_t pitch, uint32_t width, Format format, const void* initData);
	void Create1D(uint32_t width, Format format, const void* initData)
	{
		Create1D(width, width, format, initData);
	}

	// Create a 1-level 2D texture
	void Create2D(uint32_t pitch, uint32_t width, uint32_t height, Format format, const void* initData);
	void Create2D(uint32_t width, uint32_t height, Format format, const void* initData)
	{
		Create2D(width, width, height, format, initData);
	}

	// Create a 1-level 2D texture array
	void Create2DArray(uint32_t pitch, uint32_t width, uint32_t height, uint32_t arraySlices, Format format, const void* initData);
	void Create2DArray(uint32_t width, uint32_t height, uint32_t arraySlices, Format format, const void* initData)
	{
		Create2DArray(width, width, height, arraySlices, format, initData);
	}

	// Create a 1-level cubemap texture
	void CreateCube(uint32_t pitch, uint32_t width, uint32_t height, Format format, const void* initData);
	void CreateCube(uint32_t width, uint32_t height, Format format, const void* initData)
	{
		CreateCube(width, width, height, format, initData);
	}

	// Create a volume texture
	void Create3D(uint32_t pitch, uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData);
	void Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData)
	{
		Create3D(width, width, height, depth, format, initData);
	}

	// Accessors
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetDepth() const { return m_depthOrArraySize; }
	uint32_t GetArraySize() const { return m_depthOrArraySize; }
	Format GetFormat() const { return m_format; }
	TextureTarget GetTextureTarget() const { return m_target; }

	static std::shared_ptr<Texture> Load(const std::string& filename, bool sRgb = false);
	static std::shared_ptr<Texture> GetBlackTex2D();
	static std::shared_ptr<Texture> GetWhiteTex2D();
	static std::shared_ptr<Texture> GetMagentaTex2D();

	void Destroy();

	static void DestroyAll();

	const VkImageView& GetSRV() const { return m_imageView; }

	bool operator!() const { return m_imageView == VK_NULL_HANDLE; }

protected:
	void LoadDDS(const std::string& fullpath, bool sRgb);
	void LoadKTX(const std::string& fullpath, bool sRgb);

protected:
	VkImageView		m_imageView{ VK_NULL_HANDLE };
	VkImage			m_image{ VK_NULL_HANDLE };
	VkImageLayout	m_layout{ VK_IMAGE_LAYOUT_GENERAL };
	VkAccessFlags	m_accessFlags{ 0 };

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depthOrArraySize{ 0 };
	Format m_format{ Format::Unknown };
	TextureTarget m_target;
};

using TexturePtr = std::shared_ptr<Texture>;


class ManagedTexture : public Texture
{
public:
	ManagedTexture(const std::string& filename) : m_mapKey(filename), m_isValid(true) {}

	void WaitForLoad() const;

	void SetToInvalidTexture();
	bool IsValid() const { return m_isValid; }

private:
	std::string m_mapKey;		// For deleting from the map later
	bool m_isValid;
};

} // namespace Kodiak
