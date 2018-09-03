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

#include "GpuBuffer.h"

#include "GraphicsDevice.h"


using namespace Kodiak;


GpuBuffer::~GpuBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource);
}


void IndexBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = {};
	resDesc.format = Format::Unknown;
	resDesc.bufferSize = (uint32_t)m_bufferSize;
	resDesc.elementCount = (uint32_t)m_elementCount;
	resDesc.elementSize = (uint32_t)m_elementSize;

	m_srv.Create(m_resource, ResourceType::IndexBuffer, resDesc);
	m_uav.Create(m_resource, ResourceType::IndexBuffer, resDesc);
	m_ibv.Create(m_resource, resDesc);

	m_indexSize16 = (m_elementSize == 2);
}


void VertexBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = {};
	resDesc.format = Format::Unknown;
	resDesc.bufferSize = (uint32_t)m_bufferSize;
	resDesc.elementCount = (uint32_t)m_elementCount;
	resDesc.elementSize = (uint32_t)m_elementSize;

	m_srv.Create(m_resource, ResourceType::VertexBuffer, resDesc);
	m_uav.Create(m_resource, ResourceType::VertexBuffer, resDesc);
	m_vbv.Create(m_resource, resDesc);
}


void ConstantBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = {};
	resDesc.format = Format::Unknown;
	resDesc.bufferSize = (uint32_t)m_bufferSize;
	resDesc.elementCount = (uint32_t)m_elementCount;
	resDesc.elementSize = (uint32_t)m_elementSize;

	m_cbv.Create(m_resource, resDesc);
}