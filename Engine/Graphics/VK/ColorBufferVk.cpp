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

#include "ColorBufferVk.h"
#include "GraphicsDeviceVk.h"
#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


ColorBuffer::ColorBuffer(Color clearColor)
	: m_clearColor(clearColor)
{}


ColorBuffer::~ColorBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource);
}


void ColorBuffer::CreateDerivedViews(Format format, uint32_t arraySize, uint32_t numMips)
{
	assert_msg(arraySize == 1 || numMips == 1, "We don't support auto-mips on texture arrays");

	m_numMipMaps = numMips - 1;

	RenderTargetViewDesc rtvDesc = { m_format, m_arraySize, numMips, m_fragmentCount, false };
	m_rtvHandle.Create(m_resource, rtvDesc);

	TextureViewDesc srvDesc = {};
	srvDesc.usage = ResourceState::ShaderResource;
	srvDesc.format = format;
	srvDesc.arraySize = m_arraySize;
	srvDesc.mipCount = numMips;

	ResourceType resType = ResourceType::Texture2D;
	if (m_arraySize > 1)
	{
		resType = ResourceType::Texture2D_Array;
	}
	else if (m_fragmentCount > 1)
	{
		resType = ResourceType::Texture2DMS;
	}

	m_srvHandle.Create(m_resource, resType, srvDesc);

	if (m_fragmentCount == 1)
	{
		TextureViewDesc uavDesc = {};
		uavDesc.usage = ResourceState::UnorderedAccess;
		uavDesc.format = m_format;
		uavDesc.mipCount = numMips;
		uavDesc.mipLevel = 0;
		uavDesc.arraySize = m_arraySize;

		m_uavHandle.Create(m_resource, resType, uavDesc);
	}
}


void ColorBuffer::CreateFromSwapChain(const string& name, const ResourceHandle& resource, uint32_t width, uint32_t height, Format format)
{
	m_resource = resource;
	
	SetDebugName((VkImage)resource, name);

	m_width = width;
	m_height = height;
	m_arraySize = 1;
	m_format = format;
	m_numMipMaps = 1;
	m_numSamples = 1;
	m_type = ResourceType::Texture2D;

	CreateDerivedViews(format, 1, m_numMipMaps);
}


void ColorBuffer::Create(const string& name, uint32_t width, uint32_t height, uint32_t numMips, Format format)
{
	numMips = (numMips == 0 ? ComputeNumMips(width, height) : numMips);

	m_width = width;
	m_height = height;
	m_arraySize = 1;
	m_format = format;
	m_numMips = numMips;
	m_type = ResourceType::Texture2D;

	const uint32_t arraySize = 1;
	const VkImageUsageFlags flags = 
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
		VK_IMAGE_USAGE_TRANSFER_DST_BIT |
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
		VK_IMAGE_USAGE_SAMPLED_BIT | 
		VK_IMAGE_USAGE_STORAGE_BIT;

	auto imageCreateInfo = DescribeTex2D(width, height, m_arraySize, numMips, m_numSamples, format, flags);
	m_width = imageCreateInfo.extent.width;
	m_height = imageCreateInfo.extent.height;
	m_arraySize = imageCreateInfo.arrayLayers;

	m_resource = CreateTextureResource(name, imageCreateInfo);

	CreateDerivedViews(format, 1, numMips);
}