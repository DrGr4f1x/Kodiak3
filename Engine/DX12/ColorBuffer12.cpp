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

#include "ColorBuffer.h"

#include "GraphicsDevice.h"

#include "DescriptorHeap12.h"
#include "Util12.h"


using namespace Kodiak;
using namespace std;


namespace
{
D3D12_RESOURCE_FLAGS CombineResourceFlags(uint32_t fragmentCount)
{
	D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

	if (flags == D3D12_RESOURCE_FLAG_NONE && fragmentCount == 1)
	{
		flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | flags;
}
} // anonymous namespace





void ColorBuffer::CreateFromSwapChain(const std::string& name, const ResourceHandle& resource, uint32_t width, uint32_t height, Format format)
{
	assert(resource != nullptr);
	D3D12_RESOURCE_DESC resourceDesc = resource->GetDesc();

	m_resource.Attach(resource);
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

	RenderTargetViewDesc rtvDesc = { m_format, 0, 1, 0, true };
	m_rtvHandle.Create(m_resource, rtvDesc);
}


void ColorBuffer::Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips,
	Format format)
{
	numMips = (numMips == 0 ? ComputeNumMips(width, height) : numMips);
	const uint32_t numSamples = 1;
	auto flags = CombineResourceFlags(numSamples);
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
	const uint32_t numMips = 1;
	const uint32_t numSamples = 1;
	auto flags = CombineResourceFlags(numSamples);
	auto resourceDesc = DescribeTex2D(width, height, arrayCount, numMips, numSamples, format, flags);

	m_width = width;
	m_height = height;
	m_arraySize = arrayCount;
	m_fragmentCount = numSamples;
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