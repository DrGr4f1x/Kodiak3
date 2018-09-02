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

#include "GpuResource.h"

namespace Kodiak
{

// Forward declarations
class ManagedTexture;


class TextureInitializer
{
	friend class Texture;

public:
	TextureInitializer(TextureType type, Format format, uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips)
		: m_type(type)
		, m_format(format)
		, m_width(width)
		, m_height(height)
		, m_depthOrArraySize(depthOrArraySize)
		, m_numMips(numMips)
		, m_data(nullptr)
		, m_faceSize(numMips)
		, m_faceOffsets(numMips)
		, m_arraySliceSize(0)
		, m_totalSize(0)
	{
		Initialize();
	}

	void SetData(uint32_t slice, uint32_t mipLevel, const void* data);
	size_t GetFaceSize(uint32_t mipLevel) const
	{
		return m_faceSize[mipLevel];
	}

private:
	void Initialize();

private:
	TextureType	m_type;
	Format m_format;
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depthOrArraySize;
	uint32_t m_numMips;

	std::unique_ptr<byte[]> m_data;
	std::vector<size_t>		m_faceSize;
	std::vector<size_t>		m_faceOffsets;
	size_t					m_arraySliceSize;
	size_t					m_totalSize;
};


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
	void Create1D(uint32_t width, Format format, const void* initData);

	// Create a 1-level 2D texture
	void Create2D(uint32_t width, uint32_t height, Format format, const void* initData);

	// Create a volume texture
	void Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData);

	// Create a texture from an initializer
	void Create(TextureInitializer& init);

	// Accessors
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetDepth() const { return m_depthOrArraySize; }
	uint32_t GetArraySize() const { return m_depthOrArraySize; }
	uint32_t GetNumMips() const { return m_numMips; }
	Format GetFormat() const { return m_format; }
	TextureType GetTextureType() const { return m_type; }

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
	VkImageLayout	m_layout{ VK_IMAGE_LAYOUT_GENERAL };
	VkAccessFlags	m_accessFlags{ 0 };

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depthOrArraySize{ 0 };
	uint32_t m_numMips{ 1 };
	Format m_format{ Format::Unknown };
	TextureType m_type;
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
