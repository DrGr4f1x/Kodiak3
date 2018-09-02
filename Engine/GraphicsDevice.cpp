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
#include "Shader.h"
#include "Texture.h"


using namespace Kodiak;
using namespace std;


GraphicsDevice::GraphicsDevice() = default;


void GraphicsDevice::Initialize(const string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height)
{
	assert(!m_platformData);

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

	// Flush pending deferred resources here

	PlatformDestroyData();
}


void GraphicsDevice::SubmitFrame()
{
	auto& context = GraphicsContext::Begin("Present");
	context.TransitionResource(*m_swapChainBuffers[m_currentBuffer], ResourceState::Present);
	context.Finish();

	m_currentBuffer = (m_currentBuffer + 1) % NumSwapChainBuffers;

	PlatformPresent();

	++m_frameNumber;
}


ColorBufferPtr GraphicsDevice::GetBackBuffer(uint32_t index) const
{
	assert(index < NumSwapChainBuffers);
	return m_swapChainBuffers[index];
}


void GraphicsDevice::ReleaseResource(PlatformHandle handle)
{

}