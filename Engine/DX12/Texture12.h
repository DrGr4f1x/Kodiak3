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

#include "GpuResource12.h"

namespace Kodiak
{

// Forward declarations
class ManagedTexture;


class Texture : public GpuResource
{
	friend class CommandContext;

public:
	Texture();
	explicit Texture(D3D12_CPU_DESCRIPTOR_HANDLE handle);

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

	// Accessors
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetDepth() const { return m_depthOrArraySize; }
	uint32_t GetArraySize() const { return m_depthOrArraySize; }
	Format GetFormat() const { return m_format; }

	static std::shared_ptr<Texture> Load(const std::string& filename, bool sRgb = false);
	static std::shared_ptr<Texture> GetBlackTex2D();
	static std::shared_ptr<Texture> GetWhiteTex2D();
	static std::shared_ptr<Texture> GetMagentaTex2D();

	void Destroy()
	{
		GpuResource::Destroy();
		m_cpuDescriptorHandle.ptr = 0;
	}

	static void DestroyAll();

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_cpuDescriptorHandle; }

	bool operator!() const { return m_cpuDescriptorHandle.ptr == 0; }

protected:
	void LoadDDS(const std::string& fullpath, bool sRgb);
	void LoadKTX(const std::string& fullpath, bool sRgb);

protected:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuDescriptorHandle;

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depthOrArraySize{ 0 };
	Format m_format{ Format::Unknown };
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