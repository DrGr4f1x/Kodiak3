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

#include "Graphics\ColorBuffer.h"
#include "GraphicsFeatures.h"


namespace Kodiak
{

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(const std::string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height, Format colorFormat, Format depthFormat);
	void Destroy();

	void SubmitFrame();

	void WaitForGpuIdle();

	Format GetColorFormat() const { return m_colorFormat; }
	Format GetDepthFormat() const { return m_depthFormat; }

	ColorBufferPtr GetBackBuffer(uint32_t index) const;
	uint32_t GetCurrentBuffer() const { return m_currentBuffer; }

	uint32_t GetFrameNumber() const { return m_frameNumber; }

	const std::string& GetDeviceName() const { return m_deviceName; }

	void ReleaseResource(PlatformHandle handle);

private:
	void ReleaseDeferredResources();

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
	bool m_vsync{ false };
	Format m_colorFormat{ Format::Unknown };
	Format m_depthFormat{ Format::Unknown };

	std::string m_deviceName{ "Unknown" };

	std::array<ColorBufferPtr, NumSwapChainBuffers> m_swapChainBuffers;
	uint32_t m_currentBuffer{ 0 };

	uint32_t m_frameNumber{ 0 };

	// Deferred resource release
	struct DeferredReleaseResource
	{
		uint64_t fenceValue;
		PlatformHandle resourceHandle;
	};
	std::list<DeferredReleaseResource> m_deferredResources;

	// Platform-specific implementation
	struct PlatformData;
	PlatformData* m_platformData{ nullptr };
};

extern GraphicsDevice* g_graphicsDevice;


const DeviceHandle GetDevice();


} // namespace Kodiak