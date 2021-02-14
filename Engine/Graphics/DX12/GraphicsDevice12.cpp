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

#include "Application.h"
#include "Graphics\GraphicsFeatures.h"
#include "Graphics\Shader.h"
#include "Graphics\Texture.h"

#include "CommandContext12.h"
#include "CommandListManager12.h"
#include "RootSignature12.h"
#include "Util12.h"


using namespace Kodiak;
using namespace Utility;
using namespace std;


namespace Kodiak
{
GraphicsDevice* g_graphicsDevice = nullptr;
} // namespace Kodiak


namespace
{

Microsoft::WRL::ComPtr<ID3D12Device> g_device;


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

			D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES,

			// Silence complaints about shaders not being signed by DXIL.dll.  We don't care about this.
			D3D12_MESSAGE_ID_NON_RETAIL_SHADER_MODEL_WONT_VALIDATE,

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

Microsoft::WRL::ComPtr<IDXGISwapChain3> CreateSwapChain(IDXGIFactory4* dxgiFactory, HWND hWnd, uint32_t width, uint32_t height, Format colorFormat)
{
	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain3;

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width = width;
	swapChainDesc.Height = height;
	swapChainDesc.Format = static_cast<DXGI_FORMAT>(colorFormat);
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

ID3D12Device* GetDevice()
{
	return g_device.Get();
}

} // namespace Kodiak


GraphicsDevice::GraphicsDevice() = default;
GraphicsDevice::~GraphicsDevice() = default;


void GraphicsDevice::Initialize(const string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height, Format colorFormat, Format depthFormat)
{
	g_graphicsDevice = this;

	m_appName = appName;

	m_hinst = hInstance;
	m_hwnd = hWnd;

	m_width = width;
	m_height = height;
	m_colorFormat = colorFormat;
	m_depthFormat = depthFormat;

	Create();
}


void GraphicsDevice::Destroy()
{
	WaitForGpuIdle();

	CommandContext::DestroyAllContexts();

	// Former PlatformDestroy
	g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Destroy();
	g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].Destroy();

	PSO::DestroyAll();
	Shader::DestroyAll();
	RootSignature::DestroyAll();

	// TODO
#if DX12
	DescriptorAllocator::DestroyAll();
#endif
	Texture::DestroyAll();

	for (int i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_swapChainBuffers[i] = nullptr;
	}

	WaitForGpuIdle();

	// Flush pending deferred resources here
	ReleaseDeferredResources();
	assert(m_deferredResources.empty());

	g_commandManager.Destroy();

	m_swapChain = nullptr;

#if defined(_DEBUG)
	ID3D12DebugDevice* debugInterface;
	if (SUCCEEDED(m_device->QueryInterface(&debugInterface)))
	{
		debugInterface->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL | D3D12_RLDO_IGNORE_INTERNAL);
		debugInterface->Release();
	}
#endif

	g_device = nullptr;
	m_device = nullptr;

	g_graphicsDevice = nullptr;
}


void GraphicsDevice::SubmitFrame()
{
	UINT presentInterval = 0;
	m_swapChain->Present(presentInterval, 0);

	m_currentBuffer = m_swapChain->GetCurrentBackBufferIndex();

	ReleaseDeferredResources();

	++m_frameNumber;
}


void GraphicsDevice::WaitForGpuIdle()
{
	g_commandManager.IdleGPU();
}


HRESULT GraphicsDevice::CreateQueryHeap(QueryHeapType type, uint32_t queryCount, ID3D12QueryHeap** ppHeap)
{
	D3D12_QUERY_HEAP_DESC desc = {};
	desc.Count = queryCount;
	desc.NodeMask = 0;
	desc.Type = GetQueryHeapType(type);

	return m_device->CreateQueryHeap(&desc, IID_PPV_ARGS(ppHeap));
}


ColorBufferPtr GraphicsDevice::GetBackBuffer(uint32_t index) const
{
	assert(index < NumSwapChainBuffers);
	return m_swapChainBuffers[index];
}


void GraphicsDevice::ReleaseResource(ID3D12Resource* resource)
{
	uint64_t nextFence = g_commandManager.GetGraphicsQueue().GetNextFenceValue();

	DeferredReleaseResource deferredResource{ nextFence, resource };
	m_deferredResources.emplace_back(deferredResource);
}


void GraphicsDevice::ReleaseDeferredResources()
{
	auto resourceIt = m_deferredResources.begin();
	while (resourceIt != m_deferredResources.end())
	{
		if (g_commandManager.IsFenceComplete(resourceIt->fenceValue))
		{
			resourceIt = m_deferredResources.erase(resourceIt);
		}
		else
		{
			++resourceIt;
		}
	}
}


