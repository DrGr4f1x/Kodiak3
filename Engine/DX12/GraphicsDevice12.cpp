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

#include "GraphicsDevice.h"

#include "Shader.h"
#include "Texture.h"

#include "CommandContext12.h"
#include "CommandListManager12.h"
#include "RootSignature12.h"


using namespace Kodiak;
using namespace std;


namespace
{

Microsoft::WRL::ComPtr<ID3D12Device> g_device;

const Format BackBufferColorFormat = Format::R10G10B10A2_UNorm;
const Format DepthFormat = Format::D32_Float_S8_UInt;


void ConfigureInfoQueue(ID3D12Device* device)
{
#if _DEBUG
	ID3D12InfoQueue* pInfoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&pInfoQueue))))
	{
		// Suppress whole categories of messages
		//D3D12_MESSAGE_CATEGORY Categories[] = {};

		// Suppress messages based on their severity level
		D3D12_MESSAGE_SEVERITY severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID denyIds[] =
		{
			// This occurs when there are uninitialized descriptors in a descriptor table, even when a
			// shader does not access the missing descriptors.  I find this is common when switching
			// shader permutations and not wanting to change much code to reorder resources.
			D3D12_MESSAGE_ID_INVALID_DESCRIPTOR_HANDLE,

			// Triggered when a shader does not export all color components of a render target, such as
			// when only writing RGB to an R10G10B10A2 buffer, ignoring alpha.
			D3D12_MESSAGE_ID_CREATEGRAPHICSPIPELINESTATE_PS_OUTPUT_RT_OUTPUT_MISMATCH,

			// This occurs when a descriptor table is unbound even when a shader does not access the missing
			// descriptors.  This is common with a root signature shared between disparate shaders that
			// don't all need the same types of resources.
			D3D12_MESSAGE_ID_COMMAND_LIST_DESCRIPTOR_TABLE_NOT_SET,

			// RESOURCE_BARRIER_DUPLICATE_SUBRESOURCE_TRANSITIONS
			(D3D12_MESSAGE_ID)1008,
		};

		D3D12_INFO_QUEUE_FILTER newFilter = {};
		//newFilter.DenyList.NumCategories = _countof(Categories);
		//newFilter.DenyList.pCategoryList = Categories;
		newFilter.DenyList.NumSeverities = _countof(severities);
		newFilter.DenyList.pSeverityList = severities;
		newFilter.DenyList.NumIDs = _countof(denyIds);
		newFilter.DenyList.pIDList = denyIds;

		pInfoQueue->PushStorageFilter(&newFilter);
		pInfoQueue->Release();
	}
#endif
}


Microsoft::WRL::ComPtr<IDXGISwapChain3> CreateSwapChain(IDXGIFactory4* dxgiFactory, HWND hWnd, uint32_t width, uint32_t height)
{
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = static_cast<DXGI_FORMAT>(BackBufferColorFormat);
	swapChainDesc.Scaling = DXGI_SCALING_NONE;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = NumSwapChainBuffers;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) // Win32
	assert_succeeded(dxgiFactory->CreateSwapChainForHwnd(g_commandManager.GetCommandQueue(), hWnd, &swapChainDesc, nullptr, nullptr, &swapChain));
#else // UWP
	assert_succeeded(dxgiFactory->CreateSwapChainForCoreWindow(g_commandManager.GetCommandQueue(), (IUnknown*)GameCore::g_window.Get(), &swapChainDesc, nullptr, &m_swapChain));
#endif

	ThrowIfFailed(swapChain.As(&swapChain3));
	return swapChain3;
}

} // anonymous namespace


namespace Kodiak
{

struct GraphicsDevice::PlatformData : public NonCopyable
{
	PlatformData() = default;
	~PlatformData() { Destroy(); }

	void Destroy()
	{
		swapChain = nullptr;

#if defined(_DEBUG)
		ID3D12DebugDevice* debugInterface;
		if (SUCCEEDED(device->QueryInterface(&debugInterface)))
		{
			debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
			debugInterface->Release();
		}
#endif
		
		g_device = nullptr;
		device = nullptr;
	}

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	uint64_t fenceValues[NumSwapChainBuffers]{ 0, 0, 0 };
};


const DeviceHandle& GetDevice()
{
	return g_device;
}

} // namespace Kodiak


GraphicsDevice::~GraphicsDevice() = default;


void GraphicsDevice::WaitForGpuIdle()
{
	g_commandManager.IdleGPU();
}


Format GraphicsDevice::GetColorFormat() const
{
	return BackBufferColorFormat;
}


