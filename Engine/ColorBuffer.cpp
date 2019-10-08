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

#include "ColorBuffer.h"

#include "GraphicsDevice.h"


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