void GraphicsDevice::Create()
{
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

	const bool bIsDeveloperModeEnabled = GetApplication()->IsDeveloperModeEnabled();
	const bool bIsRenderDocAvailable = GetApplication()->IsRenderDocAvailable();

	if (bIsDeveloperModeEnabled && !bIsRenderDocAvailable)
	{
		ThrowIfFailed(D3D12EnableExperimentalFeatures(1, &D3D12ExperimentalShaderModels, nullptr, nullptr));
	}

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

			D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0 };

			for (int i = 0; i < _countof(featureLevels); ++i)
			{
				if (desc.DedicatedVideoMemory > maxSize && SUCCEEDED(D3D12CreateDevice(pAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&pDevice))))
				{
					pAdapter->GetDesc1(&desc);
					Utility::Printf(L"D3D12-capable hardware found:  %s (%u MB)\n", desc.Description, desc.DedicatedVideoMemory >> 20);
					maxSize = desc.DedicatedVideoMemory;
					m_bestFeatureLevel = featureLevels[i];
					m_deviceName = MakeStr(desc.Description);
					break;
				}
			}
		}

		if (maxSize > 0)
		{
			m_device = pDevice;
		}
	}

	if (!m_device)
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
		assert_succeeded(D3D12CreateDevice(pAdapter.Get(), m_bestFeatureLevel, IID_PPV_ARGS(&pDevice)));
		m_device = pDevice;
	}
#ifndef _RELEASE
	else
	{
		// Prevent the GPU from overclocking or underclocking to get consistent timings
		if (bIsDeveloperModeEnabled)
		{
			m_device->SetStablePowerState(TRUE);
		}
	}
#endif

	EnableFeatures(false);
	EnableFeatures(true);

	// Report missing features and exit
	if (!m_unsupportedRequiredFeatures.empty())
	{
		string errMsg;
		string errDetails;
		if (m_unsupportedRequiredFeatures.size() > 1)
		{
			errMsg = "Required Features Not Supported";
			errDetails = "This Application requires:\n ";
			for (size_t i = 0; i < m_unsupportedRequiredFeatures.size(); ++i)
				errDetails += m_unsupportedRequiredFeatures[i] + "\n";
			errDetails += "\n, which are unavailable.  You may need to update your GPU or graphics driver";
		}
		else
		{
			errMsg = "Required Feature Not Supported";
			errDetails = "This Application requires:\n " + m_unsupportedRequiredFeatures[0] + "\n, which is unavailable.  You may need to update your GPU or graphics driver";

		}
		ExitFatal(errMsg, errDetails);
	}

	g_device = m_device;
	
	g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].Create("User Descriptor Heap, CBV_SRV_UAV");
	g_userDescriptorHeap[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].Create("User Descriptor Heap, SAMPLER");

	ConfigureInfoQueue(m_device.Get());

	g_commandManager.Create();

	m_swapChain = CreateSwapChain(dxgiFactory.Get(), m_hwnd, m_width, m_height, m_colorFormat);
	m_currentBuffer = m_swapChain->GetCurrentBackBufferIndex();

	for (int i = 0; i < NumSwapChainBuffers; ++i)
	{
		Microsoft::WRL::ComPtr<ID3D12Resource> displayPlane;
		assert_succeeded(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&displayPlane)));

		ColorBufferPtr buffer = make_shared<ColorBuffer>();
		buffer->CreateFromSwapChain("Primary SwapChain Buffer", displayPlane.Detach(), m_width, m_height, m_colorFormat);

		m_swapChainBuffers[i] = buffer;
	}
}


void GraphicsDevice::ReadCaps()
{
	if (m_capsRead)
		return;

	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &m_dataOptions, sizeof(m_dataOptions));
	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS1, &m_dataOptions1, sizeof(m_dataOptions1));
	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS2, &m_dataOptions2, sizeof(m_dataOptions2));
	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS3, &m_dataOptions3, sizeof(m_dataOptions3));
	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS4, &m_dataOptions4, sizeof(m_dataOptions4));
	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &m_dataOptions5, sizeof(m_dataOptions5));
	m_device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &m_dataOptions6, sizeof(m_dataOptions6));

	m_dataShaderModel.HighestShaderModel = m_bestShaderModel;
	m_device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &m_dataShaderModel, sizeof(m_dataShaderModel));
	m_bestShaderModel = m_dataShaderModel.HighestShaderModel;

	m_capsRead = true;
}


