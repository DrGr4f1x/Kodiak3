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

#include "ColorBuffer.h"

namespace Kodiak
{

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(const std::string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height);
	void Destroy();

	void SubmitFrame();

	void WaitForGpuIdle();

	Format GetColorFormat() const;
	Format GetDepthFormat() const;

	ColorBufferPtr GetBackBuffer(uint32_t index) const;
	uint32_t GetCurrentBuffer() const { return m_currentBuffer; }

	uint64_t GetFrameNumber() const { return m_frameNumber; }

	void ReleaseResource(PlatformHandle handle);

	const DeviceHandle& GetDevice();

#if VK
	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound);
	VkFormatProperties GetFormatProperties(Format format);
#endif

private:
	void ReleaseDeferredResources(uint32_t frameIndex);

	// Platform-specific methods
	void PlatformCreate();
	void PlatformPresent();
	void PlatformDestroyData();
	// TODO - Get rid of this
	void PlatformDestroy();

private:
	std::string m_appName;

	HINSTANCE m_hinst{ 0 };
	HWND m_hwnd{ 0 };

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };

	std::array<ColorBufferPtr, NumSwapChainBuffers> m_swapChainBuffers;
	uint32_t m_currentBuffer{ 0 };

	uint64_t m_frameNumber{ 0 };

	// Deferred resource release
	std::array<std::vector<PlatformHandle>, NumSwapChainBuffers> m_deferredReleasePages;

	// Platform-specific implementation
	struct PlatformData;
	PlatformData* m_platformData{ nullptr };
};

extern GraphicsDevice* g_graphicsDevice;

const DeviceHandle& GetDevice();
#if VK
uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);
VkFormatProperties GetFormatProperties(Format format);
#endif

} // namespace Kodiak