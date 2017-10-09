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


namespace Kodiak
{
extern CommandListManager g_commandManager;
} // namespace Kodiak


void SwapChain::Create(IDXGIFactory4* dxgiFactory, HWND hWnd, uint32_t width, uint32_t height, DXGI_FORMAT format)
{
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = format;
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
	ASSERT_SUCCEEDED(dxgiFactory->CreateSwapChainForCoreWindow(g_commandManager.GetCommandQueue(), (IUnknown*)GameCore::g_window.Get(), &swapChainDesc, nullptr, &m_swapChain));
#endif
}


void SwapChain::Destroy()
{
	m_swapChain.Reset();
}


void SwapChain::Present(UINT presentInterval)
{
	m_swapChain->Present(presentInterval, 0);
}


uint32_t SwapChain::GetBufferCount()
{
	return SWAP_CHAIN_BUFFER_COUNT;
}