void GraphicsDevice::EnableFeatures(bool optionalFeatures)
{
	ReadCaps();

	auto& requestedFeatures = optionalFeatures ? g_optionalFeatures : g_requiredFeatures;
	auto& enabledFeatures = const_cast<GraphicsFeatureSet&>(g_enabledFeatures);

	auto numFeatures = requestedFeatures.GetNumFeatures();
	for (auto i = 0; i < numFeatures; ++i)
	{
		const auto& requestedFeature = requestedFeatures[i];
		auto& enabledFeature = enabledFeatures[i];

		if (!requestedFeature)
			continue;

		const string& name = requestedFeature.GetName();

		switch (requestedFeature.GetFeature())
		{
		case GraphicsFeature::RobustBufferAccess:
			enabledFeature = true;
			break;
		case GraphicsFeature::FullDrawIndexUint32:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_bestFeatureLevel >= D3D_FEATURE_LEVEL_10_0);
			break;
		case GraphicsFeature::TextureCubeArray:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_bestFeatureLevel >= D3D_FEATURE_LEVEL_10_1);
			break;
		case GraphicsFeature::IndependentBlend:
			enabledFeature = true;
			break;
		case GraphicsFeature::GeometryShader:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_bestFeatureLevel >= D3D_FEATURE_LEVEL_10_0);
			break;
		case GraphicsFeature::TessellationShader:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_bestFeatureLevel >= D3D_FEATURE_LEVEL_11_0);
			break;
		case GraphicsFeature::SampleRateShading:
		case GraphicsFeature::DualSrcBlend:
		case GraphicsFeature::MultiDrawIndirect:
			enabledFeature = true;
			break;
		case GraphicsFeature::LogicOp:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_dataOptions.OutputMergerLogicOp == TRUE);
			break;
		case GraphicsFeature::DrawIndirectFirstInstance:
		case GraphicsFeature::DepthClamp:
		case GraphicsFeature::DepthBiasClamp:
		case GraphicsFeature::FillModeNonSolid:
			enabledFeature = true;
			break;
		case GraphicsFeature::DepthBounds:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_dataOptions2.DepthBoundsTestSupported == TRUE);
			break;
		case GraphicsFeature::WideLines:
		case GraphicsFeature::LargePoints:
		case GraphicsFeature::AlphaToOne:
			enabledFeature = TryEnableFeature(optionalFeatures, name, false);
			break;
		case GraphicsFeature::MultiViewport:
		case GraphicsFeature::SamplerAnisotropy:
			enabledFeature = true;
			break;
		case GraphicsFeature::TextureCompressionETC2:
		case GraphicsFeature::TextureCompressionASTC_LDR:
			enabledFeature = TryEnableFeature(optionalFeatures, name, false);
			break;
		case GraphicsFeature::TextureCompressionBC:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_bestFeatureLevel >= D3D_FEATURE_LEVEL_11_0);
			break;
		case GraphicsFeature::OcclusionQueryPrecise:
		case GraphicsFeature::PipelineStatisticsQuery:
			enabledFeature = true;
			break;
		case GraphicsFeature::VertexPipelineStoresAndAtomics:
		case GraphicsFeature::PixelShaderStoresAndAtomics:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_bestFeatureLevel > D3D_FEATURE_LEVEL_11_0);
			break;
		case GraphicsFeature::ShaderTessellationAndGeometryPointSize:
			enabledFeature = TryEnableFeature(optionalFeatures, name, false);
			break;
		case GraphicsFeature::ShaderTextureGatherExtended:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_bestShaderModel >= D3D_SHADER_MODEL_5_1);
			break;
		case GraphicsFeature::ShaderUAVExtendedFormats:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_dataOptions.TypedUAVLoadAdditionalFormats == TRUE);
			break;
			//case
		case GraphicsFeature::ShaderClipDistance:
		case GraphicsFeature::ShaderCullDistance:
			enabledFeature = true;
			break;
		case GraphicsFeature::ShaderFloat64:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_dataOptions.DoublePrecisionFloatShaderOps == TRUE);
			break;
		case GraphicsFeature::ShaderFloat16:
			enabledFeature = TryEnableFeature(optionalFeatures, name, (m_dataOptions.MinPrecisionSupport & D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT) != 0);
			break;
		case GraphicsFeature::ShaderInt64:
			enabledFeature = TryEnableFeature(optionalFeatures, name, m_dataOptions1.Int64ShaderOps == TRUE);
			break;
		case GraphicsFeature::ShaderInt16:
			enabledFeature = TryEnableFeature(optionalFeatures, name, (m_dataOptions.MinPrecisionSupport & D3D12_SHADER_MIN_PRECISION_SUPPORT_16_BIT) != 0);
			break;
		case GraphicsFeature::ShaderInt8:
			enabledFeature = false;
			break;

		case GraphicsFeature::VariableMultisampleRate:
			enabledFeature = true;
			break;
		}
	}
}

bool GraphicsDevice::TryEnableFeature(bool optional, const string& name, bool supported)
{
	if (!optional && !supported)
	{
		m_unsupportedRequiredFeatures.push_back(name);
	}
	return supported;
}