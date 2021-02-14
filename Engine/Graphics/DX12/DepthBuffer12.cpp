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
{
	m_dsvHandle[0].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	m_dsvHandle[1].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	m_dsvHandle[2].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	m_dsvHandle[3].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	m_depthSRVHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	m_stencilSRVHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
}


DepthBuffer::~DepthBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource.Get());
}


void DepthBuffer::CreateDerivedViews()
{
	ID3D12Resource* resource = m_resource.Get();

	DXGI_FORMAT dxgiFormat = static_cast<DXGI_FORMAT>(m_format);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = GetDSVFormat(dxgiFormat);

	if (resource->GetDesc().SampleDesc.Count == 1)
	{
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
	}
	else
	{
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	}

	if (m_dsvHandle[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_dsvHandle[0] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_dsvHandle[1] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	GetDevice()->CreateDepthStencilView(resource, &dsvDesc, m_dsvHandle[0]);

	dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	GetDevice()->CreateDepthStencilView(resource, &dsvDesc, m_dsvHandle[1]);

	DXGI_FORMAT stencilReadFormat = GetStencilFormat(dxgiFormat);
	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_dsvHandle[2].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_dsvHandle[2] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			m_dsvHandle[3] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		GetDevice()->CreateDepthStencilView(resource, &dsvDesc, m_dsvHandle[2]);

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		GetDevice()->CreateDepthStencilView(resource, &dsvDesc, m_dsvHandle[3]);
	}
	else
	{
		m_dsvHandle[2] = m_dsvHandle[0];
		m_dsvHandle[3] = m_dsvHandle[1];
	}

	if (m_depthSRVHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_depthSRVHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Create the shader resource view
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = GetDepthFormat(dxgiFormat);
	if (dsvDesc.ViewDimension == D3D12_DSV_DIMENSION_TEXTURE2D)
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
	}
	else
	{
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	}
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	GetDevice()->CreateShaderResourceView(resource, &srvDesc, m_depthSRVHandle);

	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_stencilSRVHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_stencilSRVHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		srvDesc.Format = stencilReadFormat;
		srvDesc.Texture2D.PlaneSlice = (srvDesc.Format == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT) ? 1 : 0;
		GetDevice()->CreateShaderResourceView(resource, &srvDesc, m_stencilSRVHandle);
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
	CreateTextureResource(name, resourceDesc, clearValue, &m_resource);

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
	CreateTextureResource(name, resourceDesc, clearValue, &m_resource);

	CreateDerivedViews();
}