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
#include "Graphics\GraphicsFeatures.h"


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

	// Create methods
	HRESULT CreateQueryHeap(QueryHeapType type, uint32_t queryCount, ID3D12QueryHeap** ppHeap);

	Format GetColorFormat() const { return m_colorFormat; }
	Format GetDepthFormat() const { return m_depthFormat; }

	ColorBufferPtr GetBackBuffer(uint32_t index) const;
	uint32_t GetCurrentBuffer() const { return m_currentBuffer; }

	uint32_t GetFrameNumber() const { return m_frameNumber; }

	const std::string& GetDeviceName() const { return m_deviceName; }

	void ReleaseResource(ID3D12Resource* resource);

private:
	void ReleaseDeferredResources();

	// Create the device, swapchain, etc.
	void Create();

	// Feature support
	void ReadCaps();
	void EnableFeatures(bool optionalFeatures);
	bool TryEnableFeature(bool optional, const std::string& name, bool supported);

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
		Microsoft::WRL::ComPtr<ID3D12Resource> resourceHandle;
	};
	std::list<DeferredReleaseResource> m_deferredResources;

	// DirectX 12 members
	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swapChain;

	D3D_FEATURE_LEVEL m_bestFeatureLevel{ D3D_FEATURE_LEVEL_11_0 };
	D3D_SHADER_MODEL m_bestShaderModel{ D3D_SHADER_MODEL_6_5 };

	bool m_capsRead{ false };
	D3D12_FEATURE_DATA_D3D12_OPTIONS m_dataOptions;
	D3D12_FEATURE_DATA_D3D12_OPTIONS1 m_dataOptions1;
	D3D12_FEATURE_DATA_D3D12_OPTIONS2 m_dataOptions2;
	D3D12_FEATURE_DATA_D3D12_OPTIONS3 m_dataOptions3;
	D3D12_FEATURE_DATA_D3D12_OPTIONS4 m_dataOptions4;
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 m_dataOptions5;
	D3D12_FEATURE_DATA_D3D12_OPTIONS6 m_dataOptions6;
	D3D12_FEATURE_DATA_SHADER_MODEL m_dataShaderModel;

	std::vector<std::string> m_unsupportedRequiredFeatures;
};

extern GraphicsDevice* g_graphicsDevice;


ID3D12Device* GetDevice();


} // namespace Kodiak