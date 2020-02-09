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


#include "PixelBuffer.h"
#include "ResourceView.h"


namespace Kodiak
{

// Forward declarations
class ManagedTexture;

class TextureInitializer
{
	friend class Texture;

public:
	TextureInitializer(ResourceType type, Format format, uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips)
		: m_type(type)
		, m_format(format)
		, m_width(width)
		, m_height(height)
		, m_depthOrArraySize(depthOrArraySize)
		, m_numMips(numMips)
		, m_platformData(nullptr)
		, m_faceSize(numMips)
	{
		PlatformCreate();
	}

	~TextureInitializer();

	void SetData(uint32_t slice, uint32_t mipLevel, const void* data);
	size_t GetFaceSize(uint32_t mipLevel) const
	{
		return m_faceSize[mipLevel];
	}

private:
	void PlatformCreate();

private:
	ResourceType	m_type;
	Format			m_format;
	uint32_t		m_width;
	uint32_t		m_height;
	uint32_t		m_depthOrArraySize;
	uint32_t		m_numMips;

	struct PlatformData;
	PlatformData*	m_platformData{ nullptr };

	
	std::vector<size_t>		m_faceSize;
	
};


class Texture : public PixelBuffer
{
public:
	Texture();
	~Texture();

	// Create a 1-level 1D texture
	void Create1D(uint32_t width, Format format, const void* initData);

	// Create a 1-level 2D texture
	void Create2D(uint32_t width, uint32_t height, Format format, const void* initData);

	// Create a volume texture
	void Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData);

	// Create a texture from an initializer
	void Create(TextureInitializer& init);

	// Get pointer to retained data
	const uint8_t* GetData() const
	{
		assert(m_retainData);
		return m_data.get();
	}

	static std::shared_ptr<Texture> Load(const std::string& filename, Format format = Format::Unknown, bool sRgb = false);
	static std::shared_ptr<Texture> GetBlackTex2D();
	static std::shared_ptr<Texture> GetWhiteTex2D();
	static std::shared_ptr<Texture> GetMagentaTex2D();

	static void DestroyAll();

	const ShaderResourceView& GetSRV() const { return m_srv; }

protected:
	void LoadDDS(const std::string& fullpath, Format format, bool sRgb);
	void LoadKTX(const std::string& fullpath, Format format, bool sRgb);
	void LoadTexture(const std::string& fullpath, Format format, bool sRgb);

	void CreateDerivedViews();
	void ClearRetainedData();

protected:
	ShaderResourceView m_srv;

	std::unique_ptr<uint8_t[]>		m_data;
	size_t						m_dataSize{ 0 };
	bool						m_retainData{ false };
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

uint32_t BytesPerPixel(Format format);

} // namespace Kodiak