//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#include "Stdafx.h"

#include "DepthBuffer12.h"

#include "GraphicsDevice12.h"
#include "DescriptorHeap12.h"
#include "Util12.h"


using namespace Kodiak;
using namespace std;


DepthBuffer::DepthBuffer(float clearDepth, uint8_t clearStencil)
	: m_clearDepth(clearDepth)
	, m_clearStencil(clearStencil)
{}


DepthBuffer::~DepthBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource);
}


void DepthBuffer::CreateDerivedViews()
{
	DepthStencilViewDesc dsvDesc = {};
	dsvDesc.format = m_format;
	dsvDesc.readOnlyDepth = false;
	dsvDesc.readOnlyStencil = false;

	m_dsv[0].Create(m_resource, dsvDesc);

	dsvDesc.readOnlyDepth = true;
	m_dsv[1].Create(m_resource, dsvDesc);

	const bool hasStencil = IsStencilFormat(m_format);

	if (hasStencil)
	{
		dsvDesc.readOnlyDepth = false;
		dsvDesc.readOnlyStencil = true;
		m_dsv[2].Create(m_resource, dsvDesc);

		dsvDesc.readOnlyDepth = true;
		m_dsv[3].Create(m_resource, dsvDesc);
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

	m_depthSRV.Create(m_resource, m_type, srvDesc);

	if (hasStencil)
	{
		srvDesc.isDepth = false;
		srvDesc.isStencil = true;
		m_stencilSRV.Create(m_resource, m_type, srvDesc);
	}
}


void DepthBuffer::Create(const string& name, uint32_t width, uint32_t height, Format format)
{
	const uint32_t arraySize = 1;
	const uint32_t numMips = 1;
	const uint32_t numSamples = 1;
	auto resourceDesc = DescribeTex2D(width, height, arraySize, numMips, numSamples, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	m_width = width;
	m_height = height;
	m_arraySize = 1;
	m_numSamples = numSamples;
	m_format = format;
	m_type = ResourceType::Texture2D;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = static_cast<DXGI_FORMAT>(format);
	clearValue.DepthStencil.Depth = m_clearDepth;
	clearValue.DepthStencil.Stencil = m_clearStencil;

	m_usageState = ResourceState::Common;
	m_resource = CreateTextureResource(name, resourceDesc, clearValue);

	CreateDerivedViews();
}


void DepthBuffer::Create(const string& name, uint32_t width, uint32_t height, uint32_t samples, Format format)
{
	const uint32_t arraySize = 1;
	const uint32_t numMips = 1;
	auto resourceDesc = DescribeTex2D(width, height, arraySize, numMips, samples, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	m_width = width;
	m_height = height;
	m_arraySize = 1;
	m_numSamples = samples;
	m_format = format;
	m_type = (m_numSamples == 1) ? ResourceType::Texture2D : ResourceType::Texture2DMS;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = static_cast<DXGI_FORMAT>(format);
	clearValue.DepthStencil.Depth = m_clearDepth;
	clearValue.DepthStencil.Stencil = m_clearStencil;

	m_usageState = ResourceState::Common;
	m_resource = CreateTextureResource(name, resourceDesc, clearValue);

	CreateDerivedViews();
}