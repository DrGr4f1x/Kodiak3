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

#include "GraphicsDevice.h"

#include "DescriptorHeap12.h"


using namespace Kodiak;
using namespace std;


void DepthBuffer::Create(const std::string& name, uint32_t width, uint32_t height, Format format)
{
	const uint32_t arraySize = 1;
	const uint32_t numMips = 1;
	const uint32_t numSamples = 1;
	auto resourceDesc = DescribeTex2D(width, height, arraySize, numMips, numSamples, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = static_cast<DXGI_FORMAT>(format);
	clearValue.DepthStencil.Depth = m_clearDepth;
	clearValue.DepthStencil.Stencil = m_clearStencil;

	CreateTextureResource(name, resourceDesc, clearValue);
	CreateDerivedViews(format);
}


void DepthBuffer::Create(const std::string& name, uint32_t width, uint32_t height, uint32_t samples, Format format)
{
	const uint32_t arraySize = 1;
	const uint32_t numMips = 1;
	auto resourceDesc = DescribeTex2D(width, height, arraySize, numMips, samples, format, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = static_cast<DXGI_FORMAT>(format);
	clearValue.DepthStencil.Depth = m_clearDepth;
	clearValue.DepthStencil.Stencil = m_clearStencil;

	CreateTextureResource(name, resourceDesc, clearValue);
	CreateDerivedViews(format);
}


void DepthBuffer::CreateDerivedViews(Format format)
{
	auto dxFormat = static_cast<DXGI_FORMAT>(format);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Format = GetDSVFormat(dxFormat);
	if (m_resource->GetDesc().SampleDesc.Count == 1)
	{
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
	}
	else
	{
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;
	}

	if (m_dsv[0].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_dsv[0] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_dsv[1] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	auto device = GetDevice();

	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	device->CreateDepthStencilView(m_resource, &dsvDesc, m_dsv[0]);

	dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH;
	device->CreateDepthStencilView(m_resource, &dsvDesc, m_dsv[1]);

	DXGI_FORMAT stencilReadFormat = GetStencilFormat(dxFormat);
	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_dsv[2].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_dsv[2] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
			m_dsv[3] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		}

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		device->CreateDepthStencilView(m_resource, &dsvDesc, m_dsv[2]);

		dsvDesc.Flags = D3D12_DSV_FLAG_READ_ONLY_DEPTH | D3D12_DSV_FLAG_READ_ONLY_STENCIL;
		device->CreateDepthStencilView(m_resource, &dsvDesc, m_dsv[3]);
	}
	else
	{
		m_dsv[2] = m_dsv[0];
		m_dsv[3] = m_dsv[1];
	}

	if (m_depthSRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_depthSRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Create the shader resource view
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = GetDepthFormat(dxFormat);
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
	device->CreateShaderResourceView(m_resource, &srvDesc, m_depthSRV);

	if (stencilReadFormat != DXGI_FORMAT_UNKNOWN)
	{
		if (m_stencilSRV.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_stencilSRV = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		srvDesc.Format = stencilReadFormat;
		srvDesc.Texture2D.PlaneSlice = stencilReadFormat == DXGI_FORMAT_X32_TYPELESS_G8X24_UINT ? 1 : 0;
		device->CreateShaderResourceView(m_resource, &srvDesc, m_stencilSRV);
	}
}