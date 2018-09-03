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

#include "ColorBuffer12.h"

#include "GraphicsDevice.h"

#include "DescriptorHeap12.h"
#include "Util12.h"


using namespace Kodiak;
using namespace std;


ColorBuffer::~ColorBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource);
}


void ColorBuffer::CreateFromSwapChain(const std::string& name, const ResourceHandle& baseResource)
{
	assert(baseResource != nullptr);
	D3D12_RESOURCE_DESC resourceDesc = baseResource->GetDesc();

	m_resource.Attach(baseResource);
	m_usageState = ResourceState::Present;

	m_width = (uint32_t)resourceDesc.Width;		// We don't care about large virtual textures yet
	m_height = resourceDesc.Height;
	m_arraySize = resourceDesc.DepthOrArraySize;
	m_numSamples = resourceDesc.SampleDesc.Count;
	m_format = MapDXGIFormatToEngine(resourceDesc.Format);

#ifndef _RELEASE
	m_resource->SetName(MakeWStr(name).c_str());
#else
	(name);
#endif

	m_rtvHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	GetDevice()->CreateRenderTargetView(m_resource, nullptr, m_rtvHandle);
}


void ColorBuffer::Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips,
	Format format)
{
	numMips = (numMips == 0 ? ComputeNumMips(width, height) : numMips);
	const uint32_t numSamples = 1;
	auto flags = CombineResourceFlags();
	auto resourceDesc = DescribeTex2D(width, height, 1, numMips, numSamples, format, flags);

	m_width = width;
	m_height = height;
	m_arraySize = 1;
	m_numSamples = numSamples;
	m_format = format;

	resourceDesc.SampleDesc.Count = m_fragmentCount;
	resourceDesc.SampleDesc.Quality = 0;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = static_cast<DXGI_FORMAT>(format);
	clearValue.Color[0] = m_clearColor.R();
	clearValue.Color[1] = m_clearColor.G();
	clearValue.Color[2] = m_clearColor.B();
	clearValue.Color[3] = m_clearColor.A();

	m_usageState = ResourceState::Common;
	m_resource = CreateTextureResource(name, resourceDesc, clearValue);

	CreateDerivedViews(format, 1, numMips);
}


void ColorBuffer::CreateArray(const std::string& name, uint32_t width, uint32_t height, uint32_t arrayCount,
	Format format)
{
	auto flags = CombineResourceFlags();
	const uint32_t numMips = 1;
	const uint32_t numSamples = 1;
	auto resourceDesc = DescribeTex2D(width, height, arrayCount, numMips, numSamples, format, flags);

	m_width = width;
	m_height = height;
	m_arraySize = arrayCount;
	m_numSamples = numSamples;
	m_format = format;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = static_cast<DXGI_FORMAT>(format);
	clearValue.Color[0] = m_clearColor.R();
	clearValue.Color[1] = m_clearColor.G();
	clearValue.Color[2] = m_clearColor.B();
	clearValue.Color[3] = m_clearColor.A();

	m_usageState = ResourceState::Common;
	m_resource = CreateTextureResource(name, resourceDesc, clearValue);

	CreateDerivedViews(format, arrayCount, 1);
}


void ColorBuffer::CreateDerivedViews(Format format, uint32_t arraySize, uint32_t numMips)
{
	assert_msg(arraySize == 1 || numMips == 1, "We don't support auto-mips on texture arrays");

	m_numMipMaps = numMips - 1;

	auto dxFormat = static_cast<DXGI_FORMAT>(format);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

	rtvDesc.Format = dxFormat;
	uavDesc.Format = GetUAVFormat(dxFormat);
	srvDesc.Format = dxFormat;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	if (arraySize > 1)
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = 0;
		rtvDesc.Texture2DArray.ArraySize = (UINT)arraySize;

		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
		uavDesc.Texture2DArray.MipSlice = 0;
		uavDesc.Texture2DArray.FirstArraySlice = 0;
		uavDesc.Texture2DArray.ArraySize = (UINT)arraySize;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.Texture2DArray.MipLevels = numMips;
		srvDesc.Texture2DArray.MostDetailedMip = 0;
		srvDesc.Texture2DArray.FirstArraySlice = 0;
		srvDesc.Texture2DArray.ArraySize = (UINT)arraySize;
	}
	else if (m_fragmentCount > 1)
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
	}
	else
	{
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = numMips;
		srvDesc.Texture2D.MostDetailedMip = 0;
	}

	if (m_srvHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_rtvHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_srvHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	auto device = GetDevice();

	// Create the render target view
	device->CreateRenderTargetView(m_resource, &rtvDesc, m_rtvHandle);

	// Create the shader resource view
	device->CreateShaderResourceView(m_resource, &srvDesc, m_srvHandle);

	if (m_fragmentCount > 1)
	{
		return;
	}

	// Create the UAVs for each mip level (RWTexture2D)
	for (uint32_t i = 0; i < numMips; ++i)
	{
		if (m_uavHandle[i].ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
		{
			m_uavHandle[i] = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		device->CreateUnorderedAccessView(m_resource, nullptr, &uavDesc, m_uavHandle[i]);

		uavDesc.Texture2D.MipSlice++;
	}
}