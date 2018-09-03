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

#include "DepthBuffer.h"

#include "GraphicsDevice.h"


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

	ResourceViewDesc srvDesc = DescribeDepthStencil(m_format);
	srvDesc.isDepth = true;

	if (m_numSamples == 1)
	{
		m_depthSRV.Create(m_resource, ResourceType::Texture2D, srvDesc);
	}
	else
	{
		m_depthSRV.Create(m_resource, ResourceType::Texture2DMS, srvDesc);
	}

	if (hasStencil)
	{
		srvDesc.isDepth = false;
		srvDesc.isStencil = true;
		m_stencilSRV.Create(m_resource, ResourceType::Texture2D, srvDesc);
	}
}