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

#include "CommandContext.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Shader.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{
GraphicsDevice* g_graphicsDevice = nullptr;
} // namespace Kodiak

extern Kodiak::CommandListManager g_commandManager;


GraphicsDevice::GraphicsDevice() = default;


void GraphicsDevice::Initialize(const string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height)
{
	assert(!m_platformData);

	g_graphicsDevice = this;

	m_appName = appName;

	m_hinst = hInstance;
	m_hwnd = hWnd;

	m_width = width;
	m_height = height;

	PlatformCreate();
}


void GraphicsDevice::Destroy()
{
	WaitForGpuIdle();

	CommandContext::DestroyAllContexts();
	
	// TODO - get rid of this
	PlatformDestroy();

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

	PlatformDestroyData();

	g_graphicsDevice = nullptr;
}


void GraphicsDevice::SubmitFrame()
{
	PlatformPresent();

	ReleaseDeferredResources();

	++m_frameNumber;
}


ColorBufferPtr GraphicsDevice::GetBackBuffer(uint32_t index) const
{
	assert(index < NumSwapChainBuffers);
	return m_swapChainBuffers[index];
}


void GraphicsDevice::ReleaseResource(PlatformHandle handle)
{
	uint64_t nextFence = g_commandManager.GetGraphicsQueue().GetNextFenceValue();

	DeferredReleaseResource resource{ nextFence, handle };
	m_deferredResources.emplace_back(resource);
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