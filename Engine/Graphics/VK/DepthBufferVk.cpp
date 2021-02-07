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

#include "Graphics\DepthBuffer.h"

#include "Graphics\GraphicsDevice.h"

#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


void DepthBuffer::Create(const string& name, uint32_t width, uint32_t height, Format format)
{
	m_format = format;

	const uint32_t arraySize = 1;
	const uint32_t numMips = 1;
	const uint32_t numSamples = 1;
	const VkImageUsageFlags flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	auto imageCreateInfo = DescribeTex2D(width, height, arraySize, numMips, numSamples, format, flags);

	m_width = imageCreateInfo.extent.width;
	m_height = imageCreateInfo.extent.height;
	m_arraySize = imageCreateInfo.arrayLayers;
	m_numSamples = 1;
	m_type = ResourceType::Texture2D;

	m_resource = CreateTextureResource(name, imageCreateInfo);

	CreateDerivedViews();
}


void DepthBuffer::Create(const string& name, uint32_t width, uint32_t height, uint32_t numSamples, Format format)
{
	m_format = format;

	const uint32_t arraySize = 1;
	const uint32_t numMips = 1;
	const VkImageUsageFlags flags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	auto imageCreateInfo = DescribeTex2D(width, height, arraySize, numMips, numSamples, format, flags);

	m_width = imageCreateInfo.extent.width;
	m_height = imageCreateInfo.extent.height;
	m_arraySize = imageCreateInfo.arrayLayers;
	m_numSamples = numSamples;
	m_type = (m_numSamples == 1) ? ResourceType::Texture2D : ResourceType::Texture2DMS;

	m_resource = CreateTextureResource(name, imageCreateInfo);

	CreateDerivedViews();
}