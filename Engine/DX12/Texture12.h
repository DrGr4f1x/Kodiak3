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

	// Create a 1-level 2D texture
	void Create(uint32_t pitch, uint32_t width, uint32_t height, Format format, const void* initData);
	void Create(uint32_t width, uint32_t height, Format format, const void* initData)
	{
		Create(width, width, height, format, initData);
	}

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