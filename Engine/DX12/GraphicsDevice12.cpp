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

#include "GraphicsDevice12.h"

#include "Shader.h"

#include "CommandListManager12.h"
#include "PipelineState12.h"
#include "RootSignature12.h"
#include "SwapChain12.h"


using namespace Kodiak;
using namespace std;


namespace
{

ID3D12Device* g_device{ nullptr };

} // anonymous namespace


GraphicsDevice::GraphicsDevice() = default;


GraphicsDevice::~GraphicsDevice() = default;


void GraphicsDevice::Initialize(const string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height)
{
	assert(!m_initialized);

	m_appName = appName;

	m_hinst = hInstance;
	m_hwnd = hWnd;

	m_width = m_destWidth = width;
	m_height = m_destHeight = height;

	Microsoft::WRL::ComPtr<ID3D12Device> pDevice;

#if _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug> debugInterface;
	if (SUCCEEDED(D3D12GetDebugInterface(MY_IID_PPV_ARGS(&debugInterface))))
	{
		debugInterface->EnableDebugLayer();
	}
	else
	{
		Utility::Print("WARNING:  Unable to enable D3D12 debug validation layer\n");
	}
#endif

	// Obtain the DXGI factory
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
	assert_succeeded(CreateDXGIFactory2(0, MY_IID_PPV_ARGS(&dxgiFactory)));

	// Create the D3D graphics device
	Microsoft::WRL::ComPtr<IDXGIAdapter1> pAdapter;

	static const bool bUseWarpDriver = false;

	if (!bUseWarpDriver)
	{
		size_t maxSize = 0;

		for (uint32_t idx = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(idx, &pAdapter); ++idx)
		{
			DXGI_ADAPTER_DESC1 desc;
			pAdapter->GetDesc1(&desc);
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
				continue;

			if (desc.DedicatedVideoMemory > maxSize && SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice))))
			{
				pAdapter->GetDesc1(&desc);
				Utility::Printf(L"D3D12-capable hardware found:  %s (%u MB)\n", desc.Description, desc.DedicatedVideoMemory >> 20);
				maxSize = desc.DedicatedVideoMemory;
			}
		}

		if (maxSize > 0)
		{
			m_device = pDevice;
		}
	}

	if (m_device == nullptr)
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
		assert_succeeded(D3D12CreateDevice(pAdapter.Get(), D3D_FEATURE_LEVEL_11_0, MY_IID_PPV_ARGS(&pDevice)));
		m_device = pDevice;
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
			m_device->SetStablePowerState(TRUE);
		}
	}
#endif

	g_device = m_device.Get();

#if _DEBUG
	ID3D12InfoQueue* pInfoQueue = nullptr;
	if (SUCCEEDED(m_device->QueryInterface(MY_IID_PPV_ARGS(&pInfoQueue))))
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

	// We like to do read-modify-write operations on UAVs during post processing.  To support that, we
	// need to either have the hardware do typed UAV loads of R11G11B10_FLOAT or we need to manually
	// decode an R32_UINT representation of the same buffer.  This code determines if we get the hardware
	// load support.
	D3D12_FEATURE_DATA_D3D12_OPTIONS featureData = {};
	if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &featureData, sizeof(featureData))))
	{
		if (featureData.TypedUAVLoadAdditionalFormats)
		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT support =
			{
				DXGI_FORMAT_R11G11B10_FLOAT, D3D12_FORMAT_SUPPORT1_NONE, D3D12_FORMAT_SUPPORT2_NONE
			};

			if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support))) &&
				(support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
			{
				m_typedUAVLoadSupport_R11G11B10_FLOAT = true;
			}

			support.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;

			if (SUCCEEDED(m_device->CheckFeatureSupport(D3D12_FEATURE_FORMAT_SUPPORT, &support, sizeof(support))) &&
				(support.Support2 & D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD) != 0)
			{
				m_typedUAVLoadSupport_R16G16B16A16_FLOAT = true;
			}
		}
	}

	ThrowIfFailed(D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, nullptr, nullptr));

	g_commandManager.Create(m_device.Get());

	m_swapChain = make_unique<SwapChain>();
	m_swapChain->Create(dxgiFactory.Get(), m_hwnd, m_width, m_height, DXGI_FORMAT_R10G10B10A2_UNORM);

	m_initialized = true;
}


void GraphicsDevice::Destroy()
{
	PSO::DestroyAll();
	Shader::DestroyAll();
	RootSignature::DestroyAll();

	g_commandManager.Shutdown();

	m_swapChain->Destroy();
	m_swapChain.reset();

#if defined(_DEBUG)
	ID3D12DebugDevice* debugInterface;
	if (SUCCEEDED(m_device->QueryInterface(&debugInterface)))
	{
		debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		debugInterface->Release();
	}
#endif

	g_device = nullptr;

	m_device.Reset();
}


void GraphicsDevice::PrepareFrame()
{}


void GraphicsDevice::SubmitFrame()
{
	m_currentBuffer = (m_currentBuffer + 1) % SwapChain::GetBufferCount();

	UINT presentInterval = 0;

	m_swapChain->Present(presentInterval);
}


ID3D12Device* Kodiak::GetDevice()
{
	return g_device;
}