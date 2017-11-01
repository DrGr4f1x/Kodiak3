//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "SwapChain12.h"

#include "Utility.h"

#include "CommandListManager12.h"


using namespace Kodiak;


#define SWAP_CHAIN_BUFFER_COUNT 3


void SwapChain::Create(IDXGIFactory4* dxgiFactory, HWND hWnd, uint32_t width, uint32_t height, Format format)
{
	m_format = format;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = static_cast<DXGI_FORMAT>(format);
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) // Win32
	assert_succeeded(dxgiFactory->CreateSwapChainForHwnd(g_commandManager.GetCommandQueue(), hWnd, &swapChainDesc, nullptr, nullptr, &m_swapChain));
#else // UWP
	assert_succeeded(dxgiFactory->CreateSwapChainForCoreWindow(g_commandManager.GetCommandQueue(), (IUnknown*)GameCore::g_window.Get(), &swapChainDesc, nullptr, &m_swapChain));
#endif

	m_displayPlanes.reserve(SWAP_CHAIN_BUFFER_COUNT);
	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> displayPlane;
		assert_succeeded(m_swapChain->GetBuffer(i, MY_IID_PPV_ARGS(&displayPlane)));

		ColorBuffer buffer;
		buffer.CreateFromSwapChain("Primary SwapChain Buffer", displayPlane.Detach());
		m_displayPlanes.push_back(buffer);
	}
}


void SwapChain::Destroy()
{
	for (uint32_t i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i)
	{
		m_displayPlanes[i].Destroy();
	}
	m_displayPlanes.clear();

	m_swapChain.Reset();
}


void SwapChain::Present(UINT presentInterval)
{
	m_swapChain->Present(presentInterval, 0);
}


uint32_t SwapChain::GetImageCount() const
{
	return SWAP_CHAIN_BUFFER_COUNT;
}