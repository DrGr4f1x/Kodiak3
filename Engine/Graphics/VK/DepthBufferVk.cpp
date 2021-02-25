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

#include "DepthBufferVk.h"

#include "GraphicsDeviceVk.h"
#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


DepthBuffer::DepthBuffer(float clearDepth, uint8_t clearStencil)
	: m_clearDepth(clearDepth)
	, m_clearStencil(clearStencil)
{}


DepthBuffer::~DepthBuffer()
{
	g_graphicsDevice->ReleaseResource(m_image.Get());
}


void DepthBuffer::CreateDerivedViews()
{
	const bool hasStencil = IsStencilFormat(m_format);

	
	ImageAspect aspect = ImageAspect::Depth;
	if (hasStencil)
		aspect |= ImageAspect::Stencil;
	ThrowIfFailed(g_graphicsDevice->CreateImageView(m_image.Get(), ResourceType::Texture2D, GpuImageUsage::DepthStencilTarget, m_format, aspect, 0, 1, 0, 1, &m_imageViewDepthStencil));

	if (hasStencil)
	{
		ThrowIfFailed(g_graphicsDevice->CreateImageView(m_image.Get(), ResourceType::Texture2D, GpuImageUsage::DepthStencilTarget, m_format, ImageAspect::Depth, 0, 1, 0, 1, &m_imageViewDepthOnly));
		ThrowIfFailed(g_graphicsDevice->CreateImageView(m_image.Get(), ResourceType::Texture2D, GpuImageUsage::DepthStencilTarget, m_format, ImageAspect::Stencil, 0, 1, 0, 1, &m_imageViewStencilOnly));
	}
	else
	{
		m_imageViewDepthOnly = m_imageViewDepthStencil;
		m_imageViewStencilOnly = m_imageViewDepthStencil;
	}

	m_imageInfoDepth = { VK_NULL_HANDLE, m_imageViewDepthOnly->Get(), GetImageLayout(ResourceState::ShaderResource) };
	m_imageInfoStencil = { VK_NULL_HANDLE, m_imageViewStencilOnly->Get(), GetImageLayout(ResourceState::ShaderResource) };
}


void DepthBuffer::Create(const string& name, uint32_t width, uint32_t height, Format format)
{
	Create(name, width, height, 1, format);
}


void DepthBuffer::Create(const string& name, uint32_t width, uint32_t height, uint32_t numSamples, Format format)
{
	m_format = format;
	m_width = width;
	m_height = height;
	m_numMips = 1;
	m_arraySize = 1;
	m_numSamples = numSamples;
	m_type = ResourceType::Texture2D;

	ImageDesc desc = {};
	desc.width = m_width;
	desc.height = m_height;
	desc.depthOrArraySize = 1;
	desc.numMips = m_numMips;
	desc.numSamples = m_numSamples;
	desc.format = m_format;
	desc.type = m_numSamples == 1 ? ResourceType::Texture2D : ResourceType::Texture2DMS;
	desc.usage = GpuImageUsage::DepthStencilTarget | GpuImageUsage::CopyDest | GpuImageUsage::CopySource | GpuImageUsage::ShaderResource;
	desc.access = MemoryAccess::GpuRead | MemoryAccess::GpuWrite;

	ThrowIfFailed(g_graphicsDevice->CreateImage(name, desc, &m_image));

	CreateDerivedViews();
}