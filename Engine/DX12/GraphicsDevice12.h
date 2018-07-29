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

#include "DescriptorHeap12.h"

namespace Kodiak
{

class DescriptorAllocator;
class SwapChain;

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(const std::string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height);
	void Destroy();

	void PrepareFrame();
	void SubmitFrame();

	SwapChain* GetSwapChain() { return m_swapChain.get(); }
	Format GetDepthFormat() const { return Format::D32_Float_S8_UInt; }
	uint32_t GetCurrentBuffer() const { return m_currentBuffer; }

	void WaitForGpuIdle();

private:
	bool m_initialized{ false };

	std::string m_appName;

	HINSTANCE m_hinst{ 0 };
	HWND m_hwnd{ 0 };

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_destWidth{ 0 };
	uint32_t m_destHeight{ 0 };

	Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	Microsoft::WRL::ComPtr<ID3D12Device1> m_device1;
	Microsoft::WRL::ComPtr<ID3D12Device2> m_device2;
	Microsoft::WRL::ComPtr<ID3D12Device3> m_device3;
	Microsoft::WRL::ComPtr<ID3D12Device4> m_device4;

	bool m_typedUAVLoadSupport_R11G11B10_FLOAT{ false };
	bool m_typedUAVLoadSupport_R16G16B16A16_FLOAT{ false };

	std::unique_ptr<SwapChain> m_swapChain;

	uint32_t m_currentBuffer{ 0 };
};

// Utility methods and accessors
ID3D12Device* GetDevice();

extern DescriptorAllocator g_descriptorAllocator[];
inline D3D12_CPU_DESCRIPTOR_HANDLE AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE type, UINT count = 1)
{
	return g_descriptorAllocator[type].Allocate(count);
}

} // namespace Kodiak