Format GraphicsDevice::GetDepthFormat() const
{
	return DepthFormat;
}


const DeviceHandle& GraphicsDevice::GetDevice()
{
	assert(m_platformData);

	return m_platformData->device;
}


void GraphicsDevice::PlatformCreate()
{
	m_platformData = new PlatformData;

#if _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface))))
	{
		debugInterface->EnableDebugLayer();
	}
	else
	{
		Utility::Print("WARNING:  Unable to enable D3D12 debug validation layer\n");
	}
#endif

	ThrowIfFailed(D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, nullptr, nullptr));

	// Obtain the DXGI factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	assert_succeeded(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

	// Create the D3D graphics device
	Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;

	static const bool bUseWarpDriver = false;

	Microsoft::WRL::ComPtr<ID3D12Device> pDevice;

	if (!bUseWarpDriver)
	{
		size_t maxSize = 0;

		for (uint32_t idx = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(idx, &pAdapter); ++idx)
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (desc.DedicatedVideoMemory > maxSize && SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice))))
			{
				pAdapter->GetDesc1(&desc);
				Utility::Printf(L"D3D12-capable hardware found:  %s (%u MB)\n", desc.Description, desc.DedicatedVideoMemory >> 20);
				maxSize = desc.DedicatedVideoMemory;
			}
		}

		if (maxSize > 0)
		{
			m_platformData->device = pDevice;
		}
	}

	if (!m_platformData->device)
	{
		if (bUseWarpDriver)
		{
			Utility::Print("WARP software adapter requested.  Initializing...\n");
		}
		else
		{
			Utility::Print("Failed to find a hardware adapter.  Falling back to WARP.\n");
		}

		assert_succeeded(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pAdapter)));
		assert_succeeded(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice)));
		m_platformData->device = pDevice;
	}
#ifndef _RELEASE
	else
	{
		bool bDeveloperModeEnabled = false;

		// Look in the Windows Registry to determine if Developer Mode is enabled
		HKEY hKey;
		LSTATUS result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\AppModelUnlock", 0, KEY_READ, &hKey);
		if (result == ERROR_SUCCESS)
		{
			DWORD keyValue, keySize = sizeof(DWORD);
			result = RegQueryValueEx(hKey, "AllowDevelopmentWithoutDevLicense", 0, nullptr, (byte*)&keyValue, &keySize);
			if (result == ERROR_SUCCESS && keyValue == 1)
			{
				bDeveloperModeEnabled = true;
			}
			RegCloseKey(hKey);
		}

		warn_once_if_not(bDeveloperModeEnabled, "Enable Developer Mode on Windows 10 to get consistent profiling results");

		// Prevent the GPU from overclocking or underclocking to get consistent timings
		if (bDeveloperModeEnabled)
		{
			m_platformData->device->SetStablePowerState(TRUE);
		}
	}
#endif

	g_device = m_platformData->device;

	ConfigureInfoQueue(m_platformData->device.Get());

	g_commandManager.Create(m_platformData->device.Get());

	m_platformData->swapChain = CreateSwapChain(dxgiFactory.Get(), m_hwnd, m_width, m_height);
	m_currentBuffer = m_platformData->swapChain->GetCurrentBackBufferIndex();

	for (int i = 0; i < NumSwapChainBuffers; ++i)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> displayPlane;
		assert_succeeded(m_platformData->swapChain->GetBuffer(i, IID_PPV_ARGS(&displayPlane)));

		ColorBufferPtr buffer = make_shared<ColorBuffer>();
		buffer->CreateFromSwapChain("Primary SwapChain Buffer", displayPlane.Detach(), m_width, m_height, BackBufferColorFormat);

		m_swapChainBuffers[i] = buffer;
	}

	// Setup fence
	m_platformData->fenceValues[0] = g_commandManager.GetGraphicsQueue().GetNextFenceValue();
}


void GraphicsDevice::PlatformPresent()
{
	UINT presentInterval = 0;
	m_platformData->swapChain->Present(presentInterval, 0);

	m_currentBuffer = m_platformData->swapChain->GetCurrentBackBufferIndex();

	g_commandManager.WaitForFence(m_platformData->fenceValues[m_currentBuffer]);
	m_platformData->fenceValues[m_currentBuffer] = g_commandManager.GetGraphicsQueue().GetNextFenceValue();
}


void GraphicsDevice::PlatformDestroyData()
{
	delete m_platformData;
	m_platformData = nullptr;
}


void GraphicsDevice::PlatformDestroy()
{
	g_commandManager.Shutdown();
}