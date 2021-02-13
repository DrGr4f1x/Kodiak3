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
	DepthStencilViewDesc dsvDesc = {};
	dsvDesc.format = m_format;
	dsvDesc.readOnlyDepth = false;
	dsvDesc.readOnlyStencil = false;

	m_dsv[0].Create(m_image.Get(), dsvDesc);

	dsvDesc.readOnlyDepth = true;
	m_dsv[1].Create(m_image.Get(), dsvDesc);

	const bool hasStencil = IsStencilFormat(m_format);

	if (hasStencil)
	{
		dsvDesc.readOnlyDepth = false;
		dsvDesc.readOnlyStencil = true;
		m_dsv[2].Create(m_image.Get(), dsvDesc);

		dsvDesc.readOnlyDepth = true;
		m_dsv[3].Create(m_image.Get(), dsvDesc);
	}
	else
	{
		m_dsv[2] = m_dsv[0];
		m_dsv[3] = m_dsv[1];
	}

	TextureViewDesc srvDesc = {};
	srvDesc.format = m_format;
	srvDesc.usage = ResourceState::ShaderResource;
	srvDesc.arraySize = 1;
	srvDesc.mipCount = 1;
	srvDesc.isDepth = true;

	m_depthSRV.Create(m_image.Get(), m_type, srvDesc);

	if (hasStencil)
	{
		srvDesc.isDepth = false;
		srvDesc.isStencil = true;
		m_stencilSRV.Create(m_image.Get(), m_type, srvDesc);
	}
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
	desc.usage = GpuImageUsage::DepthStencilTarget | GpuImageUsage::CopyDest | GpuImageUsage::CopySource;
	desc.access = MemoryAccess::GpuRead | MemoryAccess::GpuWrite;

	ThrowIfFailed(g_graphicsDevice->CreateImage(name, desc, &m_image));

	CreateDerivedViews